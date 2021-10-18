/*
 * esp8266-01.h
 *
 *  Created on: Aug 22, 2021
 *      Author: theen
 */

#ifndef INC_ESP8266_01_H_
#define INC_ESP8266_01_H_

#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"



#define BUFFER_MAX_SIZE 100
#define RXBUFFER_MAX_SIZE 750
typedef struct{

 UART_HandleTypeDef *huart;
 char buffer[BUFFER_MAX_SIZE];
 volatile uint8_t rxbufferIndex;
 volatile uint8_t rxBuffer[RXBUFFER_MAX_SIZE];
 uint8_t byteRx;

}ItemUnit;
void __printf_ctor(ItemUnit* const me,UART_HandleTypeDef *huart);
void __printf(ItemUnit* const me,const char * string);
void TEST(ItemUnit*const me);
void EspInit(ItemUnit* const me,const char *SSID,const char *PASSWORD,ItemUnit* const pc);
void connect_Broker(ItemUnit* const me,const char *Ip, const char *Port,ItemUnit *const pc);
void Connect_Secure_Broker(ItemUnit*const me,const char *Ip,const char *Port, const char *userName,
		const char *password,ItemUnit*const pc);
void Subscribe_MQTT(ItemUnit *const me,const char *topic,ItemUnit *const pc);
void publish_MQTT(ItemUnit *const me,char *topic, char *message);
void Read_Message_MQTT(ItemUnit *const me,ItemUnit *const pc);
void TEST_AT(ItemUnit *esp,ItemUnit *pc);
void clearBuffer(ItemUnit* const me);
#endif /* INC_ESP8266_01_H_ */
