/*
  Tutorial: Control servo movement with gesture using ESP32 and Arduino
  
  Board:
  - TTGO T-Call ESP32 with SIM800L GPRS Module
    https://my.cytron.io/p-ttgo-t-call-esp32-with-sim800l-gprs-module
  Output:
  - 1CH Active H/L 5V OptoCoupler Relay Module
    https://my.cytron.io/c-electronic-components/c-relays/p-1ch-active-h-l-5v-optocoupler-relay-module
  Connections   TTGO | Servo
                  5V - DC+
                 GND - DC-
                  IN - 14
  External libraries:
  - Adafruit Fona Library by Adafruit Version 1.3.5
*/

#include "Adafruit_FONA.h"

#define SIM800L_RX     27
#define SIM800L_TX     26
#define SIM800L_PWRKEY 4
#define SIM800L_RST    5
#define SIM800L_POWER  23

char replybuffer[255];
char sendto[21] = "7786684301";

HardwareSerial *sim800lSerial = &Serial1;
Adafruit_FONA sim800l = Adafruit_FONA(SIM800L_PWRKEY);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

#define LED_BLUE  13
#define RELAY 14

String smsString = "";
void send_sms(float LoadVoltage, float LoadCurrent, float Power, float SolTemp, bool WaterBreakerFlag);

void setup()
{
  pinMode(LED_BLUE, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SIM800L_POWER, OUTPUT);

  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(SIM800L_POWER, HIGH);

  Serial.begin(115200);
  Serial.println(F("Welcome to Sustaingineering 3G TxRx-ESP Version :D"));
  Serial.println(F("Initializing SIM800L communication with ESP32....(May take more than 10 seconds)"));
  
  delay(10000);

  // Make it slow so its easy to read!
  sim800lSerial->begin(4800, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
  if (!sim800l.begin(*sim800lSerial)) {
    Serial.println(F("Couldn't find GSM SIM800L"));
    while (1);
  } 
  Serial.println(F("GSM SIM800L is OK"));
  Serial.println(F("Searching for network...\n"));

  //below code checks and waits for network to be connected
  bool SIMFound= false;
  for (int countdown = 600; countdown >= 0 && SIMFound == false; countdown--)
  {
    uint8_t n = sim800l.getNetworkStatus(); // constantly check until network is connected to home sendCheckReply(F("AT+CLVL="), i, ok_reply);
    if (n == 1)
    {
      SIMFound = true;
      Serial.print(F("Found!")); // If program hangs here, SIM card cannot be read/connect to network
      Serial.println(F("Network Connected"));
    }
  }
  if (!SIMFound)
  {
    Serial.println(F("SIM card could not be found. Please ensure that your SIM card is compatible with dual-band UMTS/HSDPA850/1900MHz WCDMA + HSDPA."));
    while (1)
      ;
  }
  Serial.println("GSM SIM800L Ready"); //Everythingready!
}

void loop()
{
  delay(10000); //for now we will use a 10 second delay, and pass it values from 0->4
  float SourceVoltage=0;
  float HallAmps=1;
  float Power=2;
  float SolTemp=3;
  bool WaterBreakerFlag=4;
  send_sms(SourceVoltage, HallAmps, Power, SolTemp, WaterBreakerFlag);
  
}

void send_sms(float LoadVoltage, float LoadCurrent, float Power, float SolTemp, bool WaterBreakerFlag){
    //note, we may need to run rtc stamping inside this function according to Forbes
    char message[0]; 

    //for now we will not have the RTC timer included in this 
    String str;
    str = (String)(LoadVoltage) + "," + (String)(LoadCurrent) + "," + (String)(SolTemp); // "," (String)(WaterBreakerFlag);

    Serial.print("str content:");
    Serial.println(str);
    str.toCharArray(message,141); //do I need to change this string length

    Serial.print(F("Your message is: "));
    Serial.println(message);

    while (sim800l.sendSMS(sendto, message) == 0) //send sms can send strings. first string argument is our phone number, second string is the message
  {
    Serial.println(F("SMS sending failed."));
  }
  Serial.println(F("SMS sending succeeded."));
  Serial.println();
}