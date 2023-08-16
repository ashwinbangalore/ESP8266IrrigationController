#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL2Sw2iiF43"
#define BLYNK_TEMPLATE_NAME "WaterFlowSensor"
#define BLYNK_AUTH_TOKEN "r3fXHxrxc11GAvoYd3bi2-BPv38V9E28"

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp8266.h>


#include "secret.h"
#include "settings.h"

// NTP server
const char* ntpServer = "pool.ntp.org";

int sensorPin = D2;

volatile long pulseCount;

unsigned long lastTime;
unsigned long timeRemaining;
unsigned long timeElapsed;

float flowRate=0.0;
float flowIn15Seconds=0.0;
float totalFlow=0.0;
volatile long currentPulse;

boolean firstDay=true;

BlynkTimer blynkTimer;
int nBlynkTimerID;
int nDailyResetTimerID;
int nWeeklyResetTimerID;

WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 25200);


// runs once at device startup, once connected to Blynk server.
BLYNK_CONNECTED()
{
    Serial.println("Connected to Blynk server...");
    BLYNK_LOG("Connected to Blynk server...");
}


void setup()
{
  
    Serial.begin(115200); //Serial.begin(9600);

    // This function handles WiFi connection as well as initiating a connection with Blynk Cloud.
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

    pinMode(sensorPin, INPUT);

    // Initialize all the variables
    pulseCount = 0;
    flowRate = 0.0;
    flowIn15Seconds = 0.0;

    // you're connected now, so print out the status:
    printWifiStatus();

    attachInterrupt(digitalPinToInterrupt(sensorPin), countPulse, RISING);

    //Send to Blynk every 15 seconds.
    nBlynkTimerID = blynkTimer.setInterval(15000L, sendToBlynk);

    //Reset the counters every day.
    nDailyResetTimerID = blynkTimer.setInterval(86400000L, resetDailyCounters);

    //Restart the ESP every 14 days since the millis() parameter overflows and resets.
    nWeeklyResetTimerID = blynkTimer.setInterval(1209600000L, restartESP);

    timeClient.begin();
    timeClient.setTimeOffset(-25200);
    timeClient.update();
  
    timeElapsed = ( (timeClient.getHours() * 3600) + (timeClient.getMinutes() * 60) + timeClient.getSeconds()) * 1000;

    Serial.println("This is the time after the timeClient initialization: ");
    Serial.print("Hours: ");
    Serial.print(timeClient.getHours());
    Serial.print(", Minutes: ");
    Serial.print(timeClient.getMinutes());
    Serial.print(", Seconds: ");
    Serial.println(timeClient.getSeconds());

    Serial.print("This is timeElapsed: ");
    Serial.println(timeElapsed);
    timeRemaining = 86400000 - timeElapsed;

    Serial.print("This is timeRemaining: ");
    Serial.println(timeRemaining);
    Serial.print("Millis : ");
    Serial.println(millis());
}

void loop()
{
    Blynk.run();
    blynkTimer.run();


    if( firstDay && millis() >= timeRemaining )
    {
        pulseCount = 0;
        currentPulse = 0;
        lastTime = millis();
        totalFlow=0.0;

        blynkTimer.restartTimer(nDailyResetTimerID);
        blynkTimer.restartTimer(nWeeklyResetTimerID);

        firstDay = false;
    }

}

ICACHE_RAM_ATTR void countPulse()
{
  pulseCount++;  
}


/*
 * This function gets called every 15 seconds
 * Every 15 seconds do the following:
 *  1. Calculate current flow rate
 *  2. Calculate volume consumed
 *  3. Make API call to Blynk
 */
void sendToBlynk()  // In this function we are sending values to blynk server
{

  Serial.print("This is now: ");
  Serial.println(millis());
  Serial.print("This is the last 'time': ");
  Serial.println(lastTime);
  Serial.print("PulseCount: ");
  Serial.println(pulseCount);

  // If the system has been idle then the pulsecount will be the same as the last pulsecount.
  // In this case, don't send anything to Blynk.
  // Otherwise, calculate the flow stats and send to Blynk.
  if( pulseCount > currentPulse )
  {
    //This is Ashwin's Calc:
    flowRate = ( (pulseCount - currentPulse) * 1000) / ((millis() - lastTime) * 5.5);

    flowIn15Seconds = flowRate / 4;
    totalFlow += flowIn15Seconds;

    Serial.print(flowRate);
    Serial.println(" L/m");
    Serial.print(flowIn15Seconds);
    Serial.println(" L");
    Serial.print(totalFlow);
    Serial.println(" L");

    Blynk.virtualWrite(VPIN_TOTAL_LITERS, totalFlow);        // Total water consumption in liters (L)
    Blynk.virtualWrite(VPIN_FLOW_RATE, flowRate);            // Displays the flow rate for this second in liters / minute (L/min)
    Blynk.virtualWrite(VPIN_FLOW_15SECONDS, flowIn15Seconds);
    currentPulse = pulseCount;
  }

  lastTime = millis();

}


void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();

  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void resetDailyCounters()
{
    pulseCount = 0;
    currentPulse = 0;
    lastTime = millis();
    totalFlow=0.0;
}

void restartESP()
{
    ESP.restart();
}

