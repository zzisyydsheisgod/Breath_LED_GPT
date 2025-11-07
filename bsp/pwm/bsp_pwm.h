#ifndef __BSP_PWM_
#define __BSP_PWM_

void gpt_pwm_init(void);

void gpt_pwm_set_duty(unsigned int duty);

void gpt1_irq_handler(unsigned int giccIar, void *param);

void gpt_pwm_set_breath_step(unsigned int step);

void gpt_pwm_enable_breath(void);

void gpt_pwm_disable_breath(void);

#endif