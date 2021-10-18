/*
 * esp8266-01.c
 *
 *  Created on: Aug 22, 2021
 *      Author: Halil Gök
 */

#include "esp8266-01.h"

uint16_t ProtocolNameLength;
uint16_t ClientIDLength;
uint8_t connect = 0x10, publishCon = 0x30, subscribeCon = 0x82;
char *protocolName = "MQTT";
char *clientID = "Halil";
uint8_t level = 0x04;
uint16_t keepAlive = 90;
uint16_t packetID = 0x01;
uint8_t Qos = 0x00;

void __printf_ctor(ItemUnit *const me, UART_HandleTypeDef *huart) {
	me->huart = huart;

}
/*__printf: you can use this function to communicate
 *  with the device or pc , to receive information.*/
void __printf(ItemUnit *const me, const char *string) {
	if (string[0] != '\0') {

		//memset(Esp_buffer, 0, sizeof(Esp_buffer));
		sprintf(me->buffer, string);
		HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer, strlen(me->buffer),
				1000);
		HAL_Delay(50);
		memset(me->buffer, 0, sizeof(me->buffer));

		//memset((char*)pc_rx_buffer, 0, sizeof(pc_rx_buffer));
	}
}

void TEST(ItemUnit *const me) {
	__printf(me, "This is a Uart test.\n");
	HAL_Delay(250);
}
void TEST_AT(ItemUnit *esp, ItemUnit *pc) {
	//it is a test function that confirms that esp communicates hardware with the pc.
	//UART3 -> ESP UART2 -> PC
	uint8_t ESPEventCase = 0, stateTry = 0;
	__printf(pc, "***** ESP8266-01 Test Software *****\n");
	__printf(pc, "** It can't be learned without making mistakes. **\n");

//	__printf(esp,"AT+RST\r\n");   // At+RST command sent to ESP
//	__printf(pc,"Resetting \r\n");
	HAL_Delay(2000);
	__printf(pc, "AT command has been sent.");
	while (ESPEventCase == 0) {
		__printf(esp, "AT\r\n");
		HAL_Delay(500);

		//   __printf(pc,esp->rxBuffer);
		if (strstr((char*) (esp->rxBuffer), "OK") != NULL) {
			__printf(pc, "\r\n");
			__printf(pc, "The module has been accessed.\r\n");
			ESPEventCase = 1;
		}

		if (strstr((char*) (esp->rxBuffer), "OK") == NULL && stateTry == 5) {
			__printf(pc, "\r\nThe module was not found.");
			stateTry = 0;
			HAL_Delay(500);
		}
		__printf(pc, ".");
		stateTry++;
		clearBuffer(esp);
	}
}

/*ESP_Init: This function should be called after testing.
  ESP_Init() can be called If hardware and software problems are not encountered.*/

