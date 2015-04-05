#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"
#include "wifi.h"
#include "sensor.h"

void ICACHE_FLASH_ATTR user_init() {
  // initialize UART baud rate
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  
  // setup the SSID and SSID Password
  char ssid[32] = SSID;
  char password[64] = SSID_PASS;  
  connect_to_wifi(SSID, SSID_PASS);
  
  // setup the GPIO to be used by DHT11 sensor
  init_sensor(SENSOR_DHT11, 30000);
}
