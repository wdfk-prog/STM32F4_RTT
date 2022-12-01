/**
 * @file lifter_motor.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-28
 * @copyright Copyright (c) 2022
 * @attention 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-28 1.0     HLY     first version
 */
/* Includes ------------------------------------------------------------------*/
#include "lifter_motor.h"
/* Private includes ----------------------------------------------------------*/
#include "motor.h"
#include "user_pwm.h"
#include "tim.h"
/*ulog include*/
#define LOG_TAG              "lifter_motor"
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define PWM_DEV_NAME        "pwm14"         /* PWM�豸���� */
#define PWM_DEV_CHANNEL     1               /* PWMͨ�� */
#define FREQ                1000            /* Hz*/
#define PERIOD              (1000*1000*1000 / FREQ)/* ����Ϊ1ms����λΪ����ns */

#define ENABLE_LVL          GPIO_PIN_RESET  //������ƽ
#define DISABLE_LVL         GPIO_PIN_SET    //ֹͣ��ƽ

#define MIN_OUTPUT          500             //��С�����Χ ��λmm
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
struct rt_device_pwm *pwm_dev;      /* PWM�豸��� */
lifter_motor_typedef lifter_motor;
/* Private function prototypes -----------------------------------------------*/
static void timer_callback(void *parameter);
/**
 * @brief  ������ PWM ��ʼ��
 * @retval int 
 */
static int proportional_valve_pwm_init(void)
{
    /* �����豸 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        LOG_W("pwm sample run failed! can't find %s device!", PWM_DEV_NAME);
        return RT_ERROR;
    }
    /* ����PWM���ں�������Ĭ��ֵ */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, PERIOD, 0);
    /* ʹ���豸 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);

    return RT_EOK;
}
/***********************************��ʼ������*********************************/
/**
  * @brief  �������ֹͣ����
  * @param  None.
  * @retval None.
  * @note   ���п�����������
*/
void lifter_motor_stop(lifter_motor_typedef *p)
{
  p->pump.IO.level         = DISABLE_LVL;
  p->spill_valve.IO.level  = DISABLE_LVL;

  p->state              = LIFT_STOP;

  rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL,PERIOD, 0);
  HAL_GPIO_WritePin(p->pump.IO.GPIOx,p->pump.IO.GPIO_Pin,p->pump.IO.level);
  HAL_GPIO_WritePin(p->spill_valve.IO.GPIOx,p->spill_valve.IO.GPIO_Pin,p->spill_valve.IO.level);
}
/**
  * @brief  ���������ͣ���ȼ�.
  * @param  None.
  * @retval None.
  * @note   None.
*/
uint8_t lifter_motor_stop_priority(lifter_motor_typedef* p)
{
    if(*p->stop_state != NO_STOP)
    {
        lifter_motor_stop(p);
        return 1;
    }
    else
      return 0;
}
/**
  * @brief  �������������ʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
uint8_t lifter_motor_jack_init(lifter_motor_typedef *p)
{
  static uint8_t cnt = 0;
  //��ֹ�������˲���������
  p->pump.IO.level        = DISABLE_LVL;
  p->spill_valve.IO.level = ENABLE_LVL;
  HAL_GPIO_WritePin(p->pump.IO.GPIOx,p->pump.IO.GPIO_Pin,p->pump.IO.level);
  HAL_GPIO_WritePin(p->spill_valve.IO.GPIOx,p->spill_valve.IO.GPIO_Pin,p->spill_valve.IO.level);
  //��ʱ0.1��ر�
  if(cnt++ >= 100 / p->period)
  {
    p->pump.IO.level        = ENABLE_LVL;
    p->spill_valve.IO.level = DISABLE_LVL;
    HAL_GPIO_WritePin(p->pump.IO.GPIOx,p->pump.IO.GPIO_Pin,p->pump.IO.level);
    HAL_GPIO_WritePin(p->spill_valve.IO.GPIOx,p->spill_valve.IO.GPIO_Pin,p->spill_valve.IO.level);
    cnt = 0;
    return 1;
  }
  return 0;
}
/**
  * @brief  ��������½���ʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
void lifter_motor_fall_init(lifter_motor_typedef *p)
{
  p->pump.IO.level         = DISABLE_LVL;
  p->spill_valve.IO.level  = ENABLE_LVL;
  HAL_GPIO_WritePin(p->pump.IO.GPIOx,p->pump.IO.GPIO_Pin,p->pump.IO.level);
  HAL_GPIO_WritePin(p->spill_valve.IO.GPIOx,p->spill_valve.IO.GPIO_Pin,p->spill_valve.IO.level);
}
/**
  * @brief  ���������ʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
void lifter_motor_init(void)
{
  lifter_motor.period = 10;//ms

  lifter_motor.pump.IO.GPIOx            = PUMP_GPIO_Port;
  lifter_motor.pump.IO.GPIO_Pin         = PUMP_Pin;

  lifter_motor.spill_valve.IO.GPIOx     = SPILL_VALVE_GPIO_Port;
  lifter_motor.spill_valve.IO.GPIO_Pin  = SPILL_VALVE_Pin;

  lifter_motor.min_output = MIN_OUTPUT;//���������Χ

  lifter_motor.pump.PID.P                   = 2;
  lifter_motor.pump.PID.D                   = 0.1;
  lifter_motor.pump.PID.OutputMax           = 10000;
  
  lifter_motor.spill_valve.PID.P            = 2;
  lifter_motor.spill_valve.PID.D            = 0.1;
  lifter_motor.spill_valve.PID.OutputMax    = 10000;

  proportional_valve_pwm_init();

//  lifter_motor_stop(&lifter_motor);

  rt_err_t ret = RT_EOK;
  /* ��ʱ���Ŀ��ƿ� */
  static rt_timer_t timer;
  /* ������ʱ�� 1  ���ڶ�ʱ�� */
  timer = rt_timer_create("lift", timer_callback,
                           RT_NULL, rt_tick_from_millisecond(1),
                           RT_TIMER_FLAG_PERIODIC);

  /* ������ʱ�� 1 */
  if (timer != RT_NULL) rt_timer_start(timer);
}
/**********************************************************************************/
/**
 * @brief  �������PID
 * @param  target Ŀ��ֵ         
 * @param  fb     ����ֵ
 * @param  p                
 * @retval 0X00,�ɹ�;0x01,������ͣ;0X02,����Сֹͣ;0XFF,ʧ��;0XFE,����쳣;
 */
