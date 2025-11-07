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
	int_init();                 /* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();            /* 初始化系统时钟 */
	clk_enable();               /* 使能所有的时钟 */
	gpt_pwm_init();             /* 初始化PWM */
	
	/* 启用呼吸灯效果 */
	gpt_pwm_enable_breath();
	
	/* 设置呼吸灯步进值 */
	gpt_pwm_set_breath_step(1);

	while(1)            
	{   

    }

	return 0;
}