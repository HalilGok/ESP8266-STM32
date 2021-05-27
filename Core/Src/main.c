/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Esp8266-01 Test-Function
 * Author          : Halil Gök
 * websites        :https://embeddedsystemshalilgk.wordpress.com
 * Youtube         :https://www.youtube.com/channel/UC8cJpAVnScqDzZ7IFs2tQMw
 ******************************************************************************
 * @attention
 *ESP_Init();this function should be called after testing. ESP_Init() can be called
 *this If hardware and software problems are not encountered.
 *test_AT();first, you must query with the test_AT command.
 *  USART2 -> PC
 *  USART3 ->ESP8266
 *We do the data retrieval part with interrupt.
 * Don't forget to activate the interrupt.
 *
 * Functions ;
 * test_AT(void) ->: It is the test function of the ESP8266-01 module
 * for hardware communication. Sends the AT -> OK command via UART2.
 * test_PC_to_ESP-> Transfers data from PC to ESP
 * printf_PC(char *string)-> you can use this function to communicate
 *  with the device or pc , to receive information.
 *  printf_Esp(char *string)->this main function is used when writing a program.
 *   printf_Esp("AT + XX\r \ n")
 * ESP_Init(char *SSID,char *PASSWORD)->this function should be called after testing.
 *  ESP_Init() can be called If hardware and software problems are not encountered.
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
const char* WiFi_Name = "****";
const char* Wifi_Password = "****";
const char* Port = "***";
const char* IP = "***";
const char* User_Name = "***";
const char* User_Password = "****";

const char* TOPIC = "***";


#define MAXIMUM_SIZE 750   // data transfer size
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
char PC_buffer[MAXIMUM_SIZE];
char Esp_buffer[25];
volatile uint8_t esp_rx_buffer[MAXIMUM_SIZE];
volatile uint8_t pc_rx_buffer[MAXIMUM_SIZE];
volatile uint8_t old_pc_rx_buffer[MAXIMUM_SIZE];
uint8_t byte_pc;
uint8_t byte_esp;
volatile uint8_t counter = 0;
uint8_t esp_tx_buffer[100];


char data_buffer[100];
uint16_t ProtocolNameLength;
uint16_t ClientIDLength;
uint8_t connect = 0x10,publishCon = 0x30,subscribeCon = 0x82;
char *protocolName = "MQTT";
char *clientID = "Halil";
uint8_t level = 0x04;
uint16_t keepAlive =90;
uint16_t packetID = 0x01;
uint8_t Qos = 0x00;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

void ESP_Init(const char *SSID,const char *PASSWORD);

void printf_Esp(const char *string);
void printf_PC(const char *string);



/* test_AT() : It is the test function of the ESP8266-01 module
 * for hardware communication. Sends the AT -> OK command via UART2.
 */
void test_AT(void);

/*
 * test_PC_to_ESP(): Transfers data from PC to ESP
 * it does not require any input or output.Because it's an STM32 tool.
 */
void test_PC_to_ESP(void);

void Connect_Broker(const char *Ip ,const char *Port);

void Connect_Secure_Broker(const char *Ip,const char *Port,const char *userName,const char *password);

