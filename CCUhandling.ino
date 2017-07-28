void setStateCCU(String value) {
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "http://" + String(ccuip) + ":8181/x.exe?ret=dom.GetObject(%22" + Variable + "%22).State(%22" + value + "%22)";
    printSerial("URL = " + url);
    http.begin(url);
    int httpCode = http.GET();
    printSerial("httpcode = " + String(httpCode));
    if (httpCode > 0) {
//      String payload = http.getString();
//      Serial.println(payload);
    }
    if (httpCode != 200) {
      printSerial("HTTP fail " + String(httpCode));
    }
    http.end();
  } else ESP.restart();
}
