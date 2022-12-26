/**
  ******************************************************************************
  * @file    motor.h
  * @brief   �������
  ******************************************************************************
  * @attention  �ײ������������� ������ʱ����ʼ��Ҫ��pwm�����ʼ��ǰ��
  * @author HLY
    �Ż�Walk_Motor�����ʼ������ 2022.04.18
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_PWM_H
#define __USER_PWM_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "stm32f4xx_it.h"
/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  MOTOR Status structures definition  
  */  
typedef enum 
{
  PWM_INIT     = 0X01U,//��ʼ��
  PWM_ERROR    = 0X02U,//����
  PWM_START    = 0X03U,//��ʼ���
  PWM_STOP     = 0X04U,//ֹͣ���
  PWM_UPADTE   = 0x05U,//Ƶ�ʸ���
  PWM_NO_UPADTE= 0x06U,//Ƶ���޸���
  PWM_BUSY     = 0x07U,//pwm�������
} PWM_StatusTypeDef;

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/************************************�������**********************************/
extern void              Motor_PWM_init(TIM_HandleTypeDef* TIMx);
extern PWM_StatusTypeDef Motor_PWM_Stop(TIM_HandleTypeDef* TIMx,u32 Channel);
extern PWM_StatusTypeDef Motor_PWM_Start(TIM_HandleTypeDef* TIMx,u32 Channel);
/************************************�����޸�**********************************/
extern PWM_StatusTypeDef Modify_TIM_Freq(TIM_HandleTypeDef* TIMx,u32 Channel, uint32_t _ulFreq);
extern PWM_StatusTypeDef Modify_TIM_Duty(TIM_HandleTypeDef* TIMx,u32 Channel,u16 Duty);
#endif /* __USER_PWM_H */
