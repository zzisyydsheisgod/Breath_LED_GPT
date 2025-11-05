/***************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 bsp_delay.c
作者	   : 左忠凯
版本	   : V1.0
描述	   : 延时文件。
其他	   : 无
论坛 	   : www.wtmembed.com
日志	   : 初版V1.0 2019/1/4 左忠凯创建
***************************************************************/
#include "bsp_delay.h"

/*
* @description	: 延时有关硬件初始化,主要是GPT定时器
* @param 		: 无
* @return 		: 无
*/

void delay_init(void)
{
   GPT1->CR = 0;		//清零
   GPT1->CR = 1 <<15;	//bit15置1,进入软复位,SWR(bit15)
   while((GPT1->CR>>15)&0x01); //等待复位完成

   /*
   * GPT的CR寄存器,GPT通用设置
   * bit22:20 000输出比较1的输出功能关闭,也就是对应的引脚没反应
   * bit9:   0   Restart模式,当CNT等于OCR1的时候产生中断
   * bit8:6  001 GPT时钟源选择ipc_clk = 66MHz
   */
   GPT1->CR = (1<<6);


   /*
   *GPT的PR寄存器,GPT的分频设置
   *bit11:0  设置分辨率,设置为0表示1分频
   *		 以此类推,最大可以设置为0xFFF,也就是最大4096分频
   */
   GPT1->PR = 65; //66分频,GPT1时钟为66M/(65+1)=1MHz


   /*
   *GPT的OCR1寄存器,GPT的输出比较1比较计数值
   *GPT的时钟为1MHz,那么计数器每计一个值就是1us
   *为了实现较大的计数,我们将比较值设置为最大的0xFFFFFFFF
   *这样一次计满就是0xFFFFFFFF us=4294967296us = 4295s = 71.5min
   *也就是说一次计满最多71.5分钟，存在溢出
   */
   GPT1->OCR[0] = 0xFFFFFFFF;
   GPT1->CR |= 1<<0;   //使能GPT1

   /*
   *以下屏蔽的代码是GPT定时器中断代码
   *如果想学习GPT定时器的话可以参考以下代码
   */

#if 0
   /*
   *GPT的PR寄存器,GPT的分频设置
   *bit11:0 设置分辨率,设置为0表示1分频
   *		 以此类推,最大可以设置为0xFFF,也就是最大4096分频
   */
   GPT1->PR = 65; /* 66 分频，GPT1 时钟为 66M/(65+1)=1MHz */


   /*
   *GPT1的OCR1寄存器,GPT的输出比较1比较计数值
   *当GPT的计数值等于OCR1里面值时候,输出比较1就会发生中断
   *这里定时500ms产生中断,因此就应该为1000000/2 = 500000;
   */
   GPT1->OCR[0] = 500000;

   /*
   *GPT的IR寄存器,使能通道1的
   *bit [0:0]使能输出比较中断
   */
  GPT1->IR |= 1<<0;

  /*
  *使能GIC里面相应的中断,并且注册中断处理函数
  */
 GIC_EnableIRQ(GPT1_IRQn); //使能GIC中对应的中断
 system_register_irqhandler(GPT1_IRQn,
                           (system_irq_handler_T)gpt1_irqhandler,
                           NULL);



#endif

#if 0

//中断处理函数
void gpt1_irqhandler(void)
{
   static unsigned char state = 0;
   state = !state;

   /*
   * GPT的SR寄存器,状态寄存器
   * bit [2:1] 输出比较1发生中断
   */
  if((GPT1->SR & (1<<0)))
   {
    led_switch(LED2,state);
   }
   GPT1->SR |= 1<<0; //清楚中断标志位

}

#endif

}


/* 
 * 微妙(us)级延时
 * @param - usdelay : 需要延时的us数,最大延时0xFFFFFFFFus
 * @return 		: 无
 */
void delayus(unsigned int usdelay)
{
	unsigned long old_count, new_count;
	unsigned long sum_count = 0; /* 走过的总时间 */

	old_count = GPT1->CNT;

	while(1)
	{
		new_count = GPT1->CNT; /* 获取当前计数器值 */

		if(new_count != old_count)
		{
			if(new_count > old_count) /* GPT是向上计数器,并且没有溢出 */
				sum_count = sum_count + (new_count - old_count);
			else /* 溢出 */
				sum_count = sum_count + (0xFFFFFFFF - old_count + new_count);
		}
		old_count = new_count;
		
		if(sum_count >= usdelay) /* 延时时间到了 */
			break; /* 跳出 */
	}
}


 /*
 * @description : 毫秒(ms)级延时
 * @param - msdelay : 需要延时的 ms 数
 * @return : 无
 */
 void delayms(unsigned int msdelay)
 {
   int i = 0;

   for(i = 0; i<msdelay; i++)
   {
      delayus(1000);
   }
 }



/* 
 * 短时间延时函数
 * @param - n	: 要延时循环次数(空操作循环次数，模式延时)
 * @return 		: 无
 */
void delay_short(volatile unsigned int n)
{
	while(n--){}
}
/*
 * @description: 毫秒级延时函数，在 396MHz 主频下约等于 1ms * n
 * @param[in]   n: 需要延时的毫秒数
 * @return      无
 */
void delay(volatile unsigned int n)
{
	while(n--)
	{
		delay_short(0x7ff);
	}
}


