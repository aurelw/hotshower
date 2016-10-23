/**
 * Hot Water Tank Control with Servo and ESP8266
 *
 * Copyright (c) 2016, Aurel Wildfellner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 *
 */

#include "Arduino.h"

#include "Servo.h"

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

extern "C" {
#include "user_interface.h"
}

#include "wifi_credentials.h"

/* pin out */
#define SERVO_DATA_PIN 5 

/* button press positions */
const int homePos = 10;
const int upPos = 5;
const int downPos = 60;
int globPos = homePos;
Servo mservo;
/*********************/

/* Network */
ESP8266WiFiMulti wifiMulti;
WiFiClient wclient;
/**********/

/* MQTT */
String MQTT_USERNAME = "hotshower";
String MQTT_TOPIC = "aurel/hotshower/";
IPAddress MQTTserver(158, 255, 212, 248);
PubSubClient client(MQTTserver, 1883, wclient);
/***********/

/* TANK FILLING */
const long MIN_TANK_FILL_INTERVAL = 60 * 60 * 1000; //millis
long lastTankFill = - MIN_TANK_FILL_INTERVAL;
const long FILL_CANCEL_PERIOD = 60 * 1000; //millis
bool  inFillTank = false;
/****************/

void pressOnce(int wait) {
    mservo.attach(SERVO_DATA_PIN);

    mservo.write(upPos);             
    delay(wait);                      
    mservo.write(downPos);             
    delay(wait);                      
    mservo.write(upPos);             


    mservo.detach();
}


/* inefficient but i see no better way atm */
inline String byteArrayToString(byte* array, unsigned int length) {
    char chars[length+1];
    for (int i=0; i<length; i++) {
        chars[i] =  (char) array[i];
    }
    chars[length] = '\0';
    return chars;
}


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  
  String msg = byteArrayToString(payload, length);
  Serial.print("msg length: ");
  Serial.println(length);
  Serial.print("string length: ");
  Serial.println(msg.length());
  Serial.print("msg received: ");
  Serial.println("_" + msg + "_");
  Serial.println(msg.substring(0, 10));
  
  if (msg == "PRESS") {
    pressOnce(400);
  }
  else if (msg == "FILL_TANK") {
    long cmillis = millis();
    if (lastTankFill + MIN_TANK_FILL_INTERVAL < cmillis) {
        lastTankFill = cmillis;
        inFillTank = true;
        pressOnce(400);
        pressOnce(400);
    }
  }
  else if (msg == "CANCEL_FILL_TANK") {
      if (inFillTank && lastTankFill + FILL_CANCEL_PERIOD > millis()) {
          inFillTank = false;
          pressOnce(400);
      }
  }
  
}


void setup()
{
    Serial.begin(115200);
    delay(1000);
  
    for (int i=0; i<NUM_WIFI_CREDENTIALS; i++) {
        wifiMulti.addAP(WIFI_CREDENTIALS[i][0], WIFI_CREDENTIALS[i][1]);
    }

    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("Wifi connected.");
    } else {
        Serial.println("Wifi not connected!");
    }

}
    

void loop()
{
    static unsigned int counter = 0;
    static bool wifiConnected = false;

    /* reconnect wifi */
    if(wifiMulti.run() == WL_CONNECTED) {
        if (!wifiConnected) {
            Serial.println("Wifi connected.");
            wifiConnected = true;
        }
    } else {
        if (wifiConnected) {
            Serial.println("Wifi not connected!");
            wifiConnected = false;
        }
    }


    /* MQTT */
    if (client.connected()) {
        client.loop();
    } else {
        if (client.connect(MQTT_USERNAME.c_str(), (MQTT_TOPIC + "online").c_str(), 0, true, "false")) {
            client.publish((MQTT_TOPIC + "online").c_str(), "true", true);
            // clear the CMD topic on boot in case a cmd was retained
            client.publish((MQTT_TOPIC + "cmd").c_str(), "NOCMD", true);
            Serial.println("MQTT connected");
            client.setCallback(mqtt_callback);
            client.subscribe((MQTT_TOPIC + "cmd").c_str());
        }
    }

    yield();

}

