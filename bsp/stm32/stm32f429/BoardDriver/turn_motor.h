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
#ifndef __TURN_MOTOR_H
#define __TURN_MOTOR_H
/* Includes ------------------------------------------------------------------*/
#include "Smove.h"
#include "tim.h"
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  MOTOR���� structures definition  
  */  
typedef struct
{
  u32                   set_freq;                      //����Ķ�ʱ��Ƶ��
  int32_t               get_freq;                      //�����Ƶ��
  float                 set_radian;                    //���û���
  float                 get_radian;                    //��������
  uint8_t               limit_mode;                    //��λģʽ mode:1���ж���λ��������2�����㴫������3��һ���ж�.4��������λ
  stop_type             Stop_state;                    //��ͣ��־λ
  u8                    jog_flag;                      //�㶯��־
  
  Motor_IOTypeDef       PIN;                           //���Ų���
  SENSORTypeDef         ALM;                           //��������
  SMove_TypeDef         motor;
  Motor_StatusTypeDef   state;
}TURN_MOTOR_TypeDef;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern TURN_MOTOR_TypeDef turn;
/* Exported functions prototypes ---------------------------------------------*/
extern void Turn_Motor_Init(void);
extern void Turn_Motor_Stop(void);
extern void Turn_SMove_DoReset(void *p);
extern void Turn_Motor_Detection(void);
#endif /* __TURN_MOTOR_H */
