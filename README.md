# Thermostat
This project is originally developed for sensing the temperature and humidity in my indoor hydroponic farm. It's utilizing [DHT11](http://www.adafruit.com/datasheets/DHT11-chinese.pdf) temperature and humidity sensor that attached to [ESP8266](http://espressif.com/en/products/esp8266/) wifi chip module. I don't need the pricey Arduino or Raspberry Pi for this kind of project. 

note: You can also use DHT22 sensor for this project. 

# Configuration

1. Install the build toolchain for ESP8266. I won't explain it here, but you can check at [esp-open-sdk's repository](https://github.com/pfalcon/esp-open-sdk).
2. Copy `include/user_config_example.h` and rename it to `include/user_config.h`. You can change the wifi setting in this file.
3. Open `Makefile` and change the path setting of `XTENSA_TOOLS_ROOT`, `SDK_BASE`, `ESPPORT`, and `EXTRA_INCDIR` to your build toolchain installation path.
4. If you use DHT22 sensor instead of DHT11, you can change it at `user_main.c` line 20. Replace `SENSOR_DHT11` with `SENSOR_DHT22`. 

# How to install

1. run `make` to build the firmware.
2. run `make flash` to burn the firmware into ESP8266 chip.

# Wiring up your DHT sensor to ESP8266

| DHT11 Pin   | ESP8266 Pin |
| ----------- | ----------- |
| +           | vcc 3.3v    |
| out/data    | GPIO 2      |
| -           | GND         |


# To Do:

Currently the ESP will boot only if GPIO 2 not attached to the DHT data pin at start up, then attach it later. I need to resolve this issue. Maybe I need to add a switch between GPIO2 and DHT11 data pin.

# Special note

You can buy me a cup of coffee if you think this code is useful :)
