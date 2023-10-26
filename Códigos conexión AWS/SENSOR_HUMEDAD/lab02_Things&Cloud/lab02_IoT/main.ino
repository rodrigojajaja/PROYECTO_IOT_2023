#include "secrets.h" //I HAD PROBLEMS WITH THE POLICY OF MY "THING" IN AWS, SO TO CORRECT THIS I USED * TO BE ABLE TO COONNECT WITHOUT RESTRICTIONS.
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

 
#include "DHT.h"
#define DHTPIN 19   // Digital pin connected to the DHT sensor. THE DIGITAL PIN & THE PHYSICAL PIN BOTH WERE MY PROBLEMS TO CONNECT THE DHT11 SENSOR
#define DHTTYPE DHT11  // DHT 11
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
 
float h ;
float t;
 
DHT dht(DHTPIN, DHTTYPE); //ANOTHER PROBLEM THAT I HAD, IT WAS THIS LINE BECAUSE I USED "" TO DEFINE DHTPIN AND DHTTYPE SO IT WASN'T CORRECT BECASUSE THE ARE INT VARIABLES.
 
WiFiClientSecure net = WiFiClientSecure(); //Creating objects for wifi comunication
PubSubClient client(net);
 
void connectAWS() //Function for secure comunication between Arduino and AWS
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage() //Function for publish the message into the topic with temperature and humidity data
{
  StaticJsonDocument<200> doc;
  doc["humidity"] = h;
  doc["temperature"] = t;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length) //This function process the recieved messages and process the data in the MQTT message
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}
 
void setup() //Functions calls
{
  Serial.begin(115200);
  connectAWS();
  dht.begin();
}
 
void loop() //Repeat the instruccions in it (print the data basically)
{
  h = dht.readHumidity();
  t = dht.readTemperature();
 
 
  if (isnan(h) || isnan(t) )  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
 
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
 
  publishMessage();
  client.loop();
  delay(1000);
}