//串口模块的编写

#include "stm32f10x.h"
#include <stdio.h>
#include "Delay.h"
#include "OLED.h"

//在使用中断的方法接收数据时，我们进行了众多的函数封装，为了方便，我们这些函数中共同用到的参数单独拿出来做全局变量
//使用中断的方法接收数据的函数在末尾
uint8_t Serial_RxData;
uint8_t Serial_RxFlag;        //用于判断数据是否接收完成的标志变量

//定义用于收发数据包的缓存数组
uint8_t Serial_TxPacket[4];  //用于发送数据包的缓存数组
char Serial_RxPacket[1];  //用于接收数据包的缓存数组


//编写初始化函数
void Serial_Init(void)
{
	//根据USART的基本结构来进行初始化
	
	/*
	1.开启时钟，把需要的USART和GPIO的始终打开。
	开启USART的时钟：我们用到的USART1是在APB2总线上的，所以开启APB2的时钟。
	开启GPIO的时钟：根据引脚定义图的，TX、RX分别对应 PA9、PA10，所以开启这两个端口的时钟。
    */	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	
	//2.GPIO初始化，把TX配置为复用输出，RX配置为输入。
	//在本次实验中，既发送数据也接收数据，因此同时配置TX、RX
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;        //模式为复用推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;              //引脚选择PA9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);                 //初始化GPIOA，即让前面几行代码的配置正式生效。
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          //模式为上拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;             //引脚选择PA10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);                 //初始化GPIOA，即让前面几行代码的配置正式生效。
	
	
	//对于串口接收数据的方法有两种：查询和中断。
	//若使用查询方法，则上面的配置就可以了，若使用中断方法，则还需额外添加中断，配置NVIC。
	//查询的具体流程：在主函数中循环判断串口的RXNE位，若位1，则寄存器不为空，即有数据进来了，然后调用ReceiveData函数
	
	//为了方便回顾，我们把这两个方法进行封装，封装后的代码见末尾。
	
	//3.USART初始化
	/*
	对于USART初始化函数而言，该函数有2个参数。
	第1个参数指明 对那个USART进行初始化。
	第2个参数是一个指向结构体的指针，这个结构体是一个用来配置USART具体细节的结构体，
	因此我们要先定义一个结构体，在对结构体的各个参数进行配置。
	*/
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;                                       //设置波特率为9600bps。
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //设置硬件流控制 为 不使用硬件流控制。
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                  //设置模式为TX或RX模式，即同时开启发送数据与接收数据。。
	USART_InitStructure.USART_Parity = USART_Parity_No;                              //设置校验码 为 不使用校验码
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                           //设置停止位的长度为1位;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                      //设置字长，因为没用到校验位，所以设置字长位为8位
	
	USART_Init(USART1,&USART_InitStructure);  //使配置后的结构体正式生效
	//上述配置总结：对PA9端口对应的TX设置为：9600波特率、无流控、只发送模式、无校验位、1位停止位、8位字长
	
	//若采用中断的方式接收数据，则还要添加中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);    //使能UAART_IT_RENE，使其成为USART1的中断
	
	//配置NVIC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //分组
	NVIC_InitTypeDef NVIC_InitStructure;             //初始化NVIC的USART1通道，配置初始化结构体的参数
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//设置中断通道。
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //设置优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);                  //初始化NVIC，使前面的配置生效
	//总结：当USART_IT_RXNE被置为1后，向NVIC申请中断
	
	
	USART_Cmd(USART1, ENABLE);
	
}





//编写发送数据函数，每调用一次这个函数，便可通过TX引脚发送一个字节的数据
void Serial_SendByte(uint8_t Byte)
{
	//该函数直接从stm32f10x_usart.h中调用，其参数定义也在里面找
	USART_SendData(USART1, Byte);
	
	//获取标志位的值，若为set才进行下一次的发送数据，否则一直循环，相当于一直卡在这里，不进行下一步数据的发送
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	
	
}




//编写发送数据函数--接收一个数组并发送数组中的所有内容
//该函数有两个参数，第一个参数位指向数组首地址的指针，第二个参数为数组的长度
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Array[i]);
	}
}	




