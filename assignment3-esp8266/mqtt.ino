void connect_to_MQTT_broker() {
  if (!mqttClient.connected()) {   // not connected
    //mqttClient.setCleanSession(false);
    Serial.print(F("\nConnecting to MQTT broker..."));
    while (!mqttClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print(F("."));
      delay(1000);
    }
    Serial.println(F("\nConnected!"));

    // connected to broker, subscribe topics
    mqttClient.subscribe(TOPIC_CONFIG, 2);
    mqttClient.subscribe(TOPIC_SENSORS, 0);
    mqttClient.subscribe(TOPIC_STATE, 1);
    mqttClient.subscribe(TOPIC_CONTROL, 1);
    mqttClient.subscribe(TOPIC_TELEGRAM, 0);
    Serial.println(F("\nSubscribed to topics!"));
  }
}

// this function handles a message from the MQTT broker
void mqtt_message_received(String &topic, String &payload) {
  Serial.println(payload);

  if (topic == TOPIC_CONFIG) {
    if(payload == "frontend") {
      // return react config
      publish_frontend_config();                        // publish frontend mqtt topic config
      String msg = "Frontend connected";
      bot.sendMessage(BOT_CHAT_ID, msg, "");            // send bot message
      write_event_to_db("frontend", "connect");         // write event into MySQL
    } else
    if(payload == "rpi") {
      // return rpi config
      publish_rpi_config();                             // publish rpi mqtt topic config
    } else
    if(payload == "rpi-influxdb") {
      // return rpi-influxdb config
      publish_rpi_influxdb_config();                    // publish rpi mqtt influxdb topic config
      String msg = "RPI connected";
      bot.sendMessage(BOT_CHAT_ID, msg, "");            // send bot message
      write_event_to_db("rpi", "connect");              // write event into MySQL
    } else
    if(payload == "frontend-bye") {
      String msg = "Frontend disconnected";
      bot.sendMessage(BOT_CHAT_ID, msg, "");            // send bot message
      write_event_to_db("frontend", "disconnect");
    } else
    if(payload == "rpi-bye") {
      String msg = "RPI disconnected";
      bot.sendMessage(BOT_CHAT_ID, msg, "");            // send bot message
      write_event_to_db("rpi", "disconnect");
    }
    
  } else {
    if(topic == TOPIC_SENSORS)
      handle_sensors_data(payload);
    else
      if(topic == TOPIC_STATE)
        handle_state_data(payload);
      else
        if(topic == TOPIC_CONTROL)
          handle_control(payload);
        else
          if(topic == TOPIC_TELEGRAM)
            handle_telegram(payload);
  }
}

// send configuration to rpi
bool publish_rpi_config() {
  const int capacity = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<capacity> doc;
  doc["sensors"] = TOPIC_SENSORS;
  doc["state"] = TOPIC_STATE;
  doc["control"] = TOPIC_CONTROL;
  doc["influxdb"] = TOPIC_CONFIG_RPI_INFLUXDB;
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  return mqttClient.publish(TOPIC_CONFIG_RPI, buffer, n, 0, 2); // no retain, qos 2
}

// send influexDB configuration to rpi
bool publish_rpi_influxdb_config() {
  const int capacity = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<capacity> doc;
  doc["url"] = INFLUXDB_URL;
  doc["org"] = INFLUXDB_ORG;
  doc["token"] = INFLUXDB_TOKEN;
  doc["bucket"] = INFLUXDB_BUCKET;
  char buffer[200];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  Serial.println(n);
  return mqttClient.publish(TOPIC_CONFIG_RPI_INFLUXDB, buffer, n, 0, 2); // no retain, qos 2
}

// send configuration to frontend
bool publish_frontend_config() {
  const int capacity = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<capacity> doc;
  doc["sensors"] = TOPIC_SENSORS;
  doc["state"] = TOPIC_STATE;
  doc["control"] = TOPIC_CONTROL;
  doc["telegram"] = TOPIC_TELEGRAM;
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  return mqttClient.publish(TOPIC_CONFIG_FRONTEND, buffer, n, 0, 2); // no retain, qos 2
}

// handle incoming data from sensors
void handle_sensors_data(String &payload) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);
  last_data.oven_temp = doc["temps"]["t1"];
  last_data.floor_temp = doc["temps"]["t2"];
  last_data.puffer_temp = doc["temps"]["t3"];
  last_data.fumes_temp = doc["temps"]["t4"];
  last_data.delta_press_oven = doc["press"]["p1"];
  last_data.delta_press_gas = doc["press"]["p2"];
  last_data.valid = true;
  update_display();
}

// handle incoming data from state changes
void handle_state_data(String &payload) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);
  if(doc.containsKey("setPoint"))
    last_data.set_point = doc["setPoint"];
  if(doc.containsKey("burner"))
    last_data.burner_status = doc["burner"];
  if(doc.containsKey("resistance"))
    last_data.resistance_status = doc["resistance"];
  if(doc.containsKey("wifi"))
    last_data.wifi_signal = doc["wifi"];
  if(!doc.isNull()) {
    last_data.valid = true;
    update_display();
  }
}

// handle incoming data from controls
void handle_control(String &payload) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);
  if(doc.containsKey("burner")) {
    String control = "control-burner-";
    String val = doc["burner"];
    char buf[30];
    control.concat(val);
    control.toCharArray(buf, 30);
    Serial.println(buf);
    write_event_to_db("frontend", buf);
  } else 
  if(doc.containsKey("resistance")) {
    String control = "control-resistance-";
    String val = doc["resistance"];
    char buf[30];
    control.concat(val);
    control.toCharArray(buf, 30);
    Serial.println(buf);
    write_event_to_db("frontend", buf);
  } else
  if(doc.containsKey("thermostat")) {
    String control = "control-thermostat-";
    String val = doc["thermostat"];
    char buf[30];
    control.concat(val);
    control.toCharArray(buf, 30);
    Serial.println(buf);
    write_event_to_db("frontend", buf);
  }
}

// ToDo
void handle_telegram(String &payload) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);
    if(doc.containsKey("url")) {
      String url = doc["url"];
      String msg = "Check out this recipe: ";
      bot.sendMessage(BOT_CHAT_ID, msg + url, "");
    }
}
