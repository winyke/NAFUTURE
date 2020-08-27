#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <DHT.h>

#define DHTPIN 4      //温湿度
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

float t = 0.0;
float h = 0.0;

IPAddress temp;
String ip;

unsigned long previousMillis = 0; // will store last time DHT was updated

const long interval = 3000;

const char *ssid = "";
const char *password = "";

const int ledPin = 12; //指示灯
String ledState;

AsyncWebServer server(80);

String processor(const String &var)
{
  if (var == "LOCALIP")
  {
    return ip;
  }
  else if (var == "STATE")
  {
    if (digitalRead(ledPin))
    {
      ledState = "常亮";
    }
    else
    {
      ledState = "常灭";
    }
    return ledState;
  }
  else if (var == "TEMPERATURE")
  {
    return String(t);
  }
  else if (var == "HUMIDITY")
  {
    return String(h);
  }
  return String();
}

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  dht.begin();

  //初始化SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //连接WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println(WiFi.localIP()); //打印访问地址

  temp = WiFi.localIP();
  int buf[3];
  for (int i = 0; i < 4; i++)
  {
    buf[i] = temp[i];
  }
  ip += String(buf[0]);
  ip += '.';
  ip += String(buf[1]);
  ip += '.';
  ip += String(buf[2]);
  ip += '.';
  ip += String(buf[3]);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(ledPin, HIGH);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(ledPin, LOW);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(t).c_str());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(h).c_str());
  });

  server.begin();
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      t = newT;
      //Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value
    if (isnan(newH))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      h = newH;
      //Serial.println(h);
    }
  }
}