uint8_t lifter_motor_pid(int target,int fb,lifter_motor_typedef *p)
{
  uint16_t duty = 0;//ռ�ձ�
  uint32_t output = 0;

  if(lifter_motor_stop_priority(p))
  { 
      return 0x01;
  }
  int error = target - fb;//���

  //����С��ֹͣ���
  if(-p->min_output <= error && error <= p->min_output)
  {
      lifter_motor_stop(p);
      return 0X02;
  }
  else if(error > 0)//��Ҫ����
  {
      if(p->state == LIFT_STOP || p->state == LIFT_FALL)
      {
          if(!lifter_motor_jack_init(p))
          {
              duty = 0;
              goto run;
          }
          else
          {
            p->state = LIFT_JACK;
          }
      }
      if(p->state == LIFT_JACK)
      {
          duty = PID_Cal(&p->pump.PID,fb,target);
      }
  }
  else if(error < 0)//��Ҫ�½�
  {
      if(p->state == LIFT_STOP || p->state == LIFT_JACK)
      {
           lifter_motor_fall_init(p);
           p->state = LIFT_FALL;
           duty = 0;
      }

      if(p->state == LIFT_FALL)
      {
          duty = -1*PID_Cal(&p->spill_valve.PID,fb,target);
      }
  }

run:
  if(duty < 0 || duty > 10000)
  {
      return 0XFE;
  }
  output = PERIOD / 10000 * duty;
  return rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, PERIOD, output);
}
/**
  * @brief  ��ʱ���ص�����.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void timer_callback(void *parameter)
{
    static uint16_t cnt2 = 0;

    ++cnt2;
    if(!(cnt2 % lifter_motor.period))
    {
        lifter_motor_pid(*lifter_motor.target,*lifter_motor.feedback,&lifter_motor);
        cnt2 = 0;
    }
}