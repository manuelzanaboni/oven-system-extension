# Oven system extension

Extension of my existing project [RPi-oven-controller](https://github.com/manuelzanaboni/RPi-oven-controller), more precisely:

* [sens_reader.py](https://github.com/manuelzanaboni/RPi-oven-controller/blob/master/controller/sens_reader.py) defines the class which manages sensors.
<!-- * [MysqlConnector.py](https://github.com/manuelzanaboni/RPi-oven-controller/blob/master/utils/MysqlConnector.py) includes new code that allows data integration. -->
* [MQTTConnector.py](https://github.com/manuelzanaboni/RPi-oven-controller/blob/master/utils/MQTTConnector.py) includes new code to operate with MQTT.
* [InfluxdbConnector.py](https://github.com/manuelzanaboni/RPi-oven-controller/blob/master/utils/InfluxdbConnector.py) includes new code to operate with InfluxDB.

This repository contains the code executed by ESP8266 (**assignment3-esp8266**) and React.js frontend (**assignment3-frontend**) actually executed on my laptop. Probably, in the future, this last application will be deployed on the Rasperry Pi too.

This last assignment includes new functionalities such as:
* Telegram Bot support
* External API connection ([Edamam](https://www.edamam.com/) and [QuickCharts](https://quickchart.io/))
* Power consumption consideration
