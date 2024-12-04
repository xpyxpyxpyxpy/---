#include "stm32f10x.h"                  // Device header

uint16_t MyDMA_Size;

//初始化DMA函数
/*
初始化DMA的步骤：
1.RCC开启DMA的时钟。
2.调用DMA_Init，初始化各种参数，

各种参数又包括：
1.外设的起始地址、数据宽度、地址是否自增；
2.储存器的起始地址、数据宽度、地址是否自增；（存储器包括flash和SRAM两种）
（1）两个起始地址共同规定了数据从哪里进行转运、转运到哪里。
（2）数据宽度规定了进行一次转运时，所能接受的最大宽度，
若当前转运的数据宽度 > 最大宽度，则从高位到低位开始截断；
若					 < 		   ，则从低位到高位开始补0
这个最大宽度可以设置为：
字节Byte（8位，对应uin8_t）、
半字HalfWord（相当于2个字节，16位，对应uint16_t）、
字Word（4个字节，32位，对应于uint32_t）
（3）地址是否自增规定了在完成这一次数据转运操作后，下一次转运是否把地址移动到下一个位置，


3.数据传输方向；
可为：外设-->存储器、存储器-->外设、存储器-->存储器
而由于flash是只读的，不能更改，因此存储器-->存储器又可细分为：
flash-->SRAM、SRAM-->SRAM，
SRAM-->flash、flash-->flash这两种是非法操作。
对于不同的数据传输方向，只需把两个起始地址改成对应的即可。

4.传输计数器：用于规定 总共需要转运几次数据，他是一个自减的计数器
当它减到0之后，就不会再进行数据转运了，且若有自增的地址，也会恢复到起始地址。

5.是否需要自动重装：用于规定 传输计数器自减为0后是否要恢复到原值


6.触发源：
（1）硬件触发
（2）软件触发
具体选择哪一个，由参数M2M决定：
M2M置1，为软件触发，M2M置0，为硬件触发。

软件触发的逻辑是：尽可能快地处发转运进而尽可能快地 使传输寄存器清0
因此，软件触发 和 自动重装 不能同时使用，否则会进入死循环
软件触发一般适用于 存储器-->存储器。

硬件触发的触发源：ADC、串口、定时器等，
硬件触发的逻辑是：在收到外设传来的一些信号后（如ADC转换完成、串口收到数据、定时时间到等）
，进行数据转运

7.通道优先级：
DMA的每个通道均可选择软件触发，
DMA的每个通道适用于不同的硬件触发，
优先级默认的是：通道号越小，优先级越高，也可通过程序手动配置


8.开关控制：DMA_Cmd()函数，对其使能（设为ENABLE）后，开启DMA


注：DMA开始转运时需满足以下条件：
1.DMA_Cmd函数是使能的。
2.传输计数器 >0。
3.触发源要有触发信号，触发一次，进行一次转运，传输计数器自减一次

当传输计数器自减为0且不设置自动重装时，不管有无触发，均不会再进行转运，
这时便要 DMA_Cmd函数不使能（设为DISABLE），关闭DMA，
再给传输计数器赋值，再使能DMA_Cmd()函数，再才开启DMA



*/
void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size)
{
	MyDMA_Size = Size; //将Size作一个全局备份，便于后面函数的使用
	
	//开启时钟--DMA是AHB总线的设备，因此要开启AHB时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	//初始化DMA
	//先定义初始化结构体，配置参数
	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;                           //设置外设基地址，对于stm32单片机，地址需要是32位的，而8位的16进制数就有32位
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     //设置一次转运数据的长度：一个字节
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;             //设置地址要自增
	DMA_InitStructure.DMA_MemoryBaseAddr = AddrB;								//设置存储器基地址
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             //设置一次转运数据的长度：一个字节
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //设置地址要自增
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          //设置传输方向：外设作为源头，即外设-->存储器
	DMA_InitStructure.DMA_BufferSize =  Size;                                   //设置缓存区大小，即给传输计数器赋值
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //设置传输计数器是否要自动重装，本次实验进行的是存储器-->存储器，无需重装，
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;                                 //设置地址要自增
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//设置优先级为中等                                 
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                //初始化DMA，是上述的那些参数生效
	
	DMA_Cmd(DMA1_Channel1,DISABLE);                                              //失能DMA      
}


//DMA传输函数，调用一次这个函数后，启动一次DMA转运
void MyDMA_Transfer(void)
{
	DMA_Cmd(DMA1_Channel1,DISABLE);                       //先失能DNA
	DMA_SetCurrDataCounter(DMA1_Channel1, MyDMA_Size);    //重新给传输计数器赋值
	DMA_Cmd(DMA1_Channel1,ENABLE);						  //重新使能DMA
	while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET)	  //判断数据是否完全转运,若为RESET，则没有完全转运
														  //继续等待，直至其完全转运，标志位变为SET
	DMA_ClearFlag(DMA1_FLAG_TC1);						  //转运完成后，这个标志位需要手动清除

}
