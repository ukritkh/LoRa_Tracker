#include <TheThingsNetwork.h>
#include <CayenneLPP.h>
#include <Sodaq_UBlox_GPS.h>

// GPS
uint32_t timeout = 15UL * 1000; //Max time out is 15 seconds


// Set your AppEUI and AppKey
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";


#define loraSerial Serial1
#define debugSerial SerialUSB
// Spreading factor 7 - 10 
#define sf 9

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_US915

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan, sf);
CayenneLPP lpp(51);

void setup()
{
  loraSerial.begin(57600);
  debugSerial.begin(9600);
  Wire.begin();
  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  debugSerial.println("-- STATUS");
  ttn.showStatus();

  //OTAA
  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);

  // GPS
  sodaq_gps.init(GPS_ENABLE);
  //sodaq_gps.setDiag(debugSerial);
 
}

void loop()
{
  uint32_t start = millis();
  if (sodaq_gps.scan(false, timeout)) {
      debugSerial.println(String(" time to find fix: ") + (millis() - start) + String("ms"));
      //debugSerial.println(String(" datetime = ") + sodaq_gps.getDateTimeString());
      debugSerial.println(String(" lat = ") + String(sodaq_gps.getLat(), 7));
      debugSerial.println(String(" lon = ") + String(sodaq_gps.getLon(), 7));
      //debugSerial.println(String(" alt = ") + String(sodaq_gps.getAlt(), 7));
      //debugSerial.println(String(" num stats = ") + String(sodaq_gps.getNumberOfSatellites()));  
         } 
  else {
      debugSerial.println("No Fix");
      while(sodaq_gps.getLat()==0.0){
          SerialUSB.println("Latitude still 0.0, doing another scan");
          SerialUSB.println(String(" num stats = ") + String(sodaq_gps.getNumberOfSatellites()));
          sodaq_gps.scan(true); // Keep the GPS enabled after we do a scan, increases accuracy
        }
    }
  uint32_t start_load = millis();
  float longti = sodaq_gps.getLon();
  float lati = sodaq_gps.getLat();
  float alt = sodaq_gps.getAlt();
  
  lpp.reset();
  lpp.addGPS(1, lati, longti,alt);

  // Send it off
  ttn.sendBytes(lpp.getBuffer(), lpp.getSize());
  delay(2000);
  
}

