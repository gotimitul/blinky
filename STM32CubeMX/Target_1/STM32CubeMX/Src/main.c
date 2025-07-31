/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
#include "EventRecorder.h"
//#include "USBD_STM32.h"
#include "usbd_cdc_if.h"
#include "GPIO_STM32.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern ARM_DRIVER_GPIO Driver_GPIO0;
osThreadId_t tid1;
osThreadId_t tid2;
osThreadId_t tid3;
osThreadId_t tid4;
osThreadId_t tid5;
static osEventFlagsId_t evt_id = 0;  

osMessageQueueId_t mid1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void led_blue (void *argument) {

  // ...
	for(;;)	{
	osStatus_t status;                    // capture the return status
  uint32_t   delayTime;                 // delay time in milliseconds

		static uint32_t pin_state = 0U;
	if (pin_state == 0)	pin_state = 1U;
		else pin_state = 0U;
	Driver_GPIO0.SetOutput(63, pin_state);
//	HAL_GPIO_TogglePin(LED_Blue_GPIO_Port, LED_Blue_Pin);		
		
		GPIO_PinState led_state = Driver_GPIO0.GetInput(63); //HAL_GPIO_ReadPin(LED_Blue_GPIO_Port, LED_Blue_Pin);
		char *text;
		if (led_state == GPIO_PIN_RESET)
		{
			text = "Blue LED ON\r\n";
		}
		else 
		{
			text = "Blue LED OFF\r\n";
		}
		osMessageQueuePut(mid1, text, 3, 0U);

  delayTime = 4000U;                    // delay 1 second
  status = osDelay(delayTime);          // suspend thread execution
  }
}
void led_red (void *argument) {
	osDelay(1000);
  // ...
	
	for(;;)	{
	osStatus_t status;                    // capture the return status
  uint32_t   delayTime;                 // delay time in milliseconds

	static uint32_t pin_state = 0U;
	if (pin_state == 0)	pin_state = 1U;
		else pin_state = 0U;
	Driver_GPIO0.SetOutput(62, pin_state);
//	HAL_GPIO_TogglePin(LED_Red_GPIO_Port, LED_Red_Pin);
	
	GPIO_PinState led_state = HAL_GPIO_ReadPin(LED_Red_GPIO_Port, LED_Red_Pin);
		char *text;
		if (led_state == GPIO_PIN_RESET)
		{
			text = "Red LED ON\r\n";
		}
		else 
		{
			text = "Red LED OFF\r\n";
		}
		osMessageQueuePut(mid1, text, 1, 0U);
  delayTime = 4000U;                    // delay 1 second
  status = osDelay(delayTime);          // suspend thread execution
  }
}
void led_orange (void *argument) {
 	osDelay(2000);
  // ...
	for(;;)	{
	osStatus_t status;                    // capture the return status
  uint32_t   delayTime;                 // delay time in milliseconds
	
	static uint32_t pin_state = 0U;
	if (pin_state == 0)	pin_state = 1U;
		else pin_state = 0U;
	Driver_GPIO0.SetOutput(61, pin_state);
		
//	HAL_GPIO_TogglePin(LED_Orange_GPIO_Port, LED_Orange_Pin);
		
		GPIO_PinState led_state = HAL_GPIO_ReadPin(LED_Orange_GPIO_Port, LED_Orange_Pin);
		char *text;
		if (led_state == GPIO_PIN_RESET)
		{
			text = "Orange LED ON\r\n";
		}
		else 
		{
			text = "Orange LED OFF\r\n";
		}		
		osMessageQueuePut(mid1, text, 5, 0U);
  delayTime = 4000U;                    // delay 1 second
  status = osDelay(delayTime);          // suspend thread execution
		osThreadJoin(tid4);
  }
}
void led_green (void *argument) {
 	osDelay(3000);
	
  // ...
	for(;;)	{
	osStatus_t status;                    // capture the return status
  uint32_t   delayTime;                 // delay time in milliseconds
	static uint32_t pin_state = 0U;
	if (pin_state == 0)	pin_state = 1U;
		else pin_state = 0U;
	Driver_GPIO0.SetOutput(60, pin_state);
//	HAL_GPIO_TogglePin(LED_Green_GPIO_Port, LED_Green_Pin);
		
		GPIO_PinState led_state = HAL_GPIO_ReadPin(LED_Green_GPIO_Port, LED_Green_Pin);
		char *text;
		if (led_state == GPIO_PIN_RESET)
		{
			text = "Green LED ON\r\n";
		}
		else 
		{
			text = "Green LED OFF\r\n";
		}
		osMessageQueuePut(mid1, text, 4, 0U);
		osEventFlagsSet(evt_id, 1U);
		delayTime = 4000U;                    // delay 1 second
		status = osDelay(delayTime);          // suspend thread execution
		
		osThreadExit();
  }
}

