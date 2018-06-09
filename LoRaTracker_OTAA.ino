#include <TheThingsNetwork.h>
#include <Sodaq_UBlox_GPS.h>

// GPS
uint32_t timeout = 15UL * 1000; //Max time out is 15 seconds

//const or constant variables will never change
// Set your AppEUI and AppKey
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

#define loraSerial Serial1
#define debugSerial SerialUSB

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_US915

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan, 7);


void setup()
{
  loraSerial.begin(57600);
  debugSerial.begin(115200);
  
  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;
  debugSerial.println("-- STATUS");
  ttn.showStatus();

  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);
  
  // GPS
  sodaq_gps.init(GPS_ENABLE);
  //sodaq_gps.setDiag(debugSerial);
}

struct gps_pointers{
  float latitude;
  float longtitude;
  uint32_t delay_time;
};

struct gps_pointers f(uint32_t z){
  struct gps_pointers gps;
  sodaq_gps.scan(false, z); 
  while(sodaq_gps.getLat()==0.0){
      debugSerial.println("find no fix");
      sodaq_gps.scan(true);
    }
  
  gps.latitude = sodaq_gps.getLat();
  gps.longtitude = sodaq_gps.getLon();

  return gps;
}

void loop()
{
  struct gps_pointers gps_value;
  // declare this to enable the function
  gps_value = f(timeout);

  uint32_t LatitudeBinary = ((gps_value.latitude + 90) / 180) * 16777215;
  uint32_t LongtitudeBinary = ((gps_value.longtitude + 180) / 360) * 16777215;
  
  byte payload[6];
  payload[0] = (LatitudeBinary >> 16 ) & 0xFF;
  payload[1] = (LatitudeBinary >> 8 ) & 0xFF;
  payload[2] =  LatitudeBinary & 0xFF;

  payload[3] = (LongtitudeBinary >> 16) & 0xFF;
  payload[4] = (LongtitudeBinary >> 8) & 0xFF;
  payload[5] =  LongtitudeBinary & 0xFF;

  // Send it off
  ttn.sendBytes(payload, sizeof(payload));
  
  delay(3000);
}


