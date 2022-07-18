#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.

// const char *ssid = "KS 24 BLIMBING OUTDOOR";
// const char *password = "pancongnyamantap";
// const char *ssid = "such a person";
// const char *password = "zidanedane";
const char *ssid = "Mavens 2G";
const char *password = "adminmavens";
const char *mqtt_server = "118.98.64.212";
const char *userBroker = "admin";
const char *passBroker = "adminmavens";

//=====================================
//Parameter Menangis
const int cryingRate = 560;
const int cryCountRate = 3;
const int cryErorToleransi = 10;
//======================================

const int led = 13;
const int sound_digital = 4;
const int sound_analog = A0;
const int treshold = 566;
const int th2 = 10;
const int th_atas = 600;
const int th_bawah = 500;
const int toleransi = 15;
const int ledCount = 20;

unsigned long timeAnalisis = 0;
unsigned long time_low = 0;
unsigned long time_high = 0;
unsigned long waktuselisih = 0;
unsigned long timeNow = 0;
unsigned long startTime, startTime2, startTime3, startLow, rangeLow, startHigh, changeLow;
unsigned long currentTime, currentTime3, delayHigh;
unsigned long playing_time;
unsigned long playingdelay;
unsigned long rangegelombang, rangeHigh;

boolean sinyalLow;
boolean sinyalHigh; // kondisi mendeteksi suara
boolean gel = 0;
boolean prev_gel = 0;
boolean hening = 1; // kondisi tidak ada suara

// const int num_measure = 128;
int sound_dig = 500;

int range = 0;
int analisis = 0;
int analising = 0;
int hitungGelHigh, hitungGelLow;
int rangeHighint_prev, erorToleransi;
int selisihrentang = 0;

int kelas[50];
int dataHigh[50];
int dataBaru [25];

String suara;

unsigned long panjangGelombang = 0;
long g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13, g14, g15, g16, g17, g18, g19, g20;

const long periodeAnalisis = 10000;
const long periodeNormal = 1500;
const long periodePlaying = 500;

//const long waitTime = 10000.0; // how long after trigger to wait before playing music
// const long cancelTime = 4000.0; // during wait, how long of a lull will cancel wait
//const long playTime = 3000.0;

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
char msg2[10];
char status_msg[25];
int value = 0;

int ledLevel = 0;
// int prev_sensor_reading = 553;

//============================================================
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
//============================================================

//============================================================
// fungsi connect MQTT ke Server Broker
void connectMQTT()
{
  // Loop sampai reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // membuat client ID random
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), userBroker, passBroker))
    {
      Serial.println("connected");
      // Jika connected, publish topic sekali...
      // client.publish("sensor/suara", "Pembacaan Sensor Suara");
      // ... dan resubscribe
      // client.subscribe("sensor/suara");
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

//============================================================<
// fungsi kirim data format json
void sendJsonData(String jenisSuara, int durasi)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject &JSONencoder = JSONbuffer.createObject();

  JSONencoder["jenisSuara"] = jenisSuara;
  JSONencoder["durasi"] = durasi;

  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Kirim data analisa to MQTT Broker");
  Serial.println(JSONmessageBuffer);

  if (client.publish("analisa/suara", JSONmessageBuffer) == true)
  {
    Serial.println("Success sending message");
  }
  else
  {
    Serial.println("Error sending message");
  }

  client.loop();
  Serial.println("-------------");
}
//============================================================>

//============================================================<
void debugMode(int val1, int val2, int val3, int val4, int val5)
{
  Serial.print(val1);
  Serial.print("\t");
  Serial.print(val2);
  Serial.print("\t");
  Serial.print(val3);
  Serial.print("\t");
  Serial.print(val4);
  Serial.print("\t");
  Serial.println(val5);
}
//============================================================>

//============================================================<
void debugPlotter(int val1, int val2, int val3)
{
  Serial.print(val1);
  Serial.print("\t");
  Serial.print(val2);
  Serial.print("\t");
  Serial.println(val3);
}
//============================================================>

