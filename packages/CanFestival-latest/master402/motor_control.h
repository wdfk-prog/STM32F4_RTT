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
#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "canfestival.h"
#include <stdbool.h>
/* Exported types ------------------------------------------------------------*/
/*
 * INTEGER8��ͼ�����ṹ��
*/
typedef struct
{
  INTEGER8 *map_val; //������ַ
  UNS16 index;   //��������
}Map_Val_INTEGER8;
/*
 * UNS16��ͼ�����ṹ��
*/
typedef struct
{
  UNS16 *map_val; //������ַ
  UNS16 index;   //��������
}Map_Val_UNS16;
/*
 * UNS32��ͼ�����ṹ��
*/
typedef struct
{
  UNS32 *map_val; //������ַ
  UNS16 index;   //��������
}Map_Val_UNS32;
/*
 * INTEGER32��ͼ�����ṹ��
*/
typedef struct
{
  INTEGER32 *map_val; //������ַ
  UNS16 index;   //��������
}Map_Val_INTEGER32;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern Map_Val_UNS16 Controlword_Node[];
extern Map_Val_INTEGER8 Modes_of_operation_Node[];
/* Exported functions prototypes ---------------------------------------------*/
extern UNS8 motor_on_profile_position(UNS8 nodeId);
extern UNS8 motor_on_interpolated_position(UNS8 nodeId);
extern UNS8 motor_on_homing_mode(int32_t offset,uint8_t method,float switch_speed,float zero_speed,UNS8 nodeId);
extern UNS8 motor_on_profile_velocity(UNS8 nodeId);
/******************************�˶�ģʽ��������******************************************************************/
extern UNS8 motor_profile_position(int32_t position,uint32_t speed,bool abs_rel,bool immediately,UNS8 nodeId);
extern UNS8 motor_interpolation_position (UNS8 nodeId);
extern UNS8 motor_homing_mode (bool zero_flag,UNS8 nodeId);
extern UNS8 motor_profile_velocity(uint32_t speed,UNS8 nodeId);
/******************************�˶��رռ���ѯ����******************************************************************/
extern UNS8 motor_off(UNS8 nodeId);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_CONTROL_H */
