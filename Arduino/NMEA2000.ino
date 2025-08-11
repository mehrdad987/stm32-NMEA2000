/*
This is simple example to read all data from CAN bus and print it out to serial bus.
*/

#include "STM32_CAN.h"
STM32_CAN Can( CAN1, DEF );  //Use PA11/12 pins for CAN1.
//STM32_CAN Can( CAN1, ALT );  //Use PB8/9 pins for CAN1.
//STM32_CAN Can( CAN1, ALT_2 );  //Use PD0/1 pins for CAN1.
//STM32_CAN Can( CAN2, DEF );  //Use PB12/13 pins for CAN2.
//STM32_CAN Can( CAN2, ALT );  //Use PB5/6 pins for CAN2
//STM32_CAN Can( CAN3, DEF );  //Use PA8/15 pins for CAN3.
//STM32_CAN Can( CAN3, ALT );  //Use PB3/4 pins for CAN3
float rpm;

static CAN_message_t CAN_RX_msg;

void setup() {
  Serial.begin(115200);
  Can.begin();
  //Can.setBaudRate(250000);  //250KBPS
  Can.setBaudRate(500000);  //500KBPS
  //Can.setBaudRate(1000000);  //1000KBPS
}

void loop() {
  if (Can.read(CAN_RX_msg) ) {
    Serial.print("Channel:");
    Serial.print(CAN_RX_msg.bus);
    if (CAN_RX_msg.flags.extended == false) {
      Serial.print(" Standard ID:");
    }
    else {
      Serial.print(" pgnExt:");
    }
    
    uint32_t pgn = ((CAN_RX_msg.id >> 8) & 0x1FFFF);
    Serial.print(pgn, HEX);

    Serial.print(" DLC: ");
    Serial.print(CAN_RX_msg.len);
    if (CAN_RX_msg.flags.remote == false) {
       Serial.print(" buf: ");
      for(int i=0; i<CAN_RX_msg.len; i++) {
        Serial.print("0x"); 
        Serial.print(CAN_RX_msg.buf[i], HEX); 
        if (i != (CAN_RX_msg.len-1))  Serial.print(" ");
      }
      Serial.println();
    } else {
       Serial.println(" Data: REMOTE REQUEST FRAME");
    }
  if (pgn == 0x1F200) {
      // PGN 0x1F200 (Engine Parameters, Rapid Update)
      // Data format:
      // Byte 0: Engine Instance
      // Byte 1-2: Engine Speed (RPM)
      // Byte 3-4: Engine Boost Pressure (kPa)
      // Byte 5-6: Engine Tilt/Trim Angle (degrees)

      uint8_t engineInstance = (CAN_RX_msg.buf[0]);
      uint16_t engineSpeed = (CAN_RX_msg.buf[2] << 8) | (CAN_RX_msg.buf[1]);
      if(engineSpeed==0xFF){engineSpeed==0;}else {
        rpm = (engineSpeed *0.25);
      }
      uint16_t engineBoostPressure = (CAN_RX_msg.buf[4] << 8) | (CAN_RX_msg.buf[3]);
      if(engineBoostPressure==0xFF){engineBoostPressure==0;}else {
      }
      int16_t engineTiltTrimAngle = (CAN_RX_msg.buf[5]);
      if(engineTiltTrimAngle==0xFF){engineTiltTrimAngle==0;}else {
      }
      Serial.print("Engine Instance: ");
      Serial.println(engineInstance);
      Serial.print("Engine Speed (RPM): ");
      Serial.println(rpm);
      Serial.print("Engine Boost Pressure (kPa): ");
      Serial.println(engineBoostPressure);
      Serial.print("Engine Tilt/Trim Angle (degrees): ");
      Serial.println(engineTiltTrimAngle);
    } 
  } else {
      // Handle other PGN types as needed
      Serial.print("Unsupported PGN: 0x");
      Serial.println(pgn, HEX);
    }
  }
}