void EspInit(ItemUnit *const me, const char *SSID, const char *PASSWORD,
		ItemUnit *const pc) {
	uint8_t ESPInitCase = 0;
	while (ESPInitCase == 0) {
		__printf(me, "AT\r\n");
		if (strstr((char*) (me->rxBuffer), "OK") != NULL) {

			ESPInitCase = 1;
		}

		clearBuffer(me);
	}
	while (ESPInitCase == 1) {
		__printf(me, "AT+CWMODE?\r\n");
		HAL_Delay(1500);

		if (strstr((char*) (me->rxBuffer), "+CWMODE:1") != NULL) {

			__printf(pc, "The MODE Setting is Correct\n"); //1 'se application modd
			ESPInitCase = 2;
		} else {

			__printf(me, "AT+CWMODE=1\r\n");
			HAL_Delay(700);
			__printf(pc, "The MODE has been Changed.\n");
			ESPInitCase = 2;
		}
		clearBuffer(me);
	}
	while (ESPInitCase == 2)
	{

		__printf(me, "AT+CWQAP\r\n");
		HAL_Delay(700);
		ESPInitCase = 3;
		clearBuffer(me);
	}
	while (ESPInitCase == 3)
	{
		sprintf((char*) me->buffer, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID,
				PASSWORD);
		__printf(me, me->buffer);
		HAL_Delay(1500);

		if (strstr((char*) (me->rxBuffer), "OK") != NULL) {
			__printf(pc, "network connection has been made\n");
			ESPInitCase = 4;
			clearBuffer(me);

		} else {
			__printf(pc, "network connection is expected\n");
			HAL_Delay(500);

		}

	}

}
void connect_Broker(ItemUnit *const me, const char *Ip, const char *Port,
		ItemUnit *const pc) {
	uint8_t ESPBrokerInitCase = 0;
	uint8_t flag = 0x02;   // 02--> unencrypted
	__printf(pc,"0.0");
	while (ESPBrokerInitCase == 0) {
		__printf(me, "AT+CIPCLOSE\r\n");

		HAL_Delay(250);
		if (strstr((char*) (me->rxBuffer), "OK") != NULL) {
			ESPBrokerInitCase = 1;

		}
		clearBuffer(me);

	}
	while (ESPBrokerInitCase == 1) {
		__printf(me, "AT+CIPMUX=0\r\n");
		HAL_Delay(250);

		if (strstr((char*) (me->rxBuffer), "OK") != NULL) {
			ESPBrokerInitCase = 2;
			clearBuffer(me);
		}
		clearBuffer(me);

	}
	while (ESPBrokerInitCase == 2) {
		__printf(me, "AT+CIFSR\r\n");
		HAL_Delay(1000);

		// It sends error information until it receives an IP. We'll split it up. =)
		if (strstr((char*) (me->rxBuffer), "ERROR") == NULL)

		{
			__printf(pc, "Alinan IP = \n"); // 11 Of the incoming information.it says the IP address from the character.
			__printf(pc, (char*) (me->rxBuffer + 11));
			ESPBrokerInitCase = 3;
		}
       else {
			__printf(pc, "Try Again.\n");

			}
		clearBuffer(me);
	}
	while (ESPBrokerInitCase == 3) {

		sprintf((char*) me->buffer, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", Ip,
				Port);
		HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
				sprintf((char*) me->buffer, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",
						Ip, Port), 5000);
		HAL_Delay(2000);

		__printf(pc, "TCP Connection request has been sent\n\n");
		HAL_Delay(200);
		if (strstr((char*) (me->rxBuffer), "OK") != NULL) {

			__printf(pc, "A connection has been established with the site\n\n");

			ESPBrokerInitCase = 4;

		} else {
			HAL_Delay(1000);
			__printf(pc, "Try Again.\n");

		}

		clearBuffer(me);
	}
	ProtocolNameLength = strlen(protocolName);
	ClientIDLength = strlen(clientID);
	uint8_t Remainlength;
	Remainlength = 2 + ProtocolNameLength + 6 + ClientIDLength;
	uint16_t length = sprintf((char*) me->buffer, "%c%c%c%c%s%c%c%c%c%c%c%s",
			(char) connect, (char) Remainlength,
			(char) (ProtocolNameLength << 8), (char) ProtocolNameLength,
			protocolName, (char) level, (char) flag, (char) (keepAlive << 8),
			(char) keepAlive, (char) (ClientIDLength << 8),
			(char) ClientIDLength, clientID);

	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "AT+CIPSEND=%d\r\n", length), 1000);
	HAL_Delay(100);
	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf(((char*) me->buffer), "%c%c%c%c%s%c%c%c%c%c%c%s",
					(char) connect, (char) Remainlength,
					(char) (ProtocolNameLength << 8), (char) ProtocolNameLength,
					protocolName, (char) level, (char) flag,
					(char) (keepAlive << 8), (char) keepAlive,
					(char) (ClientIDLength << 8), (char) ClientIDLength,
					clientID), 5000);
	__printf(pc, "connected to the server..\n");

}

