/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : 
  * @brief          : 
  * @date           :
  ******************************************************************************
  * @attention
  * @author
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIFT_H
#define __LIFT_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "user_math.h"
#include "Smove.h"
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  �̵��� ���� structures definition  
  */  
typedef struct
{
  FunctionalState     flag;              //��ɱ�־
  uint16_t            RTFC;              //���赼ͨʱ��
  uint16_t            count;             //��ǰ��ͨʱ��
  
  uint16_t            min_RTFC;          //��С��ͨʱ��
//  float               min_Length;        //��С���г���
  
  PinTypeDef          IO;                //�̵���IO����
  position_PID        PID;               //PID����
}Relay_TypeDef;
/** 
  * @brief  �������״̬ structures definition  
  */  
typedef enum
{
  LIFT_STOP,
  LIFT_JACK,
  LIFT_FALL,
}Lifting_State;
/** 
  * @brief  Lifter_Motor ���� structures definition  
  */  
typedef struct
{
  float               incoming;          //����Ŀ�궨λ����
  int16_t             target;            //Ŀ�궨λ������
  int16_t             now;               //��ǰ��λֵ
  int16_t             error;             //��λ��ֵ
  uint16_t            period;            //PID��������
  int16_t             max_lenth;         //���λ����
  int16_t             min_lenth;         //��С��λ����
  
  int16_t             max_position;         //���λλ��
  int16_t             min_position;         //��С��λλ��

  SENSORTypeDef       Lower_Limit;       //����λ������
  SENSORTypeDef       SQP;               //�ӽ�����
  SENSORTypeDef       Reflection;        //�����䴫����
  
  ZERO_state          encoder_flag;      //�������Ƿ�ֹ״̬
  ZERO_state          do_reset_flag;     //�����Ƿ����״̬
  
  Relay_TypeDef       Jack;              //�����̵���
  Relay_TypeDef       Fall;              //�½��̵���
  Lifting_State       state;             //����״̬
  Lifting_State       last_state;        //��һ������״̬
  
  uint8_t             limit_mode;//��λģʽ mode:1���ж���λ��������2�����㴫������3��һ���ж�.4��������λ
}Lifter_Motor_TypeDef;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern Lifter_Motor_TypeDef  lift;
/* Exported functions prototypes ---------------------------------------------*/
extern void Lifter_Motor_Stop(Lifter_Motor_TypeDef *p);
extern void Lifter_Motor_Jack(Lifter_Motor_TypeDef *p);
extern void Lifter_Motor_Fall(Lifter_Motor_TypeDef *p);
extern void Lifter_Motor_Set_Target(Lifter_Motor_TypeDef *p,float tar);
#endif /* __LIFT_H */
