byte manage_wifi_led(byte led_wifi_low_state) {
  // wifi_check is LOW when wifi signal is low thus the corrensponding led should turn ON to signal the problem
  byte wifi_check;
  last_data.wifi_signal < RSSI_THRESHOLD ? wifi_check = LOW : wifi_check = HIGH;
  
  if(led_wifi_low_state != wifi_check) {
    digitalWrite(LED_WIFI_LOW, wifi_check);
    return(wifi_check);
  }
  return(led_wifi_low_state);
}

byte manage_burner_led(byte led_burner_state) {
  // burner_check is LOW when burner in ON thus the corrensponding led should turn ON
  byte burner_check;
  last_data.burner_status > 0 ? burner_check = LOW : burner_check = HIGH;
  
  if(led_burner_state != burner_check) {
    digitalWrite(LED_BURNER, burner_check);
    return(burner_check);
  }
  return(led_burner_state);
}

byte manage_resistance_led(byte led_resistance_state) {
  // wifi_check is LOW when wifi signal is low thus the corrensponding led should turn ON
  byte resistance_check;
  last_data.resistance_status > 0 ? resistance_check = LOW : resistance_check = HIGH;
  
  if(led_resistance_state != resistance_check) {
    digitalWrite(LED_RESISTANCE, resistance_check);
    return(resistance_check);
  }
  return(led_resistance_state);
}
