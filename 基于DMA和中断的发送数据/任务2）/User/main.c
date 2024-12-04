#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MyDMA.h"                  // Device header


//本节实验-DMA
//DMA--Direct Memory Access 直接存储器访问
//DMA的作用为：可直接访问stm32内部的储存器，协助CPU，完成数据转运的工作
//C8T6有一个DMA串口，该串口有7个通道。

char DataA[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'i', 'n', 'd', 'o', 'w', 's', '!'};       //定义DMA转运的源端数组
char DataB[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                                   //定义DMA转运的目的数组               


int main(void)
{
	uint8_t i;
	OLED_Init();
	MyDMA_Init((uint32_t)DataA, (uint32_t)DataB, 14);
	
	OLED_ShowString(1, 1, "DataA");
	OLED_ShowString(3, 1, "DataB");
	OLED_ShowHexNum(1, 8, (uint32_t)DataA, 8);
	OLED_ShowHexNum(3, 8, (uint32_t)DataB, 8);
	
	
	
	while (1)
	{
		OLED_ShowChar(2, 1, DataA[0]);
		OLED_ShowChar(2, 2, DataA[1]);
		OLED_ShowChar(2, 3, DataA[2]);
		OLED_ShowChar(2, 4, DataA[3]);
		OLED_ShowChar(2, 5, DataA[4]);
		OLED_ShowChar(2, 6, DataA[5]);
		OLED_ShowChar(2, 7, DataA[6]);
		OLED_ShowChar(2, 8, DataA[7]);
		OLED_ShowChar(2, 9, DataA[8]);
		OLED_ShowChar(2, 10, DataA[9]);
		OLED_ShowChar(2, 11, DataA[10]);
		OLED_ShowChar(2, 12, DataA[11]);
		OLED_ShowChar(2, 13, DataA[12]);
		OLED_ShowChar(2, 14, DataA[13]);
		
	
		
		Delay_ms(1000);
		
		MyDMA_Transfer();
		
		Delay_ms(1000);
		
		for(i=0;i<=13;i++)
		{
			OLED_ShowChar(4, i+1, DataB[i]);
			Delay_ms(300);
		}
		OLED_ShowString(4, 1, "              ");
		i = -1;
		Delay_ms(500);
		
	}
}
