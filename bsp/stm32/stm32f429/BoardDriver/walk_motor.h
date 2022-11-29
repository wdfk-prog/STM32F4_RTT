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
#ifndef __WALK_MOTOR_H
#define __WALK_MOTOR_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "Trapezoidal.h"
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t               limit_mode;                    //mode:1����ǰ��ʻʱǰ�״ﴥ����2�������ʻʱ���״ﴥ����3������ɲ��
  stop_type             Stop_state;                    //��ͣ��־λ
  Motor_IOTypeDef       PIN;                           //���Ų���
  SENSORTypeDef         ALM;                           //��������
  Trapezoidal_TypeDef motor;
  Motor_StatusTypeDef state;
}WALK_MOTOR_TypeDef;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern WALK_MOTOR_TypeDef walk;
/* Exported functions prototypes ---------------------------------------------*/
extern void Walk_Motor_Init(void);
extern void Walk_Motor_Stop(void);
extern void Walk_Motor_Detection(void);
#endif /* __WALK_MOTOR_H */
