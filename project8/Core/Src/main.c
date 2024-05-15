
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

CAN_HandleTypeDef hcan;
UART_HandleTypeDef huart1;
uint8_t dataLength;
uint8_t sequenceNumber;
CAN_FilterTypeDef canFilterConfig;

void SystemClock_Config(void);

static void MX_CAN_Init(void);
static void MX_USART1_UART_Init(void);
typedef struct {
  uint32_t pgn;
  const char *name;
} PGN_Name;
typedef struct {
    uint32_t pgn;
    uint8_t sourceAddress;
    uint8_t dataLength;
    uint8_t data[8]; // Maximum size for CAN frame
} CanMessage;
// Lookup table mapping PGNs to their names
PGN_Name pgnNames[] = {
    {129029, "GNSS Position Data"},
    {130306, "Wind Data"},
    {126992L,"SystemTime"},
    {127245L,"Rudder"},
    {127250L,"Heading"},
    {127257L,"Attitude"},
    {127488L,"EngineRapid"},//it is alba
    {127489L,"EngineDynamicParameters"},//it is alba
    {127493L,"TransmissionParameters"},
    {127497L,"TripFuelConsumption"},
    {127501L,"BinaryStatus"},
    {127505L,"FluidLevel"},
    {127506L,"DCStatus"},
    {127513L,"BatteryConfigurationStatus"},
    {128259L,"Speed"},
    {128267L,"WaterDepth"},
    {129026L,"COGSOG"},
    {129029L,"GNSS"},
    {129033L,"LocalOffset"},
    {129045L,"UserDatumSettings"},
    {129540L,"GNSSSatsInView"},
    {130310L,"OutsideEnvironmental"},
    {130312L,"Temperature"},
    {130313L,"Humidity"},
    {130314L,"Pressure"},
    {130316L,"TemperatureExt"},//it is alba
  // Add more PGNs and their names as needed
};

// Function to get the name of a PGN
const char* getPGNName(uint32_t pgn) {
  for (int i = 0; i < sizeof(pgnNames) / sizeof(pgnNames[0]); i++) {
    if (pgnNames[i].pgn == pgn) {
      return pgnNames[i].name;
    }
  }
  return "Unknown PGN";  // Return a default name for unknown PGNs
}