void usb_send (void *argument)
{	
	evt_id = osEventFlagsNew(NULL);
	osThreadFlagsWait(1U, osFlagsWaitAny, osWaitForever);
	
	for(;;)
	{
		osEventFlagsWait(evt_id, 1U, osFlagsWaitAny, osWaitForever);
		uint8_t msg_count = osMessageQueueGetCount(mid1);
		for (int i = 0; i < msg_count; i++)
		{
			char msg[20];
			osMessageQueueGet(mid1, msg, NULL, osWaitForever);	

			while (CDC_Transmit_FS((uint8_t*)msg, strlen((const char*)msg)))
			{
				osDelay(1);
			}
		}
	}
}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	osThreadFlagsSet(tid5, 1U);
//}

void ARM_GPIO_SignalEvent (ARM_GPIO_Pin_t pin, uint32_t event)
{
	osThreadFlagsSet(tid5, 1U);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	EventRecorderInitialize(EventRecordAll, 0);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
	Driver_GPIO0.Setup(0, ARM_GPIO_SignalEvent);
	Driver_GPIO0.SetEventTrigger(0, ARM_GPIO_TRIGGER_RISING_EDGE);
	osKernelInitialize();
	
	const osThreadAttr_t usb_rx_thread_config = {.attr_bits = osSafetyClass(3U), .priority = osPriorityLow, .name = "usb_send"};	
	
	const osThreadAttr_t led_blue_thread_config = {.attr_bits = osSafetyClass(2U), .priority = osPriorityNormal, .name = "blue"};	
	const osThreadAttr_t led_red_thread_config = {.attr_bits = osSafetyClass(1U), .priority = osPriorityNormal1, .name = "red"};	
	const osThreadAttr_t led_orange_thread_config = {.attr_bits = osSafetyClass(3U), .priority = osPriorityNormal2, .name = "orange"};	
	const osThreadAttr_t led_green_thread_config = {.attr_bits = osSafetyClass(4U), .priority = osPriorityNormal3, .name = "green"};	
	
	mid1 = osMessageQueueNew(10, 20, NULL);
	if (mid1 == NULL)
	{
		HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
	}	
	else
	{
	tid1 = osThreadNew(led_blue, NULL, &led_blue_thread_config);
	tid2 = osThreadNew(led_red, NULL, &led_red_thread_config);
	tid3 = osThreadNew(led_orange, NULL, &led_orange_thread_config);
	tid4 = osThreadNew(led_green, NULL, &led_green_thread_config);
	
	tid5 = osThreadNew(usb_send, NULL, &usb_rx_thread_config);
	}
	osKernelStart();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		for (;;) {
	
	}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_Green_Pin|LED_Orange_Pin|LED_Red_Pin|LED_Blue_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUTTON_1_Pin */
  GPIO_InitStruct.Pin = BUTTON_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Green_Pin LED_Orange_Pin LED_Red_Pin LED_Blue_Pin */
  GPIO_InitStruct.Pin = LED_Green_Pin|LED_Orange_Pin|LED_Red_Pin|LED_Blue_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 9, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