void Read_Message_MQTT();
void publish_MQTT(char *topic, char *message);
void Subscribe_MQTT(const char *topic);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	/*incoming data is read piecemeal. */
	HAL_UART_Receive_IT(&huart2, &byte_pc, 1);
	HAL_UART_Receive_IT(&huart3, &byte_esp, 1);
	 printf_PC("Program is starting..\n");
	test_AT();
	HAL_Delay(2000);
	printf_PC("Please wait...\n");
	ESP_Init(WiFi_Name,Wifi_Password);
	HAL_Delay(5000);
	 // Connect_Broker("192.168.31.203","1883");
    Connect_Secure_Broker(IP,Port,User_Name,User_Password);
	HAL_Delay(2000);
	Subscribe_MQTT(TOPIC);
	HAL_Delay(2000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		Read_Message_MQTT();
		//test_PC_to_ESP();
		HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
/*ESP_Init(char *SSID,char *PASSWORD): this function should be called after testing.
 *  ESP_Init() can be called If hardware and software problems are not encountered.*/
void ESP_Init(const char *SSID,const char *PASSWORD) {
	printf_Esp("AT+CWMODE=1\r\n");  // STA mode on
	printf_Esp("AT+CWQAP\r\n"); //closes earlier networks
	//printf_Esp("AT+RST\n\r");  // RESET

	sprintf((char*)esp_tx_buffer, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWORD);
	printf_Esp((char*) esp_tx_buffer);
    printf_PC("network connected..\n");

}


void test_AT(void) {
	//it is a test function that confirms that esp communicates hardware with the pc.
	//UART3 -> ESP UART2 -> PC
	printf_PC("***** ESP8266-01 Test Software *****\n");
	printf_PC("** It can't be learned without making mistakes. **\n");

	printf_Esp("AT\r\n");   // At command sent to ESP
	HAL_Delay(50);
	strcpy((char*)old_pc_rx_buffer, (char*)esp_rx_buffer); // to back up esp's response
	printf_PC("\nAT commend sent. Response =\n "); //
	HAL_Delay(50);
	printf_PC((char*) old_pc_rx_buffer); //  the copied array (old_pc_rx_buffer) is sent to the PC.
	memset((char*)old_pc_rx_buffer, 0, sizeof(old_pc_rx_buffer)); //reset the array
	memset((char*)pc_rx_buffer, 0, sizeof(pc_rx_buffer));         //***
	memset((char*)esp_rx_buffer, 0, sizeof(esp_rx_buffer));       //**
}


void Connect_Broker(const char *Ip , const char *Port)
{
	uint8_t flag = 0x02;   // 02--> sifresiz

	   printf_Esp("AT+CIPCLOSE\r\n");
	   HAL_Delay(250);
	   printf_Esp("AT+CIPMUX=0\r\n");
	   HAL_Delay(250);
	   printf_Esp("AT+CIFSR\r\n"); //**
	   HAL_Delay(250);
	   sprintf((char*)esp_tx_buffer,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",Ip,Port);
	   HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",Ip,Port),5000);
	   HAL_Delay(2000);
	//connect packet

	ProtocolNameLength = strlen(protocolName);
	ClientIDLength     = strlen(clientID);
	uint8_t Remainlength;
	Remainlength = 2+ProtocolNameLength+6+ClientIDLength;
	uint16_t length = sprintf((char*)esp_tx_buffer,"%c%c%c%c%s%c%c%c%c%c%c%s",(char)connect,(char)Remainlength,(char)(ProtocolNameLength << 8),(char)ProtocolNameLength,protocolName,(char)level,(char)flag,(char)(keepAlive << 8),(char)keepAlive,(char)(ClientIDLength << 8),(char)ClientIDLength,clientID);

	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"AT+CIPSEND=%d\r\n",length),1000);
	HAL_Delay(100);
	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"%c%c%c%c%s%c%c%c%c%c%c%s",(char)connect,(char)Remainlength,(char)(ProtocolNameLength << 8),(char)ProtocolNameLength,protocolName,(char)level,(char)flag,(char)(keepAlive << 8),(char)keepAlive,(char)(ClientIDLength << 8),(char)ClientIDLength,clientID),5000);
	printf_PC("connected to the server..\n");

}

void Connect_Secure_Broker(const char *Ip,const char *Port, const char *userName,const char *password)
{
	    uint16_t usernamelength;
		uint16_t passwordlength;
		uint8_t flag = 0xC2;

	       printf_Esp("AT+CIPCLOSE\r\n");
		   HAL_Delay(250);
		   printf_Esp("AT+CIPMUX=0\r\n");
		   HAL_Delay(250);
		   printf_Esp("AT+CIFSR\r\n"); //**
		   HAL_Delay(250);
		   sprintf((char*)esp_tx_buffer,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",Ip,Port);
		   HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",Ip,Port),5000);
		   HAL_Delay(2000);

           //connect packet
		   ProtocolNameLength = strlen(protocolName);
		     ClientIDLength = strlen(clientID);
		     usernamelength = strlen(userName);
		     passwordlength = strlen(password);
		      uint8_t Remainlength;
		     	Remainlength = 2+ProtocolNameLength+6+ClientIDLength+2+usernamelength+2+passwordlength;
		     	uint16_t length = sprintf((char*)esp_tx_buffer,"%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s%c%c%s",(char)connect,(char)Remainlength,(char)(ProtocolNameLength << 8),(char)ProtocolNameLength,protocolName,(char)level,(char)flag,(char)(keepAlive << 8),(char)keepAlive,(char)(ClientIDLength << 8),(char)ClientIDLength,clientID,(char)(usernamelength << 8),(char)usernamelength,userName,(char)(passwordlength << 8),(char)passwordlength,password);

		     	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"AT+CIPSEND=%d\r\n",length),1000);
		     	HAL_Delay(250);
		     	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s%c%c%s",(char)connect,(char)Remainlength,(char)(ProtocolNameLength << 8),(char)ProtocolNameLength,protocolName,(char)level,(char)flag,(char)(keepAlive << 8),(char)keepAlive,(char)(ClientIDLength << 8),(char)ClientIDLength,clientID,(char)(usernamelength << 8),(char)usernamelength,userName,(char)(passwordlength << 8),(char)passwordlength,password),5000);
		      HAL_Delay(250);
             printf_PC("connected to the server..\n");
}

