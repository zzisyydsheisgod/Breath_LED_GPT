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

		// 调整占空比，实现从完全熄灭到完全点亮的呼吸效果
		// 使用更小的步进(1)和更短的延时，使变化更加平滑
		if(direction)
		{
			//duty += 5; // 每次只增加1，使变化更加平滑
			if(duty >= 1000)
			{
				duty = 1000;
				direction = 0; // 达到最亮，开始变暗
				delay(10);
			}
		}
		else
		{
			//duty -= 5; // 每次只减少1，使变化更加平滑
			if(duty <= 0)
			{
				duty = 0;
				direction = 1; // 达到完全熄灭，开始变亮
				delay(10);  // 在谷值处稍作停留
			}
		}

		delay(5);

    }

	return 0;
}