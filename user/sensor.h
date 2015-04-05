#ifndef SENSOR
#define SENSOR

enum sensor_type {
  SENSOR_DHT11, SENSOR_DHT22
};

struct sensor_reading {
  float temperature;
  float humidity;
  const char *source;
  uint8_t sensor_id[16];
  BOOL success;
};

void init_sensor(enum sensor_type, uint32_t polltime);

#endif