void Subscribe_MQTT(const char *topic)
{
	uint16_t TopicLength = strlen(topic);
	uint8_t RemainLength = 2+2+TopicLength+1; // packetIDlength(2) + topiclengthdata(2)+topiclength+Qos
	uint16_t length = sprintf((char*)esp_tx_buffer,"%c%c%c%c%c%c%s%c",(char)subscribeCon,(char)RemainLength,(char)(packetID << 8),(char)packetID,(char)(TopicLength << 8),(char)TopicLength,topic,(char)Qos);
	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"AT+CIPSEND=%d\r\n",length),1000);
	HAL_Delay(100);
	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"%c%c%c%c%c%c%s%c",(char)subscribeCon,(char)RemainLength,(char)(packetID << 8),(char)packetID,(char)(TopicLength << 8),(char)TopicLength,topic,(char)Qos),5000);
    printf_PC("'");printf_PC(topic);printf_PC("'");printf_PC("subscribed\n");
}
void publish_MQTT(char *topic, char *message)
{

	uint16_t topiclength = strlen(topic);
	uint8_t remainlength = 2+topiclength+strlen(message);
	int length = sprintf((char*)esp_tx_buffer,"%c%c%c%c%s%s",(char)publishCon,(char)remainlength,(char)(topiclength << 8),(char)topiclength,topic,message);
	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"AT+CIPSEND=%d\r\n",length),100);
	HAL_Delay(100);
	HAL_UART_Transmit(&huart3,(uint8_t *)esp_tx_buffer,sprintf((char*)esp_tx_buffer,"%c%c%c%c%s%s",(char)publishCon,(char)remainlength,(char)(topiclength << 8),(char)topiclength,topic,message),5000);
	HAL_Delay(100);

}
void Read_Message_MQTT()
{
	int remain_length = 0, message_length = 0, topic_length = 0;
	char message[100];
	HAL_UART_AbortReceive_IT(&huart3);
	for (int i = 0; i < sizeof(esp_rx_buffer); i++) {
		if (esp_rx_buffer[i] == 0x30) {
			remain_length = esp_rx_buffer[i + 1];
			topic_length = esp_rx_buffer[i + 2] + esp_rx_buffer[i + 3];
			message_length = remain_length - (topic_length + 2);
			for (int j = 0; j < message_length; j++) {

				message[j] = esp_rx_buffer[i + 4 + topic_length + j];

			}
			break;
		}
	}
	for (int a = 0; a < message_length; a++) {
		data_buffer[a] = message[a];
	}
	if (data_buffer[0] != '\0') {
		printf_PC("Message:");
		printf_PC(data_buffer);
		printf_PC("\n");
	}

		if(strcmp(data_buffer,"test mqtt") == 0)
		{
			publish_MQTT("/gokhalil723@gmail.com/test1","Test successful");
		}

		if(message[0] == 'O' && message[1] == 'N')
		{
			publish_MQTT("/gokhalil723@gmail.com/State","Enable");
		}
		if(message[0] == 'O' && message[1] == 'F' && message[2] == 'F')
		{
			publish_MQTT("/gokhalil723@gmail.com/State","Disable");
		}

		memset((char*)esp_rx_buffer,0,sizeof(esp_rx_buffer)); 											// clear buffer
		memset(message,0,sizeof(message));
		memset(data_buffer,0,sizeof(data_buffer));
		byte_esp=0;
		HAL_UART_Receive_IT(&huart3,&byte_esp,1);
}


/*test_PC_to_ESP(void):This function is usually used to communicate
 * directly with esp for testing purposes.You can run AT commands in this function.*/
void test_PC_to_ESP(void)
{

	     HAL_Delay(50);
	     printf_Esp((char*)pc_rx_buffer);// pcden gelen veriyi espye yollar
	     HAL_Delay(50);
	     strcpy((char*)old_pc_rx_buffer,(char*)esp_rx_buffer);
	     HAL_Delay(50);
	     printf_PC((char*)old_pc_rx_buffer);
	     HAL_Delay(50);
         memset((char*)old_pc_rx_buffer, 0, sizeof(old_pc_rx_buffer));

}

/*printf_PC(char *string): you can use this function to communicate
 *  with the device or pc , to receive information.*/


void printf_PC(const char *string)
{
	counter = 0;
	byte_pc = 0;
	byte_esp=0;

	if (string[0] !='\0') {

		sprintf(PC_buffer, string);
		HAL_UART_Transmit(&huart2, (uint8_t*) PC_buffer, strlen(PC_buffer),	1000);
		HAL_Delay(50);
		memset(PC_buffer, 0, sizeof(PC_buffer));
		memset((char*)esp_rx_buffer,0, sizeof(esp_rx_buffer));


}
}
void printf_Esp(const char *string)
{

	     counter=0;
	     byte_pc=0;
	     byte_esp=0;

	if(string[0] !='\0'){

    memset(Esp_buffer, 0, sizeof(Esp_buffer));
	sprintf(Esp_buffer,string);

	HAL_UART_Transmit(&huart3,(uint8_t*)Esp_buffer,strlen(Esp_buffer),1000);
	HAL_Delay(50);
	 memset(Esp_buffer, 0, sizeof(Esp_buffer));

	 memset((char*)pc_rx_buffer, 0, sizeof(pc_rx_buffer));
	}
}
/*HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart):
 * this function is standard uart.C is the function.
 *  This function is called every time the interrupt enters
 *   the interrupt.We do 1 byte to 1 byte reading. We'll record
 *   it in the series. */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) //queried which uart is used
	{
		HAL_UART_Receive_IT(&huart2, &byte_pc, 1);

		pc_rx_buffer[counter] = byte_pc;
		counter++;

	}

	if (huart->Instance == USART3) {
		HAL_UART_Receive_IT(&huart3, &byte_esp, 1);

		esp_rx_buffer[counter] = byte_esp;

		counter++;
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
