#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "key.h"
#include "LED.h"
#include "string.h"

uint8_t RXdata;
uint8_t KeyNum;
//我们定义一下收发的HEX数据包的格式：开始符@ + 不定个数据 + 结束符\r\n
//把数据整合成数据包的意义：
//1.一次性发多个，更有效率
//2.有时，接收方需要的是一次性接收多个数据来进行下一步操作，如接收方需要知道一个三维坐标（x轴+y轴+z轴）
//这时，先前的一个字节一个字节地发送数据就不行了，要把数据3个3个的进行打包



void function_2(void)      //接收HEX数据包
{
	if(Serial_GetRxFlag() == 1)
	{
		OLED_ShowString(4, 1, "                ");
		OLED_ShowString(4, 1, Serial_RxPacket);
		
		
		if(strcmp(Serial_RxPacket, "LED1_ON") == 0)
		{
			LED1_ON();
			Serial_SendString("LED1_ON_OK\r\n");
			OLED_ShowString(2, 1, "                ");
			OLED_ShowString(2, 1, "LED1_ON_OK");
		}
		else if(strcmp(Serial_RxPacket, "LED1_OFF") == 0)
		{
			LED1_OFF();
			Serial_SendString("LED1_OFF_OK\r\n");
			OLED_ShowString(2, 1, "                ");
			OLED_ShowString(2, 1, "LED1_OFF_OK");
		}
		else
		{
			Serial_SendString("ERROR_COMMAND\r\n");
			OLED_ShowString(2, 1, "                ");
			OLED_ShowString(2, 1, "ERROR_COMMAND");
		}
	}
}



void TEXT_1(void)
{
	char str[14] ={'h', 'e', 'l', 'l', 'o', ' ', 'w', 'i', 'n', 'd', 'o', 'w', 's', '!'};
	uint8_t i;
	for(i=0;i<=13;i++)
	{
		OLED_ShowChar(1, i+1, str[i] );
		Delay_ms(1000);
	}
}

void TEXT_2(void)
{
	char str[16] ={'h', 'e', 'l', 'l', 'o', ' ', 'w', 'i', 'n', 'd', 'o', 'w', 's', '!'};
	uint8_t i;
	for(i=0;i<=13;i++)
	{
		OLED_ShowChar(1, i+1, str[i] );
//		uint8_t a = Serial_RxPacket[0] && '#' ;
		if(Serial_RxPacket[0] == '#')
		{
			while(1)
			{
				if(Serial_RxPacket[0] == '*')
				{
					break;
				}
			}
			Serial_RxPacket[0] = 'n';
			
		}
		else if(Serial_RxPacket[0] == '*')
		{
			Delay_ms(300);
			Serial_RxPacket[0] = 'n';
		}
		
		Delay_ms(300);
		//OLED_ShowNum(3, 1, a, 2);
		if(i == 13)
		{
			OLED_Clear();
			i=-1;
		}
	}

}

int main(void)
{
	OLED_Init();
	Key_Init();
	Serial_Init();
	LED_Init();
	
	TEXT_2();
//	OLED_ShowString(1, 1, "TxPacket");
//	OLED_ShowString(3, 1, "RxPacket");
	
	
	
	while (1)
	{
		TEXT_2();
		
	}
	
}

