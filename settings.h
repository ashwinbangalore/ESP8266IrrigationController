/***************************************************
 *      Hardware Settings
 **************************************************/

#define PULSE_PIN D2  //gpio4


/***************************************************
 *        Variable Settings
 **************************************************/

#define FLOW_CALIBRATION 5.5
float calibrationFactor = 5.5;
 

/***************************************************
 *        Blynk Virtual Pin Assignment
 **************************************************/

#define VPIN_TOTAL_LITERS       V1
#define VPIN_FLOW_RATE          V2
#define VPIN_FLOW_15SECONDS     V3
#define VPIN_RESET              V4

/***************************************************
 *        Server Settings
 **************************************************/
      
#define OTA_HOSTNAME "Test Flow Sensor Water Meter"
