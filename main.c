
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

CAN_HandleTypeDef hcan;
UART_HandleTypeDef huart1;

CAN_FilterTypeDef canFilterConfig;

void SystemClock_Config(void);

static void MX_CAN_Init(void);
static void MX_USART1_UART_Init(void);
typedef struct {
  uint32_t pgn;
  const char *name;
} PGN_Name;

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
    uint8_t rxData[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        // Extracting source address, destination address, PGN, priority, and data length
    	uint8_t priority = (rxHeader.ExtId >> 26) & 0x07;
    	        uint32_t pgn = (rxHeader.ExtId >> 8) & 0x3FFFF;
    	        uint8_t sourceAddress = rxHeader.ExtId & 0xFF;

    	        // Extracting destination address and data from the data payload
    	        uint8_t destinationAddress = rxData[0];
    	        uint8_t dataLength = rxHeader.DLC - 1; //

        // Print the extracted information over UART
        char buffer[100];
        int len = sprintf(buffer, "Priority: 0x%02X, PGN: 0x%06X, Source: 0x%02X, Dest: 0x%02X, Data Length: 0x%02X, Data: ", priority, pgn, sourceAddress, destinationAddress, dataLength);

        for (int i = 0; i < dataLength; i++) {
            len += sprintf(&buffer[len], "%02X ", rxData[i]);
        }
        buffer[len++] = '\r';
        buffer[len++] = '\n';

        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, HAL_MAX_DELAY);
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
25april.txt
Displaying 25april.txt.
