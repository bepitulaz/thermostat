#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mqtt.h"
#include "sensor.h"
#include "mem.h"

#define MAXTIMINGS 10000
#define DHT_MAXCOUNT 32000
#define BREAKTIME 32

#define DHTGPIO 2

enum sensor_type g_DHTType;

static ETSTimer dht_timer;

static inline float scale_humidity(int *data) {
  if(g_DHTType == SENSOR_DHT11) {
    return data[0];
  } else {
    float humidity = data[0] * 256 + data[1];
    return humidity /= 10;
  }
}

static inline float scale_temperature(int *data) {
  if(g_DHTType == SENSOR_DHT11) {
    return data[2];
  } else {
    float temperature = data[2] & 0x7f;
    temperature *= 256;
    temperature += data[3];
    temperature /= 10;

    if(data[2] & 0x80) {
      temperature *= -1;
    }

    return temperature;
  }
}

static inline void delay_ms(int sleep) {
  os_delay_us(1000 * sleep);
}

static struct sensor_reading reading = {
  .source = "DHT11", .success = 0
};

/**
 * Read DHT11 Sensor data.
 * Adapted from: https://github.com/mathew-hall/esp8266-dht/blob/master/user/dht.c
 * License: Beer-ware, but I don't drink beer. Maybe if we meet, I'll buy a cup of coffee instead.
 *
 */
void ICACHE_FLASH_ATTR poll_dht_cb(void *arg) {
  int counter = 0;
  int laststate = 1;
  int i = 0;
  int bits_in = 0;
  int data[100];

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // disable interrupts, start of critical section
  os_intr_lock();
  wdt_feed();

  // wake up device, 250ms of high
  GPIO_OUTPUT_SET(DHTGPIO, 1);
  delay_ms(20);

  // hold low for 20ms
  GPIO_OUTPUT_SET(DHTGPIO, 0);
  delay_ms(20);

  // high for 40ms
  GPIO_DIS_OUTPUT(DHTGPIO);
  os_delay_us(40);

  // wait for pin to drop?
  while (GPIO_INPUT_GET(DHTGPIO) == 1 && i < DHT_MAXCOUNT) {
    if (i >= DHT_MAXCOUNT) {
      goto fail;
    }
    i++;
  }

  os_printf("Reading DHT\n");

  // read data!
  for (i = 0; i < MAXTIMINGS; i++) {
    // Count high time (in approx us)
    counter = 0;
    while (GPIO_INPUT_GET(DHTGPIO) == laststate) {
      counter++;
      os_delay_us(1);
      if (counter == 1000)
        break;
    }
    laststate = GPIO_INPUT_GET(DHTGPIO);

    if (counter == 1000)
      break;

    // store data after 3 reads
    if ((i > 3) && (i % 2 == 0)) {
      // shove each bit into the storage bytes
      data[bits_in / 8] <<= 1;
      if (counter > BREAKTIME) {
        //os_printf("1");
        data[bits_in / 8] |= 1;
      } else {
        //os_printf("0");
      }
      bits_in++;
    }
  }

  //Re-enable interrupts, end of critical section
  os_intr_unlock();

  if (bits_in < 40) {
    os_printf("Got too few bits: %d should be at least 40", bits_in);
    goto fail;
  }
  

  int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
  
  //os_printf("DHT: %02x %02x %02x %02x [%02x] CS: %02x", data[0], data[1],data[2],data[3],data[4],checksum);
  
  if (data[4] != checksum) {
    os_printf("Checksum was incorrect after %d bits. Expected %d but got %d",
              bits_in, data[4], checksum);
    goto fail;
  }

  reading.temperature = scale_temperature(data);
  reading.humidity = scale_humidity(data);
  
  os_printf("Temperature =  %d *C, Humidity = %d %%\n", (int)(reading.temperature), (int)(reading.humidity));
  
  // publish to mqtt
  const char *temperature;
  const char *humidity;
  os_sprintf(temperature, "%d", (int)reading.temperature);
  os_sprintf(humidity, "%d", (int)reading.humidity);
  MQTT_Client* client = (MQTT_Client*)arg;
  MQTT_Publish(client, "/sensor/temperature/0", temperature, 6, 0, 0);
  MQTT_Publish(client, "/sensor/humidity/0", humidity, 6, 0, 0);

  reading.success = 1;

  os_free(temperature);
  os_free(humidity);
  return;
fail:
  
  os_printf("Failed to get reading, dying\n");
  reading.success = 0;
}

/**
 * Initialize the GPIO pin to be used by the sensor.
 *
 * @param polltime is polling time
 * @return void
 */
void init_sensor(enum sensor_type sensortype, uint32_t polltime) {
  g_DHTType = sensortype;
  
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  gpio_output_set(1, 0, DHTGPIO, 0);
  os_timer_setfn(&dht_timer, poll_dht_cb, NULL);
  os_timer_arm(&dht_timer, polltime, 1);
}
