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
  HARD_STOP     , //Ӳ����ͣ          0x02
  SOFT_STOP     , //��ͣ            0x04
  Detection_STOP, //�����쳣          0x08
  ENABLE_STOP   , //����ر�ʹ��      0x10
  ALM_STOP      , //��������          0X20
  CRASH_STOP    , //��ײ������        0X40
  VBATT_STOP    , //��ѹ���ͱ���      0X80
  BEAT_STOP    ,  //�����쳣          0X100
}Stop_Code;
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
 * @brief MOBDUS-ת�����ṹ��
 */
typedef struct
{
  uint16_t* angle_l;
  uint16_t* angle_h;
  uint16_t* speed;
  uint16_t* max_angle;
  uint16_t* min_angle;
}modbus_turn;
/**
 * @brief ת�����ṹ��
 * @note  over_range:true ������Χ
 */
typedef struct
{
  uint8_t         nodeID;         //����ڵ�ID
  uint8_t*        over_range;     //�Ƕȳ�����Χ��־
  uint16_t*       stop_state;     //��ͣ��־
  int16_t         max_angle;      //���Ƕ�
  int16_t         min_angle;      //��С�Ƕ�
  float           last;           //��һ�νǶ�
  float           err;            //�Ƕȸ������
  motor_config    cfg;            //�������
  modbus_turn     mb;             //modbus�ҹ�ָ��
}turn_motor_typeDef;
/**
 * @brief MOBDUS-���ߵ���ṹ��
 */
typedef struct
{
  int16_t* speed;
  uint16_t* max_speed;
}modbus_walk;
/**
 * @brief ���ߵ���ṹ��
 * @note  over_range:true ������Χ
 */
typedef struct
{
  uint8_t       nodeID;         //����ڵ�ID
  uint8_t*      over_range;     //�Ƕȳ�����Χ��־
  uint16_t*     stop_state;     //��ͣ��־
  int16_t       max_speed;      //����ٶ�
  int16_t       min_speed;      //��С�ٶ�
  float         last;           //��һ���ٶ�
  float         err;            //�ٶȸ������
  motor_config  cfg;            //�������
  modbus_walk   mb;             //modbus�ҹ�ָ��
}walk_motor_typeDef;
/* Exported constants --------------------------------------------------------*/
#define TURN_MOTOR_NUM 1//ת�����������
/* Exported macro ------------------------------------------------------------*/
//ת����
#define TURN_MOTOR0_SPEED_DEFAULT 60*10     //ת����[0]Ĭ���ٶ� ��λ0.1RPM
#define TURN_MOTOR0_MAX_ANGLE_DEFAULT 90    //ת����[0]Ĭ�����Ƕ�
#define TURN_MOTOR0_MIN_ANGLE_DEFAULT -90   //ת����[0]Ĭ����С�Ƕ�
//���ߵ��
#define WALK_MOTOR0_MAX_SPEED_DEFAULT 2000  //���ߵ��[0]Ĭ������ٶ� ��λ0.1RPM 
/* Exported variables ---------------------------------------------------------*/
extern turn_motor_typeDef turn_motor[TURN_MOTOR_NUM];
extern walk_motor_typeDef walk_motor[TURN_MOTOR_NUM];
/* Exported functions prototypes ---------------------------------------------*/
//ת����
extern uint8_t turn_motor_enable(turn_motor_typeDef* p);
extern uint8_t turn_motor_disable(turn_motor_typeDef* p);
extern uint8_t turn_motor_stop(turn_motor_typeDef* p);
extern uint8_t turn_motor_angle_control(float angle,float speed,turn_motor_typeDef* p);
extern void turn_motor_reentrant(turn_motor_typeDef* p);
extern float turn_motor_get_angle(turn_motor_typeDef* p);
//���ߵ��
extern uint8_t walk_motor_enable(walk_motor_typeDef* p);
extern uint8_t walk_motor_disable(walk_motor_typeDef* p);
extern uint8_t walk_motor_stop(walk_motor_typeDef* p);
extern uint8_t walk_motor_speed_control(float speed,walk_motor_typeDef* p);
extern void walk_motor_reentrant(walk_motor_typeDef* p);
extern float walk_motor_get_speed(walk_motor_typeDef* p);
#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H */