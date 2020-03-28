/*
This header contains a customized library for the operation of 
the GSM module.
Jaiden Martinson-Hatt
Sustaingeering UBC
*/

//dependencies 
#include "Adafruit_FONA.h" 

class farmGSM
{
private:
    //fona and serial constructor classes
    HardwareSerial *sim800lSerial;
    Adafruit_FONA sim800l;

    //private variables for pin assignments
    const int SIM800L_RX;
    const int SIM800L_TX;
    const int SIM800L_PWRKEY;
    const int SIM800L_RST;
    const int SIM800L_POWER;
    char sendto[21]="7786684301"; //hard coded for now

public:
    //public member functions for GSM operation
    farmGSM(int, int, int, int, int); // default constructor
    void initialize_sending();
    void send_sms(float LoadVoltage, float LoadCurrent, float Power, float SolTemp, bool WaterBreakerFlag);

};


farmGSM::farmGSM(int rx, int tx, int pwrkey, int rst, int power):
 sim800lSerial(&Serial1) //is this right?
,sim800l(pwrkey)
,SIM800L_RX(rx)
,SIM800L_TX(tx)
,SIM800L_PWRKEY(pwrkey)
,SIM800L_RST(rst)
,SIM800L_POWER(power)
{}

void farmGSM::initialize_sending()
{
    //pin declarations
    pinMode(SIM800L_POWER, OUTPUT);
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

void farmGSM::send_sms(float LoadVoltage, float LoadCurrent, float Power, float SolTemp, bool WaterBreakerFlag)
{
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