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
#ifndef __MOTOR_H
#define __MOTOR_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "stm32f4xx_it.h"
/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  ֹͣ����
  */  
typedef enum
{
  NO_STOP = 0x00U,//��ֹͣ            0x00
  HARD_STOP,      //Ӳ����ͣ          0x02
  SOFT_STOP     , //��ͣ            0x04
  Detection_STOP, //��غ�����ͣ      0x08
  ENABLE_STOP   , //����ر�ʹ�ܼ�ͣ  0x10
  ALM_STOP      , //������ͣ          0X20
  CRASH_STOP    , //��ײ����ͣ        0X40
  VBATT_STOP    , //��ѹ���ͼ�ͣ      0X80
  BEAT_STOP    ,  //��ѹ���ͼ�ͣ      0X100
}Stop_Code;
#define stop_type uint16_t
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
/** 
  * @brief  MOTOR ���� structures definition  
  */  
typedef enum 
{
  CW  = 0X00U,//˳ʱ��
  CCW = 0XFFU,//��ʱ��
} Directionstate;
/** 
  * @brief  MOTOR Status structures definition  
  */  
typedef enum 
{
  Motor_STOP     = 0x02U,
  Motor_BUSY     = 0x03U,
  Motor_BREAK    = 0x04U,//ɲ��
  Motor_RELEASE  = 0x05U,//�ɿ�ɲ��
} Motor_state;
/** 
  * @brief  �˶�״̬structures definition  
  */  
typedef enum 
{
  MOVE_DONE     = 0x01U,//�˶����
  MOVE_UNDONE   = 0x02U,//�˶�δ���
  MOVE_STOP     = 0X03U,//�˶�ֹͣ [����Ϊ��ͣ]
  MOVE_UNCHANGE = 0X04U,//�����������
} MOVE_state;
/** 
  * @brief  MOTOR ���Ų��� structures definition  
  */  
typedef struct
{
  PinTypeDef    Brake;             //ɲ������
  PinTypeDef    EN;                //ʹ�ܲ��� 
  PinTypeDef    Dir;               //�������
}Motor_IOTypeDef;
/** 
  * @brief  MOTOR ״̬���� structures definition  
  */  
typedef struct
{
  PWM_StatusTypeDef      PWM;    //PWM��ʱ��״̬
  Motor_state           MOTOR;  //���״̬
}Motor_StatusTypeDef;
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
#endif /* __MOTOR_H */
