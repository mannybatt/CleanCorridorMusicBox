



/**
 * 
 *  This code is designed to be a jumping off point for using a PIR Sensor
 *  for detecting people and using a DF Player Mini to play sounds in 
 *  response. t is setup to play the playlist of sounds in order and uses
 *  EEPROM to remember it's place in the playlist between power cycles.      
 *    -Manny Batt
 *
 **/


// ***************************************
// ********** Global Variables ***********
// ***************************************


//Globals for Wifi Setup and OTA
#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//WiFi Credentials
#ifndef STASSID
#define STASSID "your_ssid"
#endif
#ifndef STAPSK
#define STAPSK  "your_password"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;

//EEPROM
#include <ESP_EEPROM.h>

//MP3 Player
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
DFRobotDFPlayerMini myDFPlayer;
SoftwareSerial mySoftwareSerial(D4, D2);  //Pins for MP3 Player Serial (RX, TX)

//Input-Output
#define sensor D7
#define doorbell D5

//Variables
uint16_t numberOfSongs = 16;
uint16_t currentSong = 2;
int longestSongsLength = 31000;



// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {

  //Initialize Doorbell
  pinMode(doorbell, INPUT_PULLUP );


  //Initialize Serial, WiFi, & OTA
  wifiSetup();

  //Initialize MP3
  mySoftwareSerial.begin(9600);
  delay(1000);
  myDFPlayer.begin(mySoftwareSerial);
  Serial.println("DFPlayer initialized!");
  myDFPlayer.setTimeOut(500); //Timeout serial 500ms
  myDFPlayer.volume(0); //Volume 0-30
  myDFPlayer.EQ(0); //Equalization normal
  delay(1000);
  myDFPlayer.volume(25);
  myDFPlayer.play(1); //Boot Sound
  delay(1000);

  //EEPROM variable size initialization
  EEPROM.begin(16);

  /******* TO RESET CURRENT SONG UNCOMMENT BELOW *******/
  //Place data into EEPROM
  //EEPROM.put(0, currentSong);  // int - so 4 bytes (next address is '4')

  //Actually write that data to EEPROM
  //boolean ok = EEPROM.commit();
  //Serial.println((ok) ? "First commit OK" : "Commit failed");

  EEPROM.get(0, currentSong);
}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  //OTA
  ArduinoOTA.handle();

  //Search for Guests, Triple check to prevent false readings
  int sensorReading = digitalRead(sensor);
  if (sensorReading == HIGH) {
    delay(100);
    int sensorReading2 = digitalRead(sensor);
    if (sensorReading2 == HIGH) {
      delay(100);
      int sensorReading3 = digitalRead(sensor);
      if (sensorReading3 == HIGH) {
        Serial.println(" [ Showtime! ] ");
        delay(1000);

        //Play a song and update currentSong
        Serial.print("Playing song #");
        Serial.println(currentSong);
        playSong(currentSong);
        currentSong++;
        if (currentSong > numberOfSongs) {
          currentSong = 2;
        }

        //Commit currentSong to EEPROM
        EEPROM.put(0, currentSong);
        boolean ok = EEPROM.commit();
        Serial.println((ok) ? "First commit OK" : "Commit failed");
      }
    }
  }
  delay(100);
}




// ***************************************
// ********** Backbone Methods ***********
// ***************************************


void playSong(uint16_t song) {

  myDFPlayer.play(song);
  delay(longestSongsLength);
}

void wifiSetup() {

  //Serial
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println();
  Serial.println("****************************************");
  Serial.println("Booting");

  //WiFi and OTA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("CorridorMusicBox");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
