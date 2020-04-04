/*
This header contains a customized library for the operation of 
the GSM module.
Jaiden Martinson-Hatt
Sustaingeering UBC

Note: Many sections of code are commented out as I do not have the RTC timer hardware with me.
*/

//dependencies 
#include "Adafruit_FONA.h" 
//#include "RTClib.h" //for when we attach the RTC Hardware

class farmGSM
{
private:
    //fona, serial and rtc constructors
    HardwareSerial *sim800lSerial;
    Adafruit_FONA sim800l;
    //RTC_DS1307 rtc; //For when we attach the RTC hardware.

    //private member variables
    const int SIM800L_RX;
    const int SIM800L_TX;
    const int SIM800L_PWRKEY;
    const int SIM800L_RST;
    const int SIM800L_POWER;
    char sendto[21]="7786684301"; //hard coded for now

    //receive variables
    char replybuffer[255];
    int timecounts = 0;
    int last_timecounts = 0;

    //unint8_t stuff (hopefully this works)
    uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0); 
    uint8_t type;
    volatile int8_t numsms; 

    //private member functions 
    void check_get_sms();
    void delete_SMS_all();
    boolean time_sms();

    

public:
    //public member functions for GSM operation
    farmGSM(int, int, int, int, int); // default constructor
    void initialize_sending();
    void initialize_receiving();
    void send_sms(float LoadVoltage, float LoadCurrent, float Power, float SolTemp, bool WaterBreakerFlag);
    void receive_sms(); //for receiving
};

/***************************************************************************************************************************************/
farmGSM::farmGSM(int rx, int tx, int pwrkey, int rst, int power):
 sim800lSerial(&Serial1) 
