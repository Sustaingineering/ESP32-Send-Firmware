/*
  Important Pin Assignments for our ESP32-WROVER-B/SIM800L
    SIM800L_RX     27
    SIM800L_TX     26
    SIM800L_PWRKEY 4
    SIM800L_RST    5
    SIM800L_POWER  23

    This program utilizes the farmGSM.h headers
*/

#include "Adafruit_FONA.h"
#include "farmGSM.h"

//GSM constructor with pin assignments from above
farmGSM our_GSM(27,26,4,5,23);

void setup()
{
  //our_GSM.initialize_sending();
  our_GSM.initialize_receiving();
}

void loop()
{
  /* For sending testing 
  delay(10000); //for now we will use a 10 second delay, and pass it arbitrary values
  float SourceVoltage=0;
  float HallAmps=1;
  float Power=2;
  float SolTemp=3;
  bool WaterBreakerFlag=4;
  our_GSM.send_sms(SourceVoltage, HallAmps, Power, SolTemp, WaterBreakerFlag);
*/
  our_GSM.receive_sms();
}
