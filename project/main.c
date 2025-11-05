 /**************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 mian.c
作者	   : 左忠凯
版本	   : V1.0
描述	   : I.MX6U开发板裸机实验11 定时器实现按键消抖实验
其他	   : 本实验主要学习如何使用定时器来实现按键消抖，以前的按键
	     消抖都是直接使用延时函数来完成的，这种做法效率不高，因为
	     延时函数完全是浪费CPU资源的。使用按键中断+定时器来实现按键
	     驱动效率是最好的，这也是Linux驱动所使用的方法！
论坛 	   : www.wtmembed.com
日志	   : 初版V1.0 2019/1/5 左忠凯创建
**************************************************************/
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_int.h"
#include "bsp_keyfilter.h"
#include "bsp_pwm.h"

/*
 * @description : main函数
 * @param       : 无
 * @return      : 无
 */
int main(void)
{
	unsigned int duty = 0;
	unsigned char direction = 1; // 1表示增加,0表示减少

	int_init();                 /* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();            /* 初始化系统时钟 */
	clk_enable();               /* 使能所有的时钟 */
	gpt_pwm_init();             /* 初始化PWM */

	while(1)            
	{   
		// 控制pwm占空比实现呼吸效果
		gpt_pwm_set_duty(duty);

		// 调整占空比
		if(direction)
		{
			duty += 5; // 逐渐增加
			if(duty >= 1000)
			{
				duty = 1000;
				direction = 0;
			}
		}
		else
		{
			duty -= 5; // 逐渐减少
			if(duty <= 0)
			{
				duty = 0;
				direction = 1; // 开始增加
			}
		}

		/* 延时一小段时间，控制呼吸速度 */
		delay(10);
	}

	return 0;
}
