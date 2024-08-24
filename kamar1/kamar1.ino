//Firebase
#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>
#define REFERENCE_URL "https://smartkost-4e188-default-rtdb.firebaseio.com/"  // Your Firebase project reference url
Firebase firebase(REFERENCE_URL);

//Set WiFi
#define _SSID "san"          
#define _PASSWORD "12345678" 

//PZEM
#include <PZEM004Tv30.h>
#define PZEM_SERIAL Serial
PZEM004Tv30 pzem(PZEM_SERIAL);

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

//Deklrasi
int count = 0;

void setup() {
  //Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(_SSID, _PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  timeClient.begin();

  //PinMode
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void loop() {  
  //PZEM
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float pf = pzem.pf();

  //Jika Mendeteksi Tidak Ada Tegangan
  if(isnan(voltage)){
    //Proteksi
    count = firebase.getInt("Listrik/Kamar1/count");
    if(count >= 1){
      firebase.setInt("Listrik/Kamar1/relay", 0);
      firebase.setInt("Listrik/Kamar1/count", 0);
    }
  } else {
    //Update dan Get Data
    firebase.setFloat("Listrik/Kamar1/volt", voltage);
    firebase.setFloat("Listrik/Kamar1/ampere", current);
    firebase.setFloat("Listrik/Kamar1/watt", power);
    firebase.setFloat("Listrik/Kamar1/kwh", energy);
    firebase.setFloat("Listrik/Kamar1/cosphi", pf);

    //Count Plus
    count = count+1;
    firebase.setInt("Listrik/Kamar1/count", count);
  }

  int relay = firebase.getInt("Listrik/Kamar1/relay");
  digitalWrite(2, relay);

  int kwhToken = firebase.getInt("Listrik/Kamar1/kwhToken");

  if(energy > kwhToken){
    firebase.setInt("Listrik/Kamar1/emptyKwh", 1);
  }else{
    firebase.setInt("Listrik/Kamar1/emptyKwh", 0);
  }

  epochTime = getTime();
  firebase.setInt("Listrik/Kamar1/timestamp", epochTime);

  delay(500);
}