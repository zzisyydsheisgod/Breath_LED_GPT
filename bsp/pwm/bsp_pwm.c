#ifndef PWM_H
#define PWM_H

#include "imx6ul.h"

/*
 * @description	: 初始化GPT定时器用于PWM输出
 * @param 		: 无
 * @return 		: 无
 */
void gpt_pwm_init(void);

/*
 * @description	: 设置PWM占空比
 * @param - duty: 占空比(0-1000对应0%-100%)
 * @return 		: 无
 */
void gpt_pwm_set_duty(unsigned int duty);

/*
 * @description	: GPT1中断服务函数
 * @param 		: giccIar 中断号, param 参数
 * @return 		: 无
 */
void gpt1_irq_handler(unsigned int giccIar, void *param);

#endif // PWM_H
#include "bsp_delay.h"
#include "bsp_gpio.h"
#include "bsp_int.h"

// 软件PWM相关变量
static unsigned int pwm_duty = 0;       // 当前占空比 (0-1000)
static unsigned int pwm_period = 1000;  // PWM周期 (微秒)
static unsigned char pwm_state = 0;     // PWM状态 (0=低电平, 1=高电平)

/*
 * @description	: 初始化GPT定时器用于PWM输出
 * @param 		: 无
 * @return 		: 无
 */
void gpt_pwm_init(void)
{
    // 配置GPIO1_IO03为普通GPIO输出模式 (ALT5)
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x0U);  
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0U);
    
    // 初始化GPIO1_IO03为输出模式
    gpio_pin_config_t led_config;
    led_config.direction = kGPIO_DigitalOutput;
    led_config.outputLogic = 0;
    led_config.interruptMode = kGPIO_NoIntmode;
    gpio_init(GPIO1, 3, &led_config);

    // 设置初始GPIO为低电平
    gpio_pinwrite(GPIO1, 3, 0);

    // 配置GPT1用于定时中断
    GPT1->CR = 0; //清零
    GPT1->CR = 1 << 15 ; //bit15置1,进入软复位
    while((GPT1->CR >> 15)&0x01); //等待复位完成

    /*
     * GPT的CR寄存器配置
     * bit22:20 000 关闭输出比较功能
     * bit15:   0   不处于复位状态
     * bit9:    0   Restart模式,当CNT等于OCR时产生中断
     * bit8:6   001 GPT时钟源选择ipg_clk = 66MHz
     * bit3:0   010 分频值加1，即真正的分频值为3
     */
    GPT1->CR = (1 << 6) | (2 << 0);
    
    // GPT的PR寄存器,分频设置为65,即66分频，定时器时钟为1MHz (1us精度)
    GPT1->PR = 65;

    // 设置输出比较值,初始周期为100us (100个计数)
    GPT1->OCR[0] = 100;  // 100us中断周期

    // 清除中断标志位
    GPT1->SR |= 0x3F;

    // 使能GPT1输出比较1中断
    GPT1->IR |= 1 << 0;

    // 注册中断处理函数
    system_register_irqhandler(GPT1_IRQn, (system_irq_handler_t)gpt1_irq_handler, NULL);
    
    // 使能GPT1中断
    GIC_EnableIRQ(GPT1_IRQn);

    // 使能GPT1
    GPT1->CR |= (1 << 0);
}

/*
 * @description	: 设置PWM占空比
 * @param - duty: 占空比(0-1000对应0%-100%)
 * @return 		: 无
 */
void gpt_pwm_set_duty(unsigned int duty)
{
    if(duty > 1000)
        duty = 1000;
    
    pwm_duty = duty;
}

/*
 * @description	: GPT1中断服务函数
 * @param 		: 无
 * @return 		: 无
 */
void gpt1_irq_handler(unsigned int giccIar, void *param)
{
    static unsigned int counter = 0;
    
    // 清除中断标志位
    GPT1->SR |= 1 << 0;
    
    // 每次中断增加100us
    counter += 100;
    
    if(pwm_state == 0) {  // 当前处于低电平周期
        // 如果低电平时间已到（即周期减去占空比的时间），切换到高电平状态
        if(counter >= (pwm_period - pwm_duty)) {
            pwm_state = 1;  // 切换到高电平状态
            gpio_pinwrite(GPIO1, 3, 1);  // 设置GPIO高电平
        }
    } else {  // 当前处于高电平周期
        // 如果高电平时间已到（即占空比时间），切换到低电平状态
        if(counter >= pwm_duty) {
            pwm_state = 0;  // 切换到低电平状态
            gpio_pinwrite(GPIO1, 3, 0);  // 设置GPIO低电平
        }
    }
    
    // 当计数器达到周期值时重置
    if(counter >= pwm_period) {
        counter = 0;
    }
}