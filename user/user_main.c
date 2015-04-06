#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_config.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "sensor.h"
#include "user_interface.h"
#include "mem.h"

MQTT_Client mqttClient;

void wifiConnectCb(uint8_t status) {
  if(status == STATION_GOT_IP){
    MQTT_Connect(&mqttClient);
  } else {
    MQTT_Disconnect(&mqttClient);
  }
}

void mqttConnectedCb(uint32_t *args) {
  MQTT_Client* client = (MQTT_Client*) args;
  INFO("MQTT: Connected\r\n");
}

void mqttDisconnectedCb(uint32_t *args) {
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len) {
  char *topicBuf = (char*)os_zalloc(topic_len+1);
  char *dataBuf = (char*)os_zalloc(data_len+1);

  MQTT_Client* client = (MQTT_Client*)args;

  os_memcpy(topicBuf, topic, topic_len);
  topicBuf[topic_len] = 0;

  os_memcpy(dataBuf, data, data_len);
  dataBuf[data_len] = 0;

  INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
  os_free(topicBuf);
  os_free(dataBuf);
}

void ICACHE_FLASH_ATTR user_init() {
  // initialize UART baud rate
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  
  os_delay_us(1000000);
  
  // setup the GPIO to be used by DHT11 sensor
  init_sensor(SENSOR_DHT11, 30000);

  CFG_Load();

  MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);

  MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);

  MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
  MQTT_OnConnected(&mqttClient, mqttConnectedCb);
  MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
  MQTT_OnPublished(&mqttClient, mqttPublishedCb);
  MQTT_OnData(&mqttClient, mqttDataCb);

  WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

  INFO("\r\nSystem started ...\r\n");  
}
