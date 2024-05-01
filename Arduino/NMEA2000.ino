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

static CAN_message_t CAN_RX_msg;
uint8_t CT = 1;
void setup() {
  Serial.begin(115200);
  Can.begin();
  //Can.setBaudRate(250000);  //250KBPS
  Can.setBaudRate(250000);  //500KBPS
  //Can.setBaudRate(1000000);  //1000KBPS
}

void loop() {
  if (Can.read(CAN_RX_msg)) {
    Serial.print("Channel: ");
    Serial.print(CAN_RX_msg.bus);
    if (CAN_RX_msg.flags.extended == false) {
      Serial.print(" Standard ID: ");
    }
    else {
      Serial.print(" Extended ID: ");
    }
    Serial.print(CAN_RX_msg.id, HEX);

    Serial.print(" DLC: ");
    Serial.print(CAN_RX_msg.len);

    if (CAN_RX_msg.flags.remote == false) {
      // Extract NMEA2000 message information
      uint32_t pgn = (CAN_RX_msg.id >>8)& 0x1FFFF;
      uint8_t priority = (CAN_RX_msg.id >> 26) & 0x7;
      uint8_t source = CAN_RX_msg.id & 0xFF;
      uint8_t destination = 0xFF;

      Serial.print(" Pri: ");
      Serial.print(priority);
      Serial.print(" PGN: ");
      Serial.print(pgn, DEC);
      Serial.print(" Srce: ");
      Serial.print(source, HEX);
      Serial.print(" Dest: ");
      Serial.println(destination, HEX);

      Serial.print(" Data: ");
      for (int i = 0; i < CAN_RX_msg.len; i++) {
        Serial.print("0x");
        Serial.print(CAN_RX_msg.buf[i], HEX);
        if (i != (CAN_RX_msg.len - 1)) Serial.print(" ");
      }
      Serial.println();

      // Process NMEA2000 PGN
      switch (pgn) {
          
          case 127488: {// Engine Parameters, Rapid Update
            // Extract Engine Parameters, Rapid Update
            float engine_boost_pressure = (CAN_RX_msg.buf[1] << 8) | CAN_RX_msg.buf[0];
            engine_boost_pressure /= 100.0; // Convert to kPa
            float engine_fuel_rate = (CAN_RX_msg.buf[3] << 8) | CAN_RX_msg.buf[2];
            engine_fuel_rate /= 0.05; // Convert to L/h
            float engine_turbo_boost = (CAN_RX_msg.buf[5] << 8) | CAN_RX_msg.buf[4];
            engine_turbo_boost /= 0.05; // Convert to kPa
            Serial.print("Engine Boost Pressure: ");
            Serial.print(engine_boost_pressure, 2);
            Serial.println(" kPa");
            Serial.print("Engine Fuel Rate: ");
            Serial.print(engine_fuel_rate, 2);
            Serial.println(" L/h");
            Serial.print("Engine Turbo Boost: ");
            Serial.print(engine_turbo_boost, 2);
            Serial.println(" kPa");
            break;
          }
          case 127489: {// Engine Parameters, Dynamic

            switch (CT) {
              case 1: {
                
                uint8_t engine_instance = CAN_RX_msg.buf[0] & 0xFF;
                Serial.print("Instance: ");
                Serial.println(engine_instance);

                uint16_t engine_oil_pressure = (CAN_RX_msg.buf[2] << 8) | CAN_RX_msg.buf[1];
                engine_oil_pressure *= 100; // Convert to Pa
                Serial.print("Oil Pressure: ");
                Serial.print(engine_oil_pressure);
                Serial.println(" Pa");

                int16_t engine_oil_temp = (CAN_RX_msg.buf[4] << 8) | CAN_RX_msg.buf[3];
                float engine_oil_temp_celsius = engine_oil_temp / 10.0; // Convert to 0.1 K
                Serial.print("Oil Temperature: ");
                Serial.print(engine_oil_temp_celsius);
                Serial.println(" °C");

                int16_t engine_temp = (CAN_RX_msg.buf[6] << 8) | CAN_RX_msg.buf[5];
                float engine_temp_celsius = engine_temp / 100.0; // Convert to 0.01 K
                Serial.print("Temperature: ");
                Serial.print(engine_temp_celsius);
                Serial.println(" °C");
                CT=2;
                break;
              }
              case 2: {
                
                int16_t alternator_potential = (CAN_RX_msg.buf[1] << 8) | CAN_RX_msg.buf[0];
                float alternator_potential_volts = alternator_potential / 100.0; // Convert to 0.01 V
                Serial.print("Alternator Potential: ");
                Serial.print(alternator_potential_volts);
                Serial.println(" V");

                int16_t fuel_rate = (CAN_RX_msg.buf[3] << 8) | CAN_RX_msg.buf[2];
                float fuel_rate_lph = fuel_rate / 10.0; // Convert to 0.1 L/h
                Serial.print("Fuel Rate: ");
                Serial.print(fuel_rate_lph);
                Serial.println(" L/h");

                uint32_t total_engine_hours = (CAN_RX_msg.buf[7] << 24) | (CAN_RX_msg.buf[6] << 16) | (CAN_RX_msg.buf[5] << 8) | CAN_RX_msg.buf[4];
                Serial.print("Total Engine Hours: ");
                Serial.print(total_engine_hours);
                Serial.println(" s");
                CT=3;
                break;
              }
              case 3: {
                
                uint16_t coolant_pressure = (CAN_RX_msg.buf[1] << 8) | CAN_RX_msg.buf[0];
                coolant_pressure *= 100; // Convert to Pa
                Serial.print("Coolant Pressure: ");
                Serial.print(coolant_pressure);
                Serial.println(" Pa");

                uint16_t fuel_pressure = (CAN_RX_msg.buf[3] << 8) | CAN_RX_msg.buf[2];
                fuel_pressure *= 1000; // Convert to Pa
                Serial.print("Fuel Pressure: ");
                Serial.print(fuel_pressure);
                Serial.println(" Pa");
                CT=4;
                break;
              }
              case 4: {
                CT=0;
                break;
                // TODO: Handle Reserved, Discrete Status 1, Discrete Status 2, Engine Load, and Engine Torque
              }
            }
          }
          case 128259: {// Speed
            // Extract Speed
            float water_speed = (CAN_RX_msg.buf[1] << 8) | CAN_RX_msg.buf[0];
            water_speed /= 100.0; // Convert to knots
            float ground_speed = (CAN_RX_msg.buf[3] << 8) | CAN_RX_msg.buf[2];
            ground_speed /= 100.0; // Convert to knots
            Serial.print("Water Speed: ");
            Serial.print(water_speed, 2);
            Serial.println(" knots");
            Serial.print("Ground Speed: ");
            Serial.print(ground_speed, 2);
            Serial.println(" knots");
            break;
          }
          case 127493: {// Engine Transmission Parameters
            // Extract Engine Transmission Parameters
            float engine_percent_load = CAN_RX_msg.buf[0];
            engine_percent_load /= 2.55; // Convert to percentage
            float transmission_oil_pressure = (CAN_RX_msg.buf[2] << 8) | CAN_RX_msg.buf[1];
            transmission_oil_pressure /= 100.0; // Convert to PSI
            Serial.print("Engine Percent Load: ");
            Serial.print(engine_percent_load, 2);
            Serial.println(" %");
            Serial.print("Transmission Oil Pressure: ");
            Serial.print(transmission_oil_pressure, 2);
            Serial.println(" PSI");
            break;
          }
          case 130316: {// Temperature
            // Extract Temperature
            float outside_temp = CAN_RX_msg.buf[0] - 40.0; // Convert to Celsius
            float inside_temp = CAN_RX_msg.buf[1] - 40.0; // Convert to Celsius
            Serial.print("Outside Temperature: ");
            Serial.print(outside_temp, 1);
            Serial.println(" °C");
            Serial.print("Inside Temperature: ");
            Serial.print(inside_temp, 1);
            Serial.println(" °C");
            break;
          }
          case 130312: {// Temperature
            // Extract Temperature
            float bait_well_temp = CAN_RX_msg.buf[0] - 40.0; // Convert to Celsius
            float freezer_temp = CAN_RX_msg.buf[1] - 40.0; // Convert to Celsius
            float live_well_temp = CAN_RX_msg.buf[2] - 40.0; // Convert to Celsius
            Serial.print("Bait Well Temperature: ");
            Serial.print(bait_well_temp, 1);
            Serial.println(" °C");
            Serial.print("Freezer Temperature: ");
            Serial.print(freezer_temp, 1);
            break;
          }
          case 0x01FD00: {// Request
            // Process Request PGN
            break;
          }
          // Add more cases for other PGNs as needed
          default:{
            break;
          }
        }
      } 
    }
  }

