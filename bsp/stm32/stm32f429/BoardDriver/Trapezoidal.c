/**
  ******************************************************************************
  * @file    Trapezoidal.c
  * @brief   ���μӼ�������
  * @date    2022.09.04
  ******************************************************************************
  * @attention  �ع�����������������

  �ٶȿ��Ʒ�ʽ.PWM����������߷���ռ�ձ�
  ���Ҳ���ò��7S�Ӽ��ٷ�ʽ�������ٶȱ仯���ȴ���Ҫռ�ϴ��ڴ档��Ҫһ��������
  �������μӼ��ٷ�ʽ������Ҫ�ϴ��ڴ�,����򵥣�����Ҫ�����жϼ�¼����������ơ�
  ��������Ҫ����λ�ã��Ż��7s�Ӽ��ٵ��ٶȿ��ơ�����������μӼ���Ч�����á�
  * @author HLY
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "Trapezoidal.h"
/* Private includes ----------------------------------------------------------*/
#include "stdlib.h"
#include "user_math.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  Trapezoidalʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Trapezoidal_Init(Trapezoidal_TypeDef *p,u16 reduction_ratio,
int16_t max_speed,int16_t min_speed,TIM_HandleTypeDef* TIMx, u32 Channel,
uint16_t time,uint16_t step,GPIO_PinState clockwise,u32 cycle_pulse_num)
{
  p->reduction_ratio  = reduction_ratio;
  p->max_speed        = max_speed;
  p->min_speed        = min_speed;
  p->TIMx             = TIMx;
  p->Channel          = Channel;
  p->AND.time         = time;
  p->AND.step         = step;
  p->clockwise        = clockwise;
  
  p->Motor_init(p->TIMx);
  p->Motor_Enable();
}
/**
  * @brief  ���μӼ����ٶ��������
  * @param  p ����������
  * @param  speed ���ת�� ת�ٵ�λ[RPM��ת/����]
  * @retval None
  * @note   
*/
MOVE_state Trapezoidal_Out_Speed(Trapezoidal_TypeDef* p,int16_t v)
{
  if(p->Motor_Stop_Priority())
    return MOVE_STOP;

  /*��Χ�ж�*/
  if(v == 0)
    v = 0;
  else if(abs(v) > p->max_speed * (float)p->reduction_ratio)
    v = p->max_speed * ((v > 0) ? 1 : -1);
  else if(abs(v) < p->min_speed * (float)p->reduction_ratio)
    v = p->min_speed * ((v < 0) ? 1 : -1);

  //�����
  p->AND.target = v * (int16_t)p->cycle_pulse_num / 60;//RPM
  return MOVE_UNDONE;
}
/**
  * @brief  �Ӽ��ٺ���.
  * @param  None.
  * @retval None.
  * @note   None.
*/
MOVE_state Trapezoidal_AND(Trapezoidal_TypeDef* p)
{
  int32_t error_target;
  int8_t set_value_sign,target_sign;
  int8_t D_flag; //�Ƿ���   1���� 0 ����
  uint8_t and_flag;//���㷽ʽѡ��
  
  if(p->Motor_Stop_Priority())
    return MOVE_STOP;
  
  if(p->AND.set_value == p->AND.target)
  {
    goto end;
  }

  error_target = p->AND.target - p->AND.last_target;
  if(abs(error_target) >= p->AND.step)//Ŀ��ֵ�б仯����Ӽ��ٴ���
  {
     p->AND.last_target = p->AND.target;
  }
  else if(error_target != 0)//Ŀ��ֵ�仯С�����ޱ仯
  {
    p->AND.set_value = p->AND.target;
    goto end;
  }

  set_value_sign = Compute_Sign_INT(p->AND.set_value,32);
  target_sign    = Compute_Sign_INT(p->AND.target,32);
  //��ֵΪ0ʱ��Ҳ�ǲ���Ҫ����
  D_flag = set_value_sign * target_sign;
  
  if(D_flag == -1)//��Ҫ����
  {
    if(set_value_sign == 1)//�ɸ�����
    {
      and_flag = 0;
    }
    else
      and_flag = 1;
  }
  else
  {
    if(abs(p->AND.set_value) < abs(p->AND.target))//����
    {
      if(target_sign != -1)//���ǷǸ���
      {
        and_flag = 1;
      }
      else
      {
        and_flag = 0;
      }
    }
    else//����
    {
      if(target_sign != -1)//���ǷǸ���
      {
        if(set_value_sign == 1)//������0
          and_flag = 0;
        else    //������0
          and_flag = 1;
      }
      else
      {
        and_flag = 1;
      }
    }
  }
  
  if(and_flag == 1)//�ӷ�
  {
    p->AND.set_value += p->AND.step;
    if(p->AND.set_value > p->AND.target)
      p->AND.set_value = p->AND.target;
  }
  else//����
  {
    p->AND.set_value -= p->AND.step;
    if(p->AND.set_value < p->AND.target)
      p->AND.set_value = p->AND.target;
  }

  end:
  if(p->AND.set_value > 0)
  {
    p->dir = CW;                                       //˳ʱ��
  }
  else if(p->AND.set_value < 0)
  {
    p->dir    = CCW;                                      //��ʱ��
  }
  p->Motor_Direction(p->dir);

  p->set_freq = (abs(p->AND.set_value) * (float)p->reduction_ratio);
  if(p->set_freq == 0)
  {
     p->Motor_Stop();
     return MOVE_DONE;
  }
  else
  {
    p->Modify_Freq(p->TIMx,p->Channel,p->set_freq);
    p->Motor_Start();
    return MOVE_UNDONE;
  }
}