void Connect_Secure_Broker(ItemUnit *const me, const char *Ip, const char *Port,
		const char *userName, const char *password, ItemUnit *const pc) {
	uint8_t ESPBrokerInitCase2 = 0;
	uint16_t usernamelength;
	uint16_t passwordlength;
	uint8_t flag = 0xC2;
	while (ESPBrokerInitCase2 == 0) {
		__printf(me, "AT+CIPCLOSE\r\n");
		HAL_Delay(250);
	//	if (strstr((char*) (me->rxBuffer), "OK") != NULL) {
			ESPBrokerInitCase2 = 1;

	//	}
		//clearBuffer(me);

	}
	while (ESPBrokerInitCase2 == 1) {
		__printf(me, "AT+CIPMUX=0\r\n");
		HAL_Delay(250);
	//	if (strstr((char*) (me->rxBuffer), "OK") != NULL) {
			ESPBrokerInitCase2 = 2;
		//	clearBuffer(me);
	//	}


	}
	while (ESPBrokerInitCase2 == 2) {
		__printf(me, "AT+CIFSR\r\n");
		HAL_Delay(700);


//	if (strstr((char*) (me->rxBuffer), "ERROR") == NULL)
		{
			__printf(pc, "Alinan IP = \n");
			__printf(pc, (char*) (me->rxBuffer + 11));
			ESPBrokerInitCase2 = 3;
//		} else {  // ERROR der ise tekrar dene
//			HAL_Delay(1000);
//			__printf(pc, "Tekrar Dene.\n");
//
	//}
		clearBuffer(me);
	}}
	while (ESPBrokerInitCase2 == 3) {

		sprintf((char*) me->buffer, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", Ip,
				Port);
		HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
				sprintf((char*) me->buffer, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",
						Ip, Port), 5000);
		HAL_Delay(1500);

		__printf(pc, "TCP Connection request has been sent\n\n");
		HAL_Delay(200);
		if (strstr((char*) (me->rxBuffer), "OK") != NULL) {

			__printf(pc, "A connection has been established with the site\n\n");

			ESPBrokerInitCase2 = 4;

		} else {
			HAL_Delay(1000);
			__printf(pc, "Try Again.\n");

		}

	}
	ProtocolNameLength = strlen(protocolName);
	ClientIDLength = strlen(clientID);
	usernamelength = strlen(userName);
	passwordlength = strlen(password);
	uint8_t Remainlength;
	Remainlength = 2 + ProtocolNameLength + 6 + ClientIDLength + 2
			+ usernamelength + 2 + passwordlength;
	uint16_t length = sprintf((char*) me->buffer,
			"%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s%c%c%s", (char) connect,
			(char) Remainlength, (char) (ProtocolNameLength << 8),
			(char) ProtocolNameLength, protocolName, (char) level, (char) flag,
			(char) (keepAlive << 8), (char) keepAlive,
			(char) (ClientIDLength << 8), (char) ClientIDLength, clientID,
			(char) (usernamelength << 8), (char) usernamelength, userName,
			(char) (passwordlength << 8), (char) passwordlength, password);

	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "AT+CIPSEND=%d\r\n", length), 1000);
	HAL_Delay(250);
	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s%c%c%s",
					(char) connect, (char) Remainlength,
					(char) (ProtocolNameLength << 8), (char) ProtocolNameLength,
					protocolName, (char) level, (char) flag,
					(char) (keepAlive << 8), (char) keepAlive,
					(char) (ClientIDLength << 8), (char) ClientIDLength,
					clientID, (char) (usernamelength << 8),
					(char) usernamelength, userName,
					(char) (passwordlength << 8), (char) passwordlength,
					password), 5000);
	HAL_Delay(250);
	__printf(pc, "Connected to the server..\n");
	clearBuffer(me);

}
void Subscribe_MQTT(ItemUnit *const me, const char *topic, ItemUnit *const pc) {

	uint16_t TopicLength = strlen(topic);
	uint8_t RemainLength = 2 + 2 + TopicLength + 1; // packetIDlength(2) + topiclengthdata(2)+topiclength+Qos
	uint16_t length = sprintf((char*) me->buffer, "%c%c%c%c%c%c%s%c",
			(char) subscribeCon, (char) RemainLength, (char) (packetID << 8),
			(char) packetID, (char) (TopicLength << 8), (char) TopicLength,
			topic, (char) Qos);
	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "AT+CIPSEND=%d\r\n", length), 1000);
	HAL_Delay(100);
	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "%c%c%c%c%c%c%s%c", (char) subscribeCon,
					(char) RemainLength, (char) (packetID << 8),
					(char) packetID, (char) (TopicLength << 8),
					(char) TopicLength, topic, (char) Qos), 5000);
	__printf(pc, "'");
	__printf(pc, topic);
	__printf(pc, "'");
	__printf(pc, "subscribed\n");
	clearBuffer(me);

}
void publish_MQTT(ItemUnit *const me, char *topic, char *message) {
	uint16_t topiclength = strlen(topic);
	uint8_t remainlength = 2 + topiclength + strlen(message);
	int length = sprintf((char*) me->buffer, "%c%c%c%c%s%s",
			(char) publishCon, (char) remainlength, (char) (topiclength << 8),
			(char) topiclength, topic, message);
	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "AT+CIPSEND=%d\r\n", length), 100);
	HAL_Delay(100);
	HAL_UART_Transmit(me->huart, (uint8_t*) me->buffer,
			sprintf((char*) me->buffer, "%c%c%c%c%s%s", (char) publishCon,
					(char) remainlength, (char) (topiclength << 8),
					(char) topiclength, topic, message), 5000);
	HAL_Delay(100);
}
void Read_Message_MQTT(ItemUnit *const me, ItemUnit *const pc) {
	char data_buffer[100];
	int remain_length = 0, message_length = 0, topic_length = 0;
	char message[100];
//	HAL_UART_AbortReceive_IT(me->huart);
	for (int i = 0; i < sizeof(me->rxBuffer); i++) {
		if (me->rxBuffer[i] == 0x30) {
			remain_length = me->rxBuffer[i + 1];
			topic_length = me->rxBuffer[i + 2] + me->rxBuffer[i + 3];
			message_length = remain_length - (topic_length + 2);
			for (int j = 0; j < message_length; j++) {

				message[j] = me->rxBuffer[i + 4 + topic_length + j];

			}
			break;
		}
	}

	for (int a = 0; a < message_length; a++) {
		data_buffer[a] = message[a];
	}
	if (data_buffer[0] != '\0') {
		__printf(pc, "Message:");
		__printf(pc, data_buffer);
		__printf(pc, "\n");
	}

	if (strcmp(data_buffer, "test mqtt") == 0) {
		publish_MQTT(me, "**SUBJECT**", "Test successful");
	}

	if (message[0] == 'O' && message[1] == 'N') {
		publish_MQTT(me, "**SUBJECT**", "Enable");
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, ENABLE);
	}
	if (message[0] == 'O' && message[1] == 'F' && message[2] == 'F') {
		publish_MQTT(me, "**SUBJECT**", "Disable");
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, DISABLE);
	}
	memset(message, 0, sizeof(message));
	clearBuffer(me);
	memset(data_buffer, 0, sizeof(data_buffer));
	//HAL_UART_Receive_IT(&huart3, &byte_esp, 1);
}
void clearBuffer(ItemUnit *const me)
{
	memset((char*) me->rxBuffer, 0, strlen((char*) me->rxBuffer));
	me->rxbufferIndex = 0;
}

