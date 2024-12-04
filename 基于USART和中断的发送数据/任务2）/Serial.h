#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>             //把包含uint类型定义的头文件加进来
#include <stdio.h>
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);

uint32_t Serial_Pow(uint32_t x, uint32_t y);
void Serial_SendNumber(uint32_t Number, uint8_t Length);

void ReceiveData_Inquire();
uint8_t Serial_GetRxFlag(void);

void LED1_ON(void);
void LED1_OFF(void);

   //声明数组为外部可调用
extern char Serial_RxPacket[];

#endif
