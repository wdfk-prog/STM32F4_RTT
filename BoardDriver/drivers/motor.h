/**
 * @file motor.h
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-21
 * @copyright Copyright (c) 2022
 * @attention 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-21 1.0     HLY     first version
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "user_math.h"

/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  ֹͣ����
  */  
typedef enum
{
  NO_STOP = 0x00U,//��ֹͣ            0x00
  HARD_STOP,      //Ӳ����ͣ          0x02
  SOFT_STOP     , //��ͣ            0x04
  Detection_STOP, //�����쳣          0x08
  ENABLE_STOP   , //����ر�ʹ��      0x10
  ALM_STOP      , //��������          0X20
  CRASH_STOP    , //��ײ������        0X40
  VBATT_STOP    , //��ѹ���ͱ���      0X80
  BEAT_STOP    ,  //�����쳣          0X100
}Stop_Code;

#define stop_type uint16_t

/**
 * @brief ������ýṹ��
 * @note  numerator:  �˶�һȦ����������
 *        denominator:����������ֱ���
 */
typedef struct
{
  uint32_t numerator;   //���ӳ��ֱȷ��� 
  uint32_t denominator; //���ӳ��ֱȷ�ĸ
}motor_config;
/**
 * @brief ת�����ṹ��
 * @note  over_range:true ������Χ
 */
typedef struct
{
  uint8_t       nodeID;         //����ڵ�ID
  uint8_t*      over_range;     //�Ƕȳ�����Χ��־
  stop_type     stop_state;     //��ͣ��־
  int16_t       max_angle;      //���Ƕ�
  int16_t       min_angle;      //��С�Ƕ�
  float         last;           //��һ�νǶ�
  float         err;            //�Ƕȸ������
  motor_config  cfg;            //�������
}turn_motor_typeDef;
/* Exported constants --------------------------------------------------------*/
#define TURN_MOTOR_NUM 1//ת�����������
/* Exported macro ------------------------------------------------------------*/
#define TURN_MOTOR_SPEED_DEFAULT 60*10    //0.1RPM
#define TURN_MOTOR_MAX_ANGLE_DEFAULT 90 
#define TURN_MOTOR_MIN_ANGLE_DEFAULT -90
/* Exported variables ---------------------------------------------------------*/
extern turn_motor_typeDef turn_motor[TURN_MOTOR_NUM];
/* Exported functions prototypes ---------------------------------------------*/
extern uint8_t turn_motor_enable(turn_motor_typeDef* p);
extern uint8_t turn_motor_disable(turn_motor_typeDef* p);
extern uint8_t turn_motor_stop(turn_motor_typeDef* p);
extern uint8_t turn_motor_angle_control(float angle,float speed,turn_motor_typeDef* p);
extern float turn_motor_get_angle(turn_motor_typeDef* p);
extern bool turn_motor_get_over_range(turn_motor_typeDef* p);
extern void turn_motor_set_angle_range(int16_t max,int16_t min,turn_motor_typeDef* p);
#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H */