//编写发送数据函数--发送字符串
//一个字符串自带的有结束标志位，就无需判断其长度了，因此只有一个参数
void Serial_SendString(char *String)
{
	uint8_t i;
	//字符0对应0x00，是空字符，对应字符结束标志位
	//也可改写为转义字符形式，即String[i]!='\0'
	for(i=0; String[i]!=0; i++)
	{
		Serial_SendByte(String[i]);
	}
}	




//编写发送数据函数---接收一个数字，以字符形式发送该数字
/*
要想发送字符形式，首先得把数字的每一个位取出来，具体的运算方法是：
要取第i的数：对原来的数先除以10的i次方，所得结果再用10取余。
例如：对于数字114514而言，要想取到千位的数字，
先：114514/10的3次方=114，再：114%10=4

我们先编写一个求幂次的函数，便于计算10的x次方
*/
uint32_t Serial_Pow(uint32_t x, uint32_t y)
{
	uint32_t Result = 1;
	uint32_t X = x;
	uint32_t Y = y;
	for(; Y>0; Y--)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for(i=0; i<Length; i++)
	{
		Serial_SendByte(Number / Serial_Pow(10,Length - i - 1) % 10 + 0x30);
		//字符0对应的是0x30，要想准确发送字符就得加上偏移量
	}
}
//
//重写fputc函数
/*
在c语言中，fputc函数将接收到的内容输出到电脑屏幕上，但在单片机中，我们要把数据发送到串口上，所以要重写函数
这个函数是printf函数的底层，将他重写后相当于将printf函数重写，使之输出到串口
*/
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}


//
//使用查询方法接收数据
////查询的具体流程：在主函数中循环判断串口的RXNE位，若为1，则寄存器不为空，即有数据进来了，然后调用ReceiveData函数
void ReceiveData_Inquire()
{
	char RX_data_Inquire;
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)          //判断UASRT1的RXNE位是否为1
	{
		RX_data_Inquire = USART_ReceiveData(USART1);                 //将这一个字节的数据写入RX_data_Inquire,这个函数在读取RD后同时会清除标志位，因此为无需手动清除。
		                                                             //当然，若为了保险，也可手动清除，下面 “使用中断的方法接收数据” 的函数中就采用了这一种方法。
		if(RX_data_Inquire == '#')
		{
			Delay_s(6000);
		
		}
		else if(RX_data_Inquire == '*')
		{
			Delay_s(1);
		}
		OLED_ShowChar(2, 1, RX_data_Inquire);                   //以1行、1列、长度为2的格式在OLED屏幕上显示RX_data_Inquire
	}
}

//使用中断的方法接收数据
/*
首先要在 初始化USART的代码中加上开启中断的代码
然后加上中断后接收数据的函数，这个函数在startup.stm32f10x.ms中封装好了，直接拿来并自己改写就可以了
*/
void USART1_IRQHandler(void)
{
	static uint8_t RxState = 0;   //定义标志变量
	static uint8_t pRxPacket = 0; //接收数据时是依次接收4个数据到数组Serial_RxPacket中，
	                              //在此过程中，我们需要判断当前接收了几个数据，就定义了这个变量
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)  //判断UASRT1的RXNE位是否为1
	{
		char RxData = USART_ReceiveData(USART1);
		
		
		
		if(RxData == '#')
		{
			RxState = 1;
			Serial_RxPacket[0] = '#';
		}
		else if(RxData == '*')
		{
			Serial_RxPacket[0] = '*';	
		}
		else
			RxState = 0;
		
	
		
		USART_ClearITPendingBit(USART1, USART_FLAG_RXNE);    //手动清除标志位
	}
	
}

 
//获取Serial_RxData的值 功能进行单独封装
/*
读取Serial_RxFlag的值，若为0，则返回0，若为1，则返回1，并将其重新置为0，实现清除标志位的功能
*/
uint8_t Serial_GetRxFlag(void)
{
	if(Serial_RxFlag == 0)
	{
		return 0;
	}
	else
	{
		Serial_RxFlag = 0;
		return 1;
	}
}



//编写函数：发送HEX数据包，具体功能为，当调用一次这个函数后，对收到的数据加上包头和包尾，然后发送
/*
 
*/
void Serial_SendPacket(void)
{
	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket, 4);
	Serial_SendByte(0xFE);
}