//============================================================<
// Fungsi subscriber membaca payload dari broker
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
//============================================================>

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  pinMode(led, OUTPUT);
  pinMode(sound_digital, INPUT);

  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // client.setCallback(callback);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  int val_digital = digitalRead(sound_digital);
  int val_analog = analogRead(sound_analog);
  range = val_analog - treshold;

  range = abs(range);
  ledLevel = map(val_analog, treshold, th_atas, 0, ledCount);
  ledLevel = abs(ledLevel) * 4;

  // kirim data suara normal tiap periode waktu tertentu(sesuai nilai periodeNormal)
  currentTime = millis();
  if (currentTime - startTime >= periodeNormal)
  {
    // debugPlotter(val_analog, sound_dig, th_bawah);
    debugMode(val_analog, treshold, th_bawah, range, ledLevel);
    snprintf(msg, MSG_BUFFER_SIZE, "%d", ledLevel);
    client.publish("sensor/sinyal", msg);
    // snprintf(msg2, MSG_BUFFER_SIZE, "%d", sound_dig);
    // client.publish("sensor/sinyal2", msg2);
    //  kirim status jenis suara real time
    hening = 1;
    if (hening)
    {
      snprintf(status_msg, 8, "%d", 0);
      client.publish("status/suara", status_msg);
      hening = 0;
    }

    startTime = currentTime;
  }
  // debugMode(val_analog, treshold, th_bawah, range, ledLevel);
  // debugPlotter(val_analog, sound_dig, th_bawah);
  delay(10);

  // trigering
  if (range > th2)
  {
    time_high = millis();
    playing_time = millis();
    sound_dig = 700;
    playing = 1;
    sinyalLow = 0;
    sinyalHigh = 1;
    if (sinyalHigh) // jika ditriger ada suara, kirim perintah ini sekali
    {
      startHigh = millis();
      startTime = millis();
      startTime2 = millis();
      startTime3 = millis();
      // Serial.println("Kirim status ada suara ke broker ");
      snprintf(status_msg, 8, "%d", 1);
      client.publish("status/suara", status_msg);
      sinyalHigh = 0;

    }

    while (playing)
    {
      val_digital = digitalRead(sound_digital);
      val_analog = analogRead(sound_analog);
      range = val_analog - treshold;
      // prev_val = val_analog;
      range = abs(range);
      ledLevel = map(val_analog, th_bawah, th_atas, 1, ledCount);
      ledLevel = abs(ledLevel) * 4;
      // Serial.println("debug while playing");

      if (range > th2)
      {
        time_high = millis();
        playing_time = millis();
        sound_dig = th_atas;
        gel = 1; // untuk mengetahui kondis high low seperti tombol menghindari dbounce

        if (sinyalHigh)
        {
          startHigh = millis();
          // Serial.print("sinyal high2 : ");
          // Serial.println(startHigh);
          sinyalHigh = 0;

          // argumen untuk men
          // rangeLow = - changeLow
          if (gel != prev_gel)
          {
            hitungGelLow += 1;
            rangeLow = startHigh - changeLow;
            int rangeLowint = (int)rangeLow;
            Serial.print("Gelombang Low ke-");
            Serial.print(hitungGelLow);
            Serial.print(", Panjang Gelombang = ");
            Serial.println(rangeLowint);
          }
        }

        sinyalLow = 0;
      }

      currentTime = millis();
      if (currentTime - startTime2 >= periodePlaying)
      {
        // debugPlotter(val_analog, sound_dig,th_bawah);
        debugMode(val_analog, treshold, th_bawah, range, ledLevel);
        snprintf(msg, MSG_BUFFER_SIZE, "%d", ledLevel);
        client.publish("sensor/sinyal", msg);
        startTime2 = currentTime;
      }
      
      currentTime3 = millis();
      if (currentTime3 - startTime3 >= periodeAnalisis)
      {
          int countMax = kelas[0];
          int indexMax = 0;
          for (int l = 0; l <= (hitungGelHigh - 1); l++)
          {
            if(kelas[l] > countMax ){
              countMax = kelas[l];
              indexMax = l;
            }
          }
          Serial.print(indexMax);
          Serial.print(" || ");
          Serial.print(countMax);
          Serial.print(" || ");
          Serial.println(dataHigh[indexMax]);
          
          int nilaiToleransi = cryErorToleransi*cryingRate/100;
          if (countMax > cryCountRate && dataHigh[indexMax] > (cryingRate-nilaiToleransi) && dataHigh[indexMax] < (cryingRate+nilaiToleransi)){
            snprintf(status_msg, MSG_BUFFER_SIZE, "%d", 2);
            client.publish("status/suara", status_msg);
            Serial.println("Kirim Status Nangis");
          }
          
          
        
        // debugPlotter(val_analog, sound_dig,th_bawah);
        //debugMode(val_analog, treshold, th_bawah, range, ledLevel);
        //snprintf(msg, MSG_BUFFER_SIZE, "%d", ledLevel);
        //client.publish("sensor/sinyal", msg);
        //Serial.println("debug periode analisis");
        startTime3 = currentTime;
      }
      
      // debugPlotter(val_analog, sound_dig,th_bawah);

      // snprintf(msg, MSG_BUFFER_SIZE, "%d", ledLevel);
      // client.publish("sensor/suara", msg);
      delay(10);

      if (val_digital == LOW)
      {
        time_low = millis();
        if (!sinyalLow)
        {
          startLow = millis();
          sinyalLow = 1;
        }
        delayHigh = time_low - startLow; // waktu delay sinyal digital high
        waktuselisih = time_low - time_high;
        playingdelay = time_low - playing_time; // waktu keluar dari fungsi playing

        if (delayHigh > 150) // argumen untuk cek keluar kondisi playing
        {
          sound_dig = 500;
          if (gel != prev_gel)
          {
            changeLow = millis();
            // Serial.print("change low time stamp: ");
            // Serial.println(changeLow);
            rangeHigh = changeLow - startHigh;
            int rangeHighint = (int)rangeHigh;
            dataHigh[hitungGelHigh] = rangeHigh;

            hitungGelHigh += 1;
            /*if (hitungGelHigh == 1){
              kelas[0]+=1;
              Serial.print("Kelas ke-");
              Serial.print(hitungGelHigh);
              Serial.println(" bertambah 1");
            } */

            for (int i = 0; i <= (hitungGelHigh - 1); i++)
            {
              erorToleransi = toleransi * dataHigh[i] / 100;
              // Serial.println(dataHigh[i]);
              // Serial.println(kelas[0]);
              if (rangeHighint >= (dataHigh[i] - erorToleransi) && rangeHighint <= (dataHigh[i] + erorToleransi))
              {
                kelas[i] += 1;
                
                
                // Serial.println(erorToleransi);
                // Serial.println(dataHigh[i]);
                // Serial.println (keinlas[i]) ;

                // Serial.print("Kelas ke-");
                // Serial.print(i+1);
                // Serial.print(" bertambah 1, total menjadi : ");
                // Serial.println(kelas[i]);
              }
            }

            // rangeHighint_prev = rangeHighint;

            // prev_gel = gel;
            Serial.print("Gelombang High Ke-");
            Serial.print(hitungGelHigh);
            Serial.print(", Panjang Gelombang = ");
            Serial.println(rangeHighint);
            sinyalHigh = 1;
          }
          gel = 0;
        }
        if (playingdelay > 3000)
        {
          playing = 0;
          // Serial.println("Jumlah Gelombang = ");
          // Serial.println(hitungGelHigh);
          // Serial.println(kelas[]);
          //  percobaan kirim json
          
          int countMax = kelas[0];
          int indexMax = 0;
          for (int l = 0; l <= (hitungGelHigh - 1); l++)
          {
            if(kelas[l] > countMax ){
              countMax = kelas[l];
              indexMax = l;
            }
          }
          Serial.print(indexMax);
          Serial.print(" || ");
          Serial.print(countMax);
          Serial.print(" || ");
          Serial.println(dataHigh[indexMax]);
          
          
          
          for (int j = 0; j <= (hitungGelHigh - 1); j++)
          {
            Serial.print(dataHigh[j]);
            Serial.print(" | ");
            dataHigh[j] = 0;
          }
          Serial.println();

          for (int k = 0; k <= (hitungGelHigh - 1); k++)
          {
            Serial.print(kelas[k]);
            Serial.print(" | ");
            kelas[k] = 0;
          }
          Serial.println();

          

          if (hitungGelHigh > 10)
          {
            suara = "Tangisan Bayi";
          }
          else
          {
            suara = "Bukan Tangisan Bayi";
          }
          sendJsonData(suara, hitungGelHigh);
          hitungGelHigh = 0;
          hitungGelLow = 0;

          hening = 1;
        }
      }
    }
  }

  //============================================================
  // tes fungsi sendJsonData

  /*
  String suara = "Tangis Bayi";
  int duration = 29;

  sendJsonData(suara, duration);
  delay(3000);*/
  //============================================================

  //============================================================
  // kodingan lama 1

  /*
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
  //============================================================

  //============================================================
  // codingan lama 2
  /*

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
  //============================================================>

  //==============================================<
  // Reconnect MQTT
  if (!client.connected())
  {
    connectMQTT();
  }
  client.loop();
  //==============================================>

  //==========================================================<
  // tes publish
  /*
  snprintf(msg, MSG_BUFFER_SIZE, "%d", 100);
  client.publish("sensor/suara", msg);
  Serial.println(msg);
  delay(1000);*/
  //============================================================>

  //============================================================<
  // codingan lama 1
  /*unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
    //https://www.geeksforgeeks.org/snprintf-c-library/
    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("codersid/nodemcu/v1", msg);

  //  Serial.print(val_analog);
  //  Serial.print("\t");
  //  Serial.println(val_digital);tes
  }*/
  //===========================================================>
}
