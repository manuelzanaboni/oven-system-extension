void botExecMessages() {
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++) {
    String messageRcvd = bot.message[i][5];
    Serial.println(messageRcvd);

    if (messageRcvd.equals("/burnerON")) {
      if(last_data.burner_status)
        bot.sendMessage(BOT_CHAT_ID, "Burner is already turned ON", "");
      else {
        mqttPublishBurner(1);
        bot.sendMessage(BOT_CHAT_ID, "Burner is ON", "");
      }
    }
    else if (messageRcvd.equals("/burnerOFF")) {
      if(!last_data.burner_status)
        bot.sendMessage(BOT_CHAT_ID, "Burner is already turned OFF", "");
      else {
        mqttPublishBurner(0);
        bot.sendMessage(BOT_CHAT_ID, "Burner is OFF", "");
      }
    }
    else if (messageRcvd.equals("/resistanceON")) {
      if(last_data.resistance_status)
        bot.sendMessage(BOT_CHAT_ID, "Resistance is already turned ON", "");
      else {
        mqttPublishResistance(1);
        bot.sendMessage(BOT_CHAT_ID, "Resistance is ON", "");
      }
    }
    else if (messageRcvd.equals("/resistanceOFF")) {
      if(!last_data.resistance_status)
        bot.sendMessage(BOT_CHAT_ID, "Resistance is already turned OFF", "");
      else {
        mqttPublishResistance(0);
        bot.sendMessage(BOT_CHAT_ID, "Resistance is OFF", "");
      }
    }
    else if (messageRcvd.equals("/state")) {
      String msg1 = "Oven state";
      String msg2;
      if(last_data.burner_status) msg2 = "Burner: ON"; else msg2 = "Burner: OFF"; 
      String msg3;
      if(last_data.resistance_status) msg3 = "Resistance: ON"; else msg3 = "Resistance: OFF"; 
      String msg4 = "SetPoint: " + String(last_data.set_point);
      bot.sendMessage(BOT_CHAT_ID, msg1, "");
      bot.sendMessage(BOT_CHAT_ID, msg2, "");
      bot.sendMessage(BOT_CHAT_ID, msg3, "");
      bot.sendMessage(BOT_CHAT_ID, msg4, "");
    }
    else if (messageRcvd.equals("/help")) {
      String welcome = "Welcome from OvenBot! Your personal Bot on ESP8266";
      String welcomeCmd1 = "/burnerON : to turn the burner ON";
      String welcomeCmd2 = "/burnerOFF : to turn the burner OFF";
      String welcomeCmd3 = "/resistanceON : to turn the resistance ON";
      String welcomeCmd4 = "/resistanceOFF : to turn the resistance OFF";
      String welcomeCmd5 = "/state : to list oven state";
      bot.sendMessage(BOT_CHAT_ID, welcome, "");
      bot.sendMessage(BOT_CHAT_ID, welcomeCmd1, "");
      bot.sendMessage(BOT_CHAT_ID, welcomeCmd2, "");
      bot.sendMessage(BOT_CHAT_ID, welcomeCmd3, "");
      bot.sendMessage(BOT_CHAT_ID, welcomeCmd4, "");
      bot.sendMessage(BOT_CHAT_ID, welcomeCmd5, "");
    } else {
      bot.sendMessage(BOT_CHAT_ID, F("Unknown command! Use /help to see all the available commands"), "");
    }
  }
  bot.message[0][0] = "";   // all messages have been replied, reset new messages
}

void mqttPublishBurner(bool val) {
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  if(val)
    doc["burner"] = true;
  else
    doc["burner"] = false;
    
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  mqttClient.publish(TOPIC_CONTROL, buffer, n, 0, 1); // no retain, qos 1
}

void mqttPublishResistance(bool val) {
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  if(val)
    doc["resistance"] = true;
  else
    doc["resistance"] = false;
    
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  mqttClient.publish(TOPIC_CONTROL, buffer, n, 0, 1); // no retain, qos 1
}
