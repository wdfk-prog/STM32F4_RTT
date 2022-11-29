/**
  ******************************************************************************
  * @file    pwm.c
  * @brief   PWM����V1.0
  * @date    2021.01.06
  ******************************************************************************
  * @attention  ������ʱ����ʼ��Ҫ��pwm�����ʼ��ǰ��
  * @author HLY
  �Ż�Walk_Motor�����ʼ������             --2022.04.18
  �޸�Modify_TIM_Freq()�и���Ƶ�ʹ��ߵ���ռ�ձ���Ƶ�������󣬲�����������궨��  --2022.07.20
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "motor.h"
/* Private includes ----------------------------------------------------------*/
#include "tim.h"
#include "user_math.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ���PWM��ʼ��
  * @param  None
  * @retval None
  * @note   None��
*/
void Motor_PWM_init(TIM_HandleTypeDef* TIMx)
{
  __HAL_TIM_CLEAR_FLAG(TIMx,TIM_FLAG_UPDATE);
//  __HAL_TIM_ENABLE_IT(TIMx,TIM_IT_UPDATE);
  __HAL_TIM_DISABLE(TIMx);
}
/**
  * @brief  PWMֹͣ����
  * @param  TIMx   : TIM1 - TIM17
  * @param  Channel��ʹ�õĶ�ʱ��ͨ������Χ1 - 4
  * @retval PWM_StatusTypeDef��pwmֹͣ
  * @note   None
*/
PWM_StatusTypeDef Motor_PWM_Stop(TIM_HandleTypeDef* TIMx,u32 Channel)
{
  HAL_TIM_PWM_Stop(TIMx,Channel);
  return PWM_STOP;
}
/**
  * @brief  PWM��ʼ����
  * @param  TIMx   : TIM1 - TIM17
  * @param  Channel��ʹ�õĶ�ʱ��ͨ������Χ1 - 4
  * @retval PWM_StatusTypeDef��pwm��ʼ
  * @note   ��ǰͨ��״̬��Ϊ׼��ʱ�����ش���
*/
PWM_StatusTypeDef Motor_PWM_Start(TIM_HandleTypeDef* TIMx,u32 Channel)
{
  if(TIM_CHANNEL_STATE_GET(TIMx, Channel) != HAL_TIM_CHANNEL_STATE_READY)
  {
      //HAL_TIM_PWM_Stop(TIMx,Channel);
      TIM_CHANNEL_STATE_SET(TIMx,Channel,HAL_TIM_CHANNEL_STATE_READY);
      if(HAL_TIM_PWM_Start(TIMx,Channel) != HAL_OK)
      {
        return PWM_ERROR;
      }
      else
        return PWM_BUSY;
  }
  else if(HAL_TIM_PWM_Start(TIMx,Channel) != HAL_OK)
  {
      return PWM_ERROR;
  }
  return PWM_START;
}
/************************************�����޸�**********************************/
/**
  * @brief  �޸Ķ�ʱ��Ƶ��
  * @param  TIMx   : TIM1 - TIM17
  * @param  Channel��ʹ�õĶ�ʱ��ͨ������Χ1 - 4
  * @param  _ulFreq: PWM�ź�Ƶ�ʣ���λHz (ʵ�ʲ��ԣ��������100MHz����0 ��ʾ��ֹ���
  * @retval PWM_StatusTypeDef��Ƶ�ʸ��»��߲�����
  * @note   https://blog.csdn.net/qq_35021496/article/details/106120181
            PWM_StatusTypeDef:��ʱ��״̬
            ��Ƶ���ޱ仯ʱ��������pwm����������仯ʱ�ٿ�ʼ����
            ����ռ�ձ����޸�

�޸�����Ƶ�ʹ��ߵ���ռ�ձ���Ƶ�������󣬲�����������궨��  --2022.07.20
*/
PWM_StatusTypeDef Modify_TIM_Freq(TIM_HandleTypeDef* TIMx,u32 Channel, uint32_t _ulFreq)
{
  uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t pulse;
	uint32_t uiTIMxCLK;
  uint32_t error = 0;
  static uint32_t last_freq = 0;
  
  if(_ulFreq == 0)
  {
    return Motor_PWM_Stop(TIMx,Channel);
  }
  
  error = _ulFreq - last_freq;
  last_freq = _ulFreq;

  if(error == 0)
  {
    return PWM_NO_UPADTE;    //Ƶ��һ�£������¡�
  }
  else
  {
    /* APB1 ��ʱ�� = 90M */
    uiTIMxCLK = SystemCoreClock / 2;
    //(uiTIMxCLK / (TIMx->Init.Prescaler + 1 / _ulFreq ��������
    usPeriod =  DIV_ROUND_CLOSEST((uiTIMxCLK / (TIMx->Init.Prescaler + 1)),_ulFreq)  - 1;		/* �Զ���װ��ֵ */
    
    __HAL_TIM_SET_AUTORELOAD(TIMx,usPeriod);
    
    TIM_CHANNEL_STATE_SET(TIMx,Channel,HAL_TIM_CHANNEL_STATE_READY);
                                                   //(usPeriod+1)/2 ��������
    __HAL_TIM_SET_COMPARE(TIMx,Channel,(uint16_t)(DIV_ROUND_CLOSEST((usPeriod+1),2)));
    return PWM_UPADTE;
  }
}
/**
  * @brief  �޸Ķ�ʱ��ռ�ձ�
  * @param  TIMx   : TIM1 - TIM17
  * @param  Channel��ʹ�õĶ�ʱ��ͨ������Χ1 - 4
  * @param  Duty: ռ�ձ� 0��100.��λ�ٷֱȡ�50����ٷ�֮50
  * @retval PWM_StatusTypeDef��Ƶ�ʸ��»��߲�����
  * @note   PWM_StatusTypeDef:��ʱ��״̬
            ��Ƶ���ޱ仯ʱ��������pwm����������仯ʱ�ٿ�ʼ����
            ����Ƶ�����޸�
*/
PWM_StatusTypeDef Modify_TIM_Duty(TIM_HandleTypeDef* TIMx,u32 Channel,u16 Duty)
{
  u16 error = 0;
  static u16 last_Duty = 0;
  
  if(Duty > 10000)
    Duty = 10000;
  else if(Duty < 0)
    Duty = 0;
  
  error = Duty - last_Duty;
  last_Duty = Duty;

  if(error == 0)
  {
    return PWM_NO_UPADTE;    //ռ�ձ�һ�£������¡�
  }
  else
  {
    __HAL_TIM_SetCompare(TIMx,Channel,((__HAL_TIM_GET_AUTORELOAD(TIMx) + 1) * Duty) /10000);//�޸ıȽ�ֵ���޸�ռ�ձ�
    return PWM_UPADTE;
  }
}