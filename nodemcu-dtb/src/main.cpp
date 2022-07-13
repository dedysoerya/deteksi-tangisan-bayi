#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

// const char *ssid = "KS 24 BLIMBING OUTDOOR";
// const char *password = "pancongnyamantap";
const char *ssid = "such a person";
const char *password = "zidanedane";
//const char *ssid = "Mavens 2G";
//const char *password = "adminmavens";
const char *mqtt_server = "118.98.64.212";
const char *userBroker = "admin";
const char *passBroker = "adminmavens";

const int led = 13;
const int sound_digital = 15;
const int sound_analog = A0;
const int num_measure = 128;
const int ledCount = 20;

unsigned long timeAnalisis = 0;
unsigned long time_low = 0;
unsigned long time_high = 0;
unsigned long waktuselisih = 0;
unsigned long timeNow = 0;
unsigned long startTime, startLow, rangeLow, startHigh, changeLow;
unsigned long currentTime;
unsigned long playing_time;
unsigned long playingdelay;
unsigned long rangegelombang, rangeHigh;

boolean sinyalLow;
boolean sinyalHigh;
boolean gel=0;
boolean prev_gel=0;
int prev_val = 553;
int range = 0;
int analisis = 0;
int analising = 0;
int hitungGelombang = 0;
int selisihrentang = 0;
unsigned long panjangGelombang = 0;
long g1,g2,g3,g4,g5,g6,g7,g8,g9,g10,g11,g12,g13,g14,g15,g16,g17,g18,g19,g20;

const long lamaAnalisis = 10000;
const long periodeNormal = 500;

const long waitTime = 10000.0;  // how long after trigger to wait before playing music
const long cancelTime = 4000.0; // during wait, how long of a lull will cancel wait
const long playTime = 3000.0;

boolean waiting = 0;
boolean playing = 0;
long waitStart;    // when trigger initiated waiting
long waitDuration; // how long we have been waiting
long playStart;    // when play was initiated
long playDuration; // how long we've been playing
long cancelStart;  // when the last trigger was
long cancelDuration;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int ledLevel = 0;
int prev_sensor_reading = 553;

// fungsi modul untuk koneksi wifi
void setup_wifi()
{

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
/*
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    digitalWrite(led, HIGH);
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  if ((char)payload[0] == '0')
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    digitalWrite(led, LOW);
  }
}*/

// fungsi connect MQTT ke Server Broker
void reconnect()
{
  // Loop sampai reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // membuat client ID random
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),userBroker,passBroker))
    {
      Serial.println("connected");
      // Jika connected, publish topic sekali...
      client.publish("sensor/suara", "Pembacaan Sensor Suara");
      // ... dan resubscribe
      client.subscribe("sensor/suara");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Delay 5 detik sampai tersambung lagi
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  pinMode(led, OUTPUT);
  pinMode(sound_digital, INPUT);

  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{ /*
  //int sensor_reading = analogRead(sound_analog);
  //ledLevel = map(sensor_reading, 540, 700, 0, ledCount);
  //Serial.println(ledLevel);

  //for (int i = 0; i < ledCount; i++) {
    // if the array element's index is less than ledLevel,
    // turn the pin for this element on:
    //if (i < ledLevel) {
    //  client.publish("codersid/nodemcu/v1", i);

    //}
    // turn off all pins higher than the ledLevel:
    //else {
    //  digitalWrite(ledPin[i], LOW);
    //}
  }*/
  
  /*
  //====================
  
  int sensor_reading = analogRead(sound_analog);
  int rangeSensor = sensor_reading - prev_sensor_reading;
  rangeSensor = abs(rangeSensor);
  ledLevel = map(sensor_reading, 553, 600, 0, ledCount);
  ledLevel = abs(ledLevel);

  Serial.print(prev_sensor_reading);
  Serial.print("  |  ");
  prev_sensor_reading = sensor_reading;
  Serial.print(sensor_reading);
  Serial.print("  |  ");
  Serial.print(ledLevel);
  Serial.print("  |  ");
  Serial.println(rangeSensor);
  Serial.print(waiting);
  delay(10);

  // over treshold
  if (rangeSensor > 10)
  {
    cancelStart = millis();
    if (!waiting)
    {
      waiting = 1;
      digitalWrite(LED_BUILTIN, LOW); // aktiv low
      waitStart = millis();
    }
  }

  if (waiting)
  {
    snprintf(msg, MSG_BUFFER_SIZE, "%ld", ledLevel);
    client.publish("sensor/suara", msg);
    Serial.print("send sensor suara to broker");
    cancelDuration = millis() - cancelStart;
    waitDuration = millis() - waitStart;
    if (cancelDuration > cancelTime)
    {
      Serial.println("Cancel Time");
      waiting = 0;
      digitalWrite(LED_BUILTIN, HIGH);
      ledLevel = 0;
    }
    else if (waitDuration > waitTime)
    {
      // snprintf(msg, MSG_BUFFER_SIZE, "%ld", ledLevel);
      Serial.println("Suara Bayi terdeteksi");
      playStart = millis();
      playing = 1;

      while (playing)
      {
        playDuration = millis() - playStart;
        Serial.println("Suara Bayi terdeteksi1");
        if (playDuration < playTime)
        {
          Serial.println("Suara Bayi terdeteksi2");
          client.publish("sensor/suara", "Bayi Menangis");
          Serial.println("Bayi Menangis");
          digitalWrite(led, HIGH);
          delay(200);
          digitalWrite(led, LOW);
          delay(200);
        }
        else
        {
          playing = 0;
          ledLevel = 0;
        }
      }
    }
  }
  */
  //==============================================
  //Reconnect MQTT
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  //==============================================

  //tes publish
  snprintf(msg, MSG_BUFFER_SIZE, "%ld", 100);
  client.publish("sensor/suara", msg);
  Serial.println(msg);
  delay(1000);


  //  int val_digital = digitalRead(sound_digital);
  //  int val_analog = analogRead(sound_analog);

  /*unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("codersid/nodemcu/v1", msg);

  //  Serial.print(val_analog);
  //  Serial.print("\t");
  //  Serial.println(val_digital);
  }*/
}