int main(void) {
  HAL_Init();
  SystemClock_Config();

  MX_CAN_Init();
  MX_USART1_UART_Init();

  if (HAL_CAN_Start(&hcan) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
    Error_Handler();
  }

  while (1) {
    // Your application code here
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[32];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        // Extracting priority, PGN, source address, and device instance from the CAN ID
        uint8_t priority = (rxHeader.ExtId >> 26) & 0x07;
        uint32_t pgn = (rxHeader.ExtId >> 8) & 0x3FFFF;
        uint8_t sourceAddress = rxHeader.ExtId & 0xFF;
        uint8_t deviceInstance = (rxHeader.ExtId >> 18) & 0x1F;

        if (rxHeader.IDE == CAN_ID_EXT) {
            // Handle the extended CAN frame
            uint32_t extId = rxHeader.ExtId;
            sequenceNumber = (extId >> 18) & 0x1F;
            dataLength = (extId >> 8) & 0xFF; // Extract the data length from the extended ID
            if(dataLength <= 8){
                dataLength = rxHeader.DLC;
            }
        } else {
            dataLength = rxHeader.DLC; // Use the DLC value for standard CAN frames
        }

        // Determine the destination address based on the PGN
        uint8_t destinationAddress = 0xFF; // Default value, you may need to determine the destination address based on the PGN

        char buff[100];
        int lenraw = sprintf(buff, "Source: 0x%02X, Dest: 0x%02X, PGN: %lu, Priority: 0x%02X, Data Length: %u, Data: ", sourceAddress, destinationAddress, pgn, priority, dataLength);

        for (int i = 0; i < dataLength; i++) {
            lenraw += sprintf(&buff[lenraw], "%02X ", rxData[i]);
        }
        buff[lenraw++] = '\r';
        buff[lenraw++] = '\n';

        HAL_UART_Transmit(&huart1, (uint8_t*)buff, lenraw, HAL_MAX_DELAY);

        switch (pgn) {

                    case 130316: // Temperature
                                    {
                                        // Extract temperature data (assuming 12 bytes)
                                        uint16_t temperature1 = rxData[0] | (rxData[1] << 8);
                                        uint16_t temperature2 = rxData[2] | (rxData[3] << 8);
                                        uint16_t temperature3 = rxData[4] | (rxData[5] << 8);
                                        uint16_t temperature4 = rxData[6] | (rxData[7] << 8);
                                        char uartBuffer[100];
                                                            int len = snprintf(uartBuffer, sizeof(uartBuffer),
                                                                               "PGN: %lu, Temperature1: %u, Temperature2: %u, Temperature3: %u, Temperature4: %u\r\n",
                                                                               pgn, temperature1, temperature2, temperature3, temperature4);
                                                            HAL_UART_Transmit(&huart1, (uint8_t*)uartBuffer, len, HAL_MAX_DELAY);

                                        // Now you can use the extracted temperature data as needed
                                        // ...
                                    }

                    	// Handle these PGNs as before
                    	break;

                    case 127489: // Engine Dynamic Parameters
                        {
                            if (dataLength > 8) {
                                // Handle the extended data payload
                                uint8_t numFrames = (dataLength + 7) / 8; // Calculate the number of frames needed to transmit the data
                                uint8_t currentFrame = 0;
                                uint8_t remainingData = dataLength;

                                while (remainingData > 0) {
                                    uint8_t frameLength = (remainingData > 8) ? 8 : remainingData;
                                    char frameBuffer[200];
                                    int frameLen = sprintf(frameBuffer, "Source: 0x%02X, Dest: 0x%02X, PGN: %lu, Priority: 0x%02X, Device Instance: 0x%02X, Frame: %u/%u, Data: ",
                                                          sourceAddress, destinationAddress, pgn, priority, deviceInstance, currentFrame + 1, numFrames);

                                    for (int i = 0; i < frameLength; i++) {
                                        frameLen += sprintf(&frameBuffer[frameLen], "%02X ", rxData[currentFrame * 8 + i]);
                                    }

                                    frameBuffer[frameLen++] = '\r';
                                    frameBuffer[frameLen++] = '\n';
                                    HAL_UART_Transmit(&huart1, (uint8_t*)frameBuffer, frameLen, HAL_MAX_DELAY);

                                    currentFrame++;
                                    remainingData -= frameLength;
                                }
                            } else {
                                // Extract the data from the first 8 bytes
                                uint16_t engineOilPressure = rxData[2] | (rxData[3] << 8);
                                int16_t engineCoolantTemperature = rxData[4] | (rxData[5] << 8);
                                uint16_t engineSpeed = rxData[0] | (rxData[1] << 8);
                                uint32_t engineHours = (rxData[6] | (rxData[7] << 8)) * 50;

                                // Get the additional data for PGN 127489
                                uint16_t engineTorque = 0;
                                uint16_t fuelRate = 0;
                                uint16_t engineCoolantPressure = 0;

                                if (dataLength > 8) {
                                    // Read the additional data
                                    engineTorque = rxData[8] | (rxData[9] << 8);
                                    fuelRate = rxData[10] | (rxData[11] << 8);
                                    engineCoolantPressure = rxData[12] | (rxData[13] << 8);
                                }

                                // Print the extracted information over UART
                                char buffer[200];
                                int len = sprintf(buffer, "Source: 0x%02X, Dest: 0x%02X, PGN: %lu, Priority: 0x%02X, Device Instance: 0x%02X, Data Length: %u\r\n"
                                                          "Oil Pressure: %u kPa, Coolant Temp: %d°C\r\n"
                                                          "Engine Speed: %u RPM, Engine Hours: %u h\r\n"
                                                          "Engine Torque: %u Nm, Fuel Rate: %u L/h\r\n"
                                                          "Coolant Pressure: %u kPa\r\n",
                                                 sourceAddress, destinationAddress, pgn, priority, deviceInstance, dataLength,
                                                 engineOilPressure, engineCoolantTemperature,
                                                 engineSpeed, engineHours,
                                                 engineTorque, fuelRate,
                                                 engineCoolantPressure);
                                HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, HAL_MAX_DELAY);
                            }
                        }
                        break;

                    default:
                    	// Handle other PGNs as needed
                    	break;
        }
    }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_CAN_Init(void) {
    hcan.Instance = CAN1;
    hcan.Init.Prescaler = 8;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_10TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_7TQ;
    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = DISABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = DISABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;
    //hcan.Init.ExtendedMode = CAN_EXTENDEDFRAME; // Enable extended ID format
    if (HAL_CAN_Init(&hcan) != HAL_OK) {
        Error_Handler();
    }

  canFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
  canFilterConfig.FilterBank = 0;
  canFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  canFilterConfig.FilterIdHigh = 0x0000;
  canFilterConfig.FilterIdLow = 0x0000;
  canFilterConfig.FilterMaskIdHigh = 0x0000;
  canFilterConfig.FilterMaskIdLow = 0x0000;
  canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canFilterConfig.SlaveStartFilterBank = 14;
  if (HAL_CAN_ConfigFilter(&hcan, &canFilterConfig) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void) {
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
}

void Error_Handler(void) {
  while (1) {
    // Error handling code
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
  // User can add his own implementation to report the file name and line number
}
#endif