,sim800l(pwrkey)
,SIM800L_RX(rx)
,SIM800L_TX(tx)
,SIM800L_PWRKEY(pwrkey)
,SIM800L_RST(rst)
,SIM800L_POWER(power)
{}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
void farmGSM::initialize_sending()
{
    //Turn Sim800L on + Initialize Serial                 
    //pin declarations
    pinMode(SIM800L_POWER, OUTPUT);
    digitalWrite(SIM800L_POWER, HIGH);

    Serial.begin(115200);
    Serial.println(F("Welcome to Sustaingineering 3G TxRx-ESP Version :D"));
    Serial.println(F("Initializing SIM800L communication with ESP32....(May take more than 10 seconds)"));
  
    delay(10000);

    
    /*
    //Set up the RTC timer module                    
     rtc.begin();

    if (! rtc.isrunning()) {
      Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
     Serial.println(F("RTC Timer synced with real time"));
   */
   
    //Initialize SIM800L for sending
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
  Serial.println("GSM SIM800L Ready to send"); //Everythingready!
}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
void farmGSM::send_sms(float LoadVoltage, float LoadCurrent, float Power, float SolTemp, bool WaterBreakerFlag)
{
   // DateTime now = rtc.now();
   //fix hour, minute, and seconds format since they are only integers in the RTC library
  // String rtc_hour = ((now.hour() >= 10) ? (String)(now.hour()) : "0" + (String)(now.hour()));
  // String rtc_minute = ((now.minute() >= 10) ? (String)(now.minute()) : "0" + (String)(now.minute()));
  // String rtc_second = ((now.second() >= 10) ? (String)(now.second()) : "0" + (String)(now.second()));

    char message[0]; 

    //for now we will not have the RTC timer included in this 
    String str;
    //without timestamp from RTC
    str = (String)(LoadVoltage) + "," + (String)(LoadCurrent) + "," + (String)(SolTemp) + "," + (String)(WaterBreakerFlag);
    //uncomment for timestamp with RTC timer
    // str = (String)(now.unixtime()) + "," + (String)(LoadVoltage) + "," + (String)(LoadCurrent) + "," + (String)(Power) + "," + (String)(SolTemp) + "," + (String)(WaterBreakerFlag);
    
    Serial.print("str content:");
    Serial.println(str);
    str.toCharArray(message,141); 

    Serial.print(F("Your message is: "));
    Serial.println(message);

    while (sim800l.sendSMS(sendto, message) == 0) //send sms can send strings. first string argument is our phone number, second string is the message
  {
    Serial.println(F("SMS sending failed."));
  }
    Serial.println(F("SMS sending succeeded."));
    Serial.println();
}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
void farmGSM::initialize_receiving(){
    //Turn Sim800L on + Initialize Serial
    //pin declarations
    pinMode(SIM800L_POWER, OUTPUT);
    digitalWrite(SIM800L_POWER, HIGH);

    Serial.begin(115200);
    Serial.println(F("Welcome to Sustaingineering 3G TxRx-ESP Version :D"));
    Serial.println(F("Initializing SIM800L communication with ESP32....(May take more than 10 seconds)"));
  
    delay(10000);


    //Initialize SIM800L for receiving                   
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
      numsms = sim800l.getNumSMS();
      if (numsms != -1){
        Serial.print(F("Number of messages:"));
        Serial.println(numsms);
        break;
      }
    }
    }
     if (!SIMFound)
    {
        Serial.println(F("SIM card could not be found. Please ensure that your SIM card is compatible with dual-band UMTS/HSDPA850/1900MHz WCDMA + HSDPA."));
        while (1)
      ;
  }
  
  Serial.println("GSM SIM800L Ready to receive"); //Everythingready!
}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
void farmGSM::receive_sms(){
  if(numsms !=0){
    Serial.print(F("Number of messages: "));
    Serial.println(numsms);
    Serial.println(F("Delete all SMS"));
    delete_SMS_all();
  }
  while(!Serial.available()){
    if(time_sms())
    check_get_sms();
  }
}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
void farmGSM::check_get_sms(){
    Serial.println(F("Start check_get_sms"));
    if (numsms != sim800l.getNumSMS()) //we might get an "AT+CMGR=255" fail code, but this is nothing of deep concern
  {
    numsms = sim800l.getNumSMS();
    uint8_t smsn = numsms - 1; // the sms# (aka SMS index) starts from 0
    
    if (!sim800l.getSMSSender(smsn, replybuffer, 250))
    {
      Serial.println(F("Failed!")); 
      return;
    }
    Serial.print(F("FROM: "));
    Serial.println(replybuffer);
    
    //retrieve SMS value.
    uint16_t smslen;
    if (!sim800l.readSMS(smsn, replybuffer, 250, &smslen))
    { //pass in buffer and max len!
      Serial.println(F("Failed!"));
      return;
    }
    /*
       Data is sent in csv format. The data will appear as follows:
       RTC Time, LoadVoltage, LoadCurrent, Power, AtmTemp, SolTemp, WaterBreakerFlag
    */

    //retrieve delimited values for use
    char *vars[5];
    vars[0] = strtok(replybuffer, ",");
    for (int i = 1; i < 6; i++)
      vars[i] = strtok(NULL, ",");

    //Serial print results
    Serial.print(F("RTC Time [YYYY/MM/DD-hh:mm:ss]: "));
    Serial.println(vars[0]);
    Serial.print(F("Load Voltage [V]: "));
    Serial.println(vars[1]);
    Serial.print(F("Load Current [A]: "));
    Serial.println(vars[2]);
    Serial.print(F("Power [W]: "));
    Serial.println(vars[3]);
    Serial.print(F("Solar Panel Temperature [^oC]: "));
    Serial.println(vars[4]);
    Serial.print(F("Water Breaker Flag: "));
    Serial.println(vars[5]);

    //Since all messages are deleted in the beginning, we just need to delete the current message at index 0
    if (sim800l.deleteSMS(0))
      Serial.println(F("Deleted SMS index #0"));
    else
      Serial.println(F("Couldn't delete"));
  }
}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
void farmGSM::delete_SMS_all(){
   //delete all SMS
  int max_SMS_num = 50;
  
  for (int i = 0; i < max_SMS_num; i++)
  {
    if (sim800l.deleteSMS(i))
      Serial.println(F("Deleted SMS index # "));
    else
      Serial.println(F("Couldn't delete"));
  }
  Serial.println(F("All messages are cleared"));
}
/***************************************************************************************************************************************/

/***************************************************************************************************************************************/
boolean farmGSM::time_sms(){
  // use millis to check the number of sms every second.
  timecounts = millis();
  if (timecounts > last_timecounts + 1000)
  {
    last_timecounts = timecounts;
    return 1;
  }
  else
    return 0;
}
/***************************************************************************************************************************************/


