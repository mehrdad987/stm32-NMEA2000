going to edit

''
#proj

void decodeNMEA2000Message(uint8_t *data) {
  uint8_t priority = (data[0] >> 2) & 0x07;  // Extract priority from the first byte
  uint32_t PGN = ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];  // Extract PGN from bytes 2, 3, and 4
  uint8_t destination = data[4];  // Extract destination address
  uint8_t source = data[5];  // Extract source address
  // ... Decode other relevant data fields from the message

  // Placeholder for storing the raw data payload as a uint32_t variable
  uint32_t rawData = 0;
  for (int i = 0; i < 4; i++) {
    rawData |= ((uint32_t)data[i + 6] << (i * 8));  // Concatenate the bytes into the rawData variable
  }

  // Get the name of the PGN
  const char* pgnName = getPGNName(PGN);

  // Print the decoded information including the PGN name and the raw data payload to the serial interface
  char buffer[150];
  sprintf(buffer, "Decoded NMEA 2000 message - Priority: %d, PGN: %lu (%s), Destination: %d, Source: %d, Raw Data: %lu\r\n",
          priority, PGN, pgnName, destination, source, rawData);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}
'
