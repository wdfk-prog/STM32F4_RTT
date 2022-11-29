/**
  ******************************************************************************
  * @file   
  * @brief   
  * @date   
  ******************************************************************************
  * @attention  
  * @author HLY
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "lifter_motor.h"
/* Private includes ----------------------------------------------------------*/
#include "mb_handler.h"
#include "adc_dma.h"
#include "stdlib.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define LIFT  (lift)
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
Lifter_Motor_TypeDef  lift;
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ֹͣ
  * @param  None.
  * @retval None.
  * @note   ����
            0 0
*/
void Lifter_Motor_Stop(Lifter_Motor_TypeDef *p)
{
  p->Jack.IO.level = GPIO_PIN_SET;
  p->Fall.IO.level = GPIO_PIN_SET;
  p->state         = LIFT_STOP;
  HAL_GPIO_WritePin(p->Jack.IO.GPIOx,p->Jack.IO.GPIO_Pin,p->Jack.IO.level);
  HAL_GPIO_WritePin(p->Fall.IO.GPIOx,p->Fall.IO.GPIO_Pin,p->Fall.IO.level);
}
/**
  * @brief  ���ߵ����ͣ���ȼ�.
  * @param  None.
  * @retval None.
  * @note   None.
*/
uint8_t Lifter_Motor_Stop_Priority(Lifter_Motor_TypeDef* p)
{
    return 0;
}
/**
  * @brief  ���������������
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_drive(Lifter_Motor_TypeDef *p)
{
  static uint16_t count = 0;
  if(Lifter_Motor_Stop_Priority(p))
    return;
  if(p->last_state != p->state)//״̬�л�ʱ
  {
      if(p->last_state != LIFT_STOP)//����ֹͣ״̬�л�
      {
        if(p->state == LIFT_STOP)
        {
          HAL_GPIO_WritePin(p->Jack.IO.GPIOx,p->Jack.IO.GPIO_Pin,p->Jack.IO.level);
          HAL_GPIO_WritePin(p->Fall.IO.GPIOx,p->Fall.IO.GPIO_Pin,p->Fall.IO.level);
          return;
        }
        count++;
        if(count > p->Jack.min_RTFC)
        {
          count = 0;
          if(p->last_state == LIFT_JACK)
          {
            p->Jack.IO.level = GPIO_PIN_RESET;
            p->Fall.IO.level = GPIO_PIN_SET;
          }
          else if(p->last_state == LIFT_FALL)
          {
            p->Jack.IO.level = GPIO_PIN_SET;
            p->Fall.IO.level = GPIO_PIN_RESET;
          }
          HAL_GPIO_WritePin(p->Jack.IO.GPIOx,p->Jack.IO.GPIO_Pin,p->Jack.IO.level);
          HAL_GPIO_WritePin(p->Fall.IO.GPIOx,p->Fall.IO.GPIO_Pin,p->Fall.IO.level);
        }
        else
        {
          Lifter_Motor_Stop(p);
        }
      }
      else
      {
        HAL_GPIO_WritePin(p->Jack.IO.GPIOx,p->Jack.IO.GPIO_Pin,p->Jack.IO.level);
        HAL_GPIO_WritePin(p->Fall.IO.GPIOx,p->Fall.IO.GPIO_Pin,p->Fall.IO.level);
      }
  }
  else
  {
    HAL_GPIO_WritePin(p->Jack.IO.GPIOx,p->Jack.IO.GPIO_Pin,p->Jack.IO.level);
    HAL_GPIO_WritePin(p->Fall.IO.GPIOx,p->Fall.IO.GPIO_Pin,p->Fall.IO.level);
  }
}
/**
  * @brief  ����
  * @param  None.
  * @retval None.
  * @note   ����
            1 0
*/
void Lifter_Motor_Jack(Lifter_Motor_TypeDef *p)
{
  p->Jack.IO.level = GPIO_PIN_RESET;
  p->Fall.IO.level = GPIO_PIN_SET;
  p->last_state    = p->state;
  p->state         = LIFT_JACK;
}
/**
  * @brief  �½�
  * @param  None.
  * @retval None.
  * @note   ����
            0 1
*/
void Lifter_Motor_Fall(Lifter_Motor_TypeDef *p)
{
  p->Jack.IO.level = GPIO_PIN_SET;
  p->Fall.IO.level = GPIO_PIN_RESET;
  p->last_state    = p->state;
  p->state         = LIFT_FALL;
}
/**
  * @brief  ����Ŀǰλ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_Set_Target(Lifter_Motor_TypeDef *p,float tar)
{
  p->max_position = lifter_encoder.height_20ma;
  p->min_position = 0;

  if(tar > p->max_position)
    tar = p->max_position;
  else if(tar < p->min_position)
    tar = p->min_position;
  else
    p->target = tar;

  if(p->target < 0)
    p->target = 0;
}
/**
  * @brief  �������PID
  * @param  None.
  * @retval None.
  * @note   None.
ע���PID�������������½�ʵ�ַ�ʽ��һ����
���ò�ͬPID������
���������������½������С.�������������������ʱ������һ���½�����,��С���.
���ô��Pֵ��С����Χ��
*/
void Lifter_Motor_PID(Lifter_Motor_TypeDef *p)
{
  Rope_Encoder_Get_Postion();
  p->now   = lifter_encoder.position;//Ŀǰ��ֵ

  p->error = p->target - p->now;
  
  if(p->target == 0 && abs(p->error) >= 10)//������͵���û�ԭ�������
  {
    if(p->do_reset_flag == ZERO_INIT)//û����ɻ�ԭ�����ٴ�����
    {
      LIFT_LOCATE_RESET;
      LIFT_ZERO_SET;
    }
    return;
  }
  LIFT_ZERO_RESET;//ȡ����ԭ���ڶ�λʱ��ԭ�����жϣ������ͻ
  if(0 < p->error && p->error <= 10)
  {
    p->Jack.flag  = DISABLE;
    p->Jack.RTFC  = 0;
    p->Jack.count = 0;
    return;
  }
  else if(-10 < p->error && p->error < 0)
  {
    p->Fall.flag  = DISABLE;
    p->Fall.RTFC  = 0;
    p->Fall.count = 0;
    return;
  }
  
  if(p->error == 0)
  {
    p->Jack.flag  = DISABLE;
    p->Jack.RTFC  = 0;
    p->Jack.count = 0;
    p->Fall.flag  = DISABLE;
    p->Fall.RTFC  = 0;
    p->Fall.count = 0;
    return;
  }
  else if(p->error > 0)//��Ҫ����
  {
    p->Fall.flag  = DISABLE;
    
    p->Jack.RTFC = PID_Cal(&p->Jack.PID,p->now,p->target);
    if(p->Jack.RTFC <= p->Jack.min_RTFC)//��ͨʱ��С����С��ͨʱ��
    {
      p->Jack.flag  = DISABLE;
      p->Jack.RTFC  = 0;
      p->Jack.count = 0;
    }
    else if(p->Jack.RTFC >= 2000)
    {
      p->Jack.flag  = DISABLE;
      p->Jack.RTFC  = 0;
      p->Jack.count = 0;
    }
    else
      p->Jack.flag = ENABLE;//��ʼ��ͨ
  }
  else if(p->error < 0)//��Ҫ�½�
  {
    p->Jack.flag  = DISABLE;
    p->Fall.RTFC = -1*PID_Cal(&p->Fall.PID,p->now,p->target);
    if(p->Fall.RTFC <= p->Fall.min_RTFC)//��ͨʱ��С����С��ͨʱ��
    {
      p->Fall.flag  = DISABLE;
      p->Fall.RTFC  = 0;
      p->Fall.count = 0;
    }
    else if(p->Fall.RTFC >= 2000)
    {
      p->Jack.flag  = DISABLE;
      p->Jack.RTFC  = 0;
      p->Jack.count = 0;
    }
    else
      p->Fall.flag = ENABLE;//��ʼ��ͨ
  }
}
/**
  * @brief  None.
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_Elevator(Lifter_Motor_TypeDef *p)
{
  if(LIFT_BREAK_GET == 1)
  {
    Lifter_Motor_Stop(&LIFT);
    return;
  }
  /*���˶�ʱ��ԭ�������ǰ��*/
  if(p->target == 0 && p->error !=0)//������͵���û�ԭ�������
  {
    p->Jack.flag  = DISABLE;
    p->Jack.RTFC  = 0;
    p->Jack.count = 0;
    p->Fall.flag  = DISABLE;
    p->Fall.RTFC  = 0;
    p->Fall.count = 0;
    return;
  }
  
  if(p->Jack.flag == ENABLE && p->Fall.flag == ENABLE)
  {
    if(p->state == LIFT_JACK)
    {
      p->Jack.count = 0;
      p->Jack.flag = DISABLE;
    }
    else if(p->state == LIFT_FALL)
    {
      p->Fall.count = 0;
      p->Fall.flag = DISABLE;
    }
  }
  
  if(p->Jack.flag == ENABLE)
  {
    if(p->Jack.count >= p->Jack.RTFC)
    {
      p->Jack.count = 0;
      Lifter_Motor_Stop(&LIFT);
    }
    else
    {
      p->Jack.count++;
      Lifter_Motor_Jack(&LIFT);
    }
  }
  else if(p->Fall.flag == ENABLE)
  {
    if(p->Fall.count >= p->Fall.RTFC)
    {
      p->Fall.count = 0;
      Lifter_Motor_Stop(&LIFT);
    }
    else
    {
      p->Fall.count++;
      Lifter_Motor_Fall(&LIFT);
    }
  }
  else
  {
    Lifter_Motor_Stop(&LIFT);
  }
}
/**
  * @brief  ����Ƿ�ֹ��ѯ
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_Static_Query(Lifter_Motor_TypeDef *p)
{
  static uint16_t temp = 0,count = 0;//��һ��λ��
  int16_t error = 0;

  temp = lifter_encoder.position;
  error = temp - lifter_encoder.position;

  if(error == 0)
  {
    if(count == 2000)
    {
      if(p->do_reset_flag != ZERO_INIT)
      {
        p->encoder_flag = ZERO_OK;//��������ֹ
        count = 0;
      }
    }
    else
    {
      count++;//�ȴ�����
    }
  }
  else
  {
    count = 0;
    p->encoder_flag = LOW_LEAVE;//�������仯
  }
}
/**
  * @brief  ��ۻ���
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_Do_Reset(Lifter_Motor_TypeDef *p)
{
  if(HAL_GPIO_ReadPin(Lower_Limit_GPIO_Port,Lower_Limit_Pin) == GPIO_PIN_RESET)
  {
    p->Lower_Limit.state = SENSOR_TOUCH;
    LIFT_LOWER_SET;
  }
  else
  {
    p->Lower_Limit.state = SENSOR_LEAVE;
    LIFT_LOWER_RESET;
  }
    
  if(p->Lower_Limit.state == SENSOR_TOUCH)//����λ�Ӵ�
  {
    if(p->encoder_flag == ZERO_OK)//��������ֹ
    {
      Lifter_Motor_Stop(p);
      p->do_reset_flag = ZERO_OK;//�½����
      p->encoder_flag = ZERO_OK;
    }
    else//�������仯
    {
      Lifter_Motor_Fall(p);
      p->do_reset_flag = QUICK_TOUCH;//�����½�
      p->encoder_flag = QUICK_TOUCH;
    }
  }
  else//����λδ�Ӵ�
  {
    Lifter_Motor_Fall(p);
    p->do_reset_flag = QUICK_TOUCH;//�����½�
    p->encoder_flag = QUICK_TOUCH;
  }
}
/**
  * @brief  ���������λ����.
  * @param  
  * @retval None.
  * @note   ��λģʽ mode:1���ж����ޡ�2���ж����ޡ�3��˫���жϡ�4������λ
*/
void Lift_Motor_Limiting(Lifter_Motor_TypeDef* p)
{
  p->max_position = p->max_lenth;
  p->min_position = p->min_lenth;
  Rope_Encoder_Get_Postion();
  p->now       = lifter_encoder.position;
  switch(LIFT.limit_mode)
  {
    case 1:
      if(p->now > LIFT.max_position)
      {
        Lifter_Motor_Stop(&LIFT);
      }
      break;
    case 2:
      if(p->now < LIFT.min_position)
      {
        Lifter_Motor_Stop(&LIFT);
      }
      break;
    case 3:
      if(LIFT.incoming - p->now > 0)
      {
        if(p->now > LIFT.max_position)
        {
          Lifter_Motor_Stop(&LIFT);
        }
      }
      else if(LIFT.incoming - p->now < 0)
      {
        if(p->now < LIFT.min_position)
        {
          Lifter_Motor_Stop(&LIFT);
        }
      }
      break;
    case 4:
      break;
    default:
      Lifter_Motor_Stop(&LIFT);
      break;
  }
}
/************************�û���ʼ������******************************************/
/**
  * @brief  None.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void Lift_Motor_1MS(void *parameter)
{
  static uint16_t cnt2 = 0;
  Rope_Encoder_Get_Postion();
  if(LIFT_LOCATE_GET == 1)
  {
    ++cnt2;
    if(!(cnt2 % LIFT.period))
    {
      Lifter_Motor_PID(&LIFT);
      cnt2 = 0;
    }
    Lifter_Motor_Elevator(&LIFT);
  }
  Lifter_Motor_drive(&LIFT);
}
/**
  * @brief  ��������ṹ���ʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_Struct_Init(void)
{
  LIFT.period               = 10;
  
  LIFT.max_lenth            = ROPE_20MA_HEIGHT;
  LIFT.min_lenth            = ROPE_4MA_HEIGHT;
  LIFT.max_position = LIFT.max_lenth;
  LIFT.min_position = LIFT.min_lenth;
  
  LIFT.Jack.IO.GPIOx    = Jack_Relay_GPIO_Port;
  LIFT.Jack.IO.GPIO_Pin = Jack_Relay_Pin;
  
  LIFT.Fall.IO.GPIOx    = Fall_Relay_GPIO_Port;
  LIFT.Fall.IO.GPIO_Pin = Fall_Relay_Pin;
  
  LIFT.Jack.min_RTFC        = 50;//ms

  LIFT.Jack.PID.P           = 2;
#ifdef 	PID_INTEGRAL_ON
  LIFT.Jack.PID.I           = 0.1;
  LIFT.Jack.PID.Integral    = 1000;
#endif
  LIFT.Jack.PID.D           = 0.1;
  LIFT.Jack.PID.OutputMax   = 1000;
  
  LIFT.Fall.IO.GPIOx        = Fall_Relay_GPIO_Port;
  LIFT.Fall.IO.GPIO_Pin     = Fall_Relay_Pin;
  
  LIFT.Fall.min_RTFC        = 50;//ms
  LIFT.Fall.PID.P           = 2;
#ifdef 	PID_INTEGRAL_ON
  LIFT.Fall.PID.I           = 0.1;
  LIFT.Fall.PID.Integral    = 1000;
#endif
  LIFT.Fall.PID.D           = 0.1;
  LIFT.Fall.PID.OutputMax   = 1000;
}
/**
  * @brief  Lifter_Motorʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Lifter_Motor_Init(void)
{
  Lifter_Motor_Struct_Init();
  Lifter_Motor_Stop(&LIFT);
  
  rt_err_t ret = RT_EOK;
  /* ��ʱ���Ŀ��ƿ� */
  static rt_timer_t timer;
  /* ������ʱ�� 1  ���ڶ�ʱ�� */
  timer = rt_timer_create("lift 1ms", Lift_Motor_1MS,
                             RT_NULL, rt_tick_from_millisecond(1),
                             RT_TIMER_FLAG_PERIODIC);

  /* ������ʱ�� 1 */
  if (timer != RT_NULL) rt_timer_start(timer);
}