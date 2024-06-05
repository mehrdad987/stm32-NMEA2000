#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
//#include <SD.h>
#include "STM32_CAN.h"

// Pin definitions
/*#define MOSI_PIN PA7
#define MISO_PIN PA6
#define SCK_PIN  PA5
#define CS_PIN   PA4*/

STM32_CAN Can( CAN1, DEF );  //Use PA11/12 pins for CAN1.
uint32_t pgn;
/*-------------------------------------------*/
//char timeStr[16]; // Buffer to hold the time string

uint8_t buffer58[8];
int chekifitiscalled=0;
//uint8_t safeguard_buffer[8];
static CAN_message_t CAN_RX_msg;
//const char* timeStr2;
//char data_to_write[200]; // Increase the size as needed
char fileName[15];
/*-------------------------------------------*/
// Global engine data variables

uint8_t checkpgn201 = 0;

uint8_t engineInstance = 0;
uint8_t engineInstancerapid = 0;
float rpm = 0.0f;
uint16_t engineBoostPressure = 0;
int16_t engineTiltTrimAngle = 0;
uint8_t batteryInstance = 0;
float batteryVoltage = 0.0f;
float batteryCurrent = 0.0f;
float batteryTemperature = 0.0f;
uint8_t chargerInstance = 0;
uint32_t pressure = 0;
uint8_t transmissionInstance = 0;
uint8_t transmissionGear = 0;
uint16_t oilPressureTR = 0;
uint16_t oilTemperatureTR = 0;
uint16_t oilPressure = 0;
uint16_t oilTemperature = 0;
float engineTemperature = 0.0f;
float alternatorPotential = 0.0f;
float fuelRate = 0.0f;
uint32_t totalEngineHours = 0;
uint16_t coolantpressure = 0;
uint16_t fuelPressure = 0;
uint8_t EngineLoad = 0;
uint8_t EngineTorque = 0;
uint16_t batteryVoltageRaw = 0;
int16_t batteryCurrentraw = 0;
int16_t batteryTemperatureraw = 0;
uint16_t engineTemperatureRaw= 0;
uint16_t alternatorPotentialRAW = 0;
uint16_t fuelRateRAW = 0;
/*-------------------------------------------*/
RTC_DS3231 rtc;
File dataFile;
String dataStringtime;


unsigned long previousMillis = 0;
const unsigned long interval = 1000; // 3 seconds

void setup() {
  Serial.begin(115200);

  // Initialize the SPI pins
 /* pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);*/

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
//to one time adjustment
  //rtc.adjust(DateTime(__DATE__, __TIME__));
  //str
  //DateTime now = rtc.now();
 
  Can.begin();
  Can.setBaudRate(250000);  //250KBPS
  DateTime now = rtc.now();

}

void loop() {
  delay(10);
  TaskReadCAN();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    TaskTime();
  }
}

void TaskReadCAN() {
 
    Serial.println("                                                                                     TaskReadCAN");
    delay(10);

    unsigned long currentMillis = millis();
    if (Can.read(CAN_RX_msg) ) {
      pgn = ((CAN_RX_msg.id >> 8) & 0x1FFFF);
      Serial.print(pgn, HEX);
      if (CAN_RX_msg.flags.remote == false) {
        Serial.print(" buf: ");
        for(int i=0; i<CAN_RX_msg.len; i++) {
          //Serial.print("0x"); 
          
          Serial.print(CAN_RX_msg.buf[i], HEX); 
          //safeguard_buffer[i]=CAN_RX_msg.buf[i];
          //string2 =string1 + (string2,"0x%02X", CAN_RX_msg.buf[i]);
          if (i != (CAN_RX_msg.len-1))  Serial.print(" ");
        }
        Serial.println(" ");
        
      } else {
        Serial.println(" Data: REMOTE REQUEST FRAME");
    }
      if (pgn == 0x1F201) {
        // PGN 0x1F201 (Engine Parameters)
        // Data format:
        // Byte 0: Engine Instance
        // Byte 1-2: Oil Pressure (kPa)
        // Byte 3-4: Oil Temperature (°C)
        // Byte 5-6: Engine Temperature (°C)
        chekifitiscalled++;
        
        Serial.print(chekifitiscalled);
        if(chekifitiscalled == 1){
          Serial.println("pgn201=1");
          checkpgn201= CAN_RX_msg.buf[0];
          engineInstancerapid = CAN_RX_msg.buf[2];
          oilPressure = (CAN_RX_msg.buf[4] << 8) | CAN_RX_msg.buf[3];
          oilTemperature = (CAN_RX_msg.buf[6] << 8) | CAN_RX_msg.buf[5];
          buffer58[0] = CAN_RX_msg.buf[7];
          
        }else if ( (checkpgn201+1) == CAN_RX_msg.buf[0]){
          Serial.println("pgn201=2");
          buffer58[1] = CAN_RX_msg.buf[1];
          engineTemperatureRaw= (buffer58[0] << 8) | buffer58[1];
          alternatorPotentialRAW = (CAN_RX_msg.buf[3] << 8) | CAN_RX_msg.buf[2];
          fuelRateRAW = (CAN_RX_msg.buf[5] << 8) | CAN_RX_msg.buf[4];
          buffer58[2]=CAN_RX_msg.buf[6];
          buffer58[3]=CAN_RX_msg.buf[7];
 
        }else if ((checkpgn201+2) == CAN_RX_msg.buf[0]){
          Serial.println("pgn201=3");
          buffer58[4]=CAN_RX_msg.buf[1];
          buffer58[5]=CAN_RX_msg.buf[2];
          totalEngineHours = (buffer58[5] << 24) | (buffer58[4] << 16) | (buffer58[3] << 8) | buffer58[2];
          coolantpressure = (CAN_RX_msg.buf[4] << 8) | CAN_RX_msg.buf[3];
          fuelPressure = (CAN_RX_msg.buf[6] << 8) | CAN_RX_msg.buf[5];

          //delete[] buffer58;

        }else if ( (checkpgn201+3) == CAN_RX_msg.buf[0]){
          Serial.println("pgn201=4");
          chekifitiscalled =0;
          EngineLoad = CAN_RX_msg.buf[5];
          EngineTorque = CAN_RX_msg.buf[6];
          
        }
      } else {
        chekifitiscalled =0;
        checkpgn201=0;
        // Handle other PGN types as needed
        //Serial.print("Unsupported PGN: 0x");
        //Serial.println(pgn, HEX);
      }
    }
    
}

void TaskTime() {
  
    Serial.println("                          TaskTime.S");
    DateTime now = rtc.now();
    dataStringtime = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) + ", ";
    Serial.println(dataStringtime);
    Serial.println("                          TaskTime.E");
    
}
