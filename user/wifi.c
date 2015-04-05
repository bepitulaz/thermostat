#include "osapi.h"
#include "user_interface.h"
#include "wifi.h"

/**
 * Connect the module to WiFi router.
 * 
 * ssid is an ssid name
 * password is password for logging in to the router
 */
void connect_to_wifi(char ssid[32], char password[64]) {
  struct station_config station_conf;
  int connection_status = wifi_station_get_connect_status();
  
  // disconnect first
  if(connection_status == STATION_GOT_IP) {
    wifi_station_disconnect();
  }

  // set station mode
  wifi_set_opmode(0x01);

  // set ap settings
  os_memcpy(&station_conf.ssid, ssid, 32);
  os_memcpy(&station_conf.password, password, 64);

  wifi_station_set_config(&station_conf);

  wifi_station_connect();
}

