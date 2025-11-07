#include "bsp_delay.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_pwm.h"

// 软件PWM相关变量
static unsigned int pwm_duty = 0;       // 当前占空比 (0-1000)
static unsigned int pwm_period = 1000;  // PWM周期 (微秒)
static unsigned char direction = 1;     // 呼吸灯变化方向 (1=增加, 0=减少)
static unsigned int breath_step = 1;    // 呼吸灯变化步进
static unsigned char breath_enabled = 1; // 呼吸灯使能标志
static unsigned int pwm_counter = 0;    // PWM计数器

/*
 * @description	: 初始化GPT定时器用于PWM输出
 * @param 		: 无
 * @return 		: 无
 */
void gpt_pwm_init(void)
{
    // 配置GPIO1_IO03为普通GPIO功能
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x0U);  
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B1U);
    
    
    // 配置GPT1用于定时中断
    GPT1->CR = 0; //清零
    GPT1->CR = 1 << 15 ; //bit15置1,进入软复位
    while((GPT1->CR >> 15)&0x01); //等待复位完成

    /*
     * GPT的CR寄存器配置
     * bit15:   0   不处于复位状态
     * bit9:    0   Restart模式,当CNT等于OCR时产生中断并重置计数器
     * bit8:6   001 GPT时钟源选择ipg_clk = 66MHz
     */
    GPT1->CR = (1 << 6); // 基本配置，关闭输出比较模式
    
    // GPT的PR寄存器,分频设置为65,即66分频，定时器时钟为1MHz (1us精度)
    GPT1->PR = 65;

    // 设置中断周期为10us
    GPT1->OCR[0] = 10;

    // 清除中断标志位
    GPT1->SR |= 0x3F;

    /*
    * GPT 的 IR 寄存器，使能通道 1 的比较中断
    * bit0： 1 使能输出比较1中断
    */
    GPT1->IR |= 1 << 0;

    /*
    * 使能 GIC 里面相应的中断，并且注册中断处理函数
    */
    GIC_EnableIRQ(GPT1_IRQn); /* 使能 GIC 中对应的中断 */ 
    system_register_irqhandler(GPT1_IRQn,
                              (system_irq_handler_t)gpt1_irq_handler,
                              NULL);

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
    if(duty > pwm_period)
        duty = pwm_period;
    
    breath_enabled = 0; // 禁用呼吸灯效果
    pwm_duty = duty;
}

/*
 * @description	: 设置呼吸灯变化步进
 * @param - step: 步进值
 * @return 		: 无
 */
void gpt_pwm_set_breath_step(unsigned int step)
{
    breath_step = step;
}

/*
 * @description	: 启用呼吸灯效果
 * @param 		: 无
 * @return 		: 无
 */
void gpt_pwm_enable_breath(void)
{
    breath_enabled = 1;
}

/*
 * @description	: 禁用呼吸灯效果
 * @param 		: 无
 * @return 		: 无
 */
void gpt_pwm_disable_breath(void)
{
    breath_enabled = 0;
}

/*
 * @description	: GPT1中断服务函数
 * @param 		: 无
 * @return 		: 无
 */
void gpt1_irq_handler(unsigned int giccIar, void *param)
{
    static unsigned int breath_counter = 0;
    
    // 清除中断标志位
    GPT1->SR |= 1 << 0;
    
    // 软件PWM实现
    pwm_counter++;
    if(pwm_counter >= pwm_period) 
    {
        pwm_counter = 0;
    }
    
    if(pwm_counter < pwm_duty) 
    {
        gpio_pinwrite(GPIO1, 3, 1); // 输出高电平
    } 
    else 
    {
        gpio_pinwrite(GPIO1, 3, 0); // 输出低电平
    }
    
    // 如果启用了呼吸灯效果
    if(breath_enabled)
    {
        breath_counter++;
        // 每10次PWM周期更新一次呼吸灯效果，控制变化速度
        if(breath_counter >= 100)
        {
            breath_counter = 0;
            
            // 控制呼吸灯效果
            if(direction)
            {
                pwm_duty += breath_step; // 增加占空比
                if(pwm_duty >= pwm_period)
                {
                    pwm_duty = pwm_period;
                    direction = 0; // 达到最亮，开始变暗
                }
            }
            else
            {
                if(pwm_duty > breath_step)
                    pwm_duty -= breath_step; // 减少占空比
                else
                    pwm_duty = 0;
                    
                if(pwm_duty == 0)
                {
                    direction = 1; // 达到完全熄灭，开始变亮
                }
            }
        }
    }
}