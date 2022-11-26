/**
 * @file modbus_slave_common.h
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @copyright Copyright (c) 2022
 * @attention   ��ʼ��ַ�ͽ�����ַ�����ļĴ������������ơ����ĺ����ڲ� map_buf �����С��ʹ����
                bit �Ĵ��� < 250  register �Ĵ��� < 125
                �ӿں���Ϊ NULL���Ĵ�����Ӧ�Ĺ���������Ӧ��Ϊ�ɹ���
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     first version
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODBUS_SLAVE_COMMON_H
#define __MODBUS_SLAVE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <agile_modbus.h>
#include "agile_modbus_slave_util.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "rtthread.h"
/* Exported types ------------------------------------------------------------*/
/**
 * @brief ���ڵ��Խṹ��
 */
typedef struct
{
  uint8_t   read_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
  rt_size_t size;
  bool      flag; //��һ��������
}uart_debug;
/**
 * @brief MODBUS����CANOPEN�ṹ��
 */
typedef struct
{
  uint16_t* nodeID;
  uint16_t* motor_mode;
  uint16_t* offset_l;
  uint16_t* offset_h;
  uint16_t* method;
  uint16_t* switch_speed;
  uint16_t* zero_speed;
}canopen_debug;
/**
 * @brief MODBUSʱ����ṹ��
 */
typedef struct
{
  uint16_t* year;
  uint16_t* mon;
  uint16_t* mday;
  uint16_t* hour;
  uint16_t* min;
  uint16_t* sec;
}modbus_tm;

/* Exported constants --------------------------------------------------------*/
#define MODBUS_START_ADDR       1     
#define MODBUS_REG_MAX_NUM      125
#define MODBUS_BIT_MAX_NUM      250

#define BIT_MAPS_NUM            1
#define INPUT_BIT_MAPS_NUM      1
#define REGISTER_MAPS_NUM       2
#define INPUT_REGISTER_MAPS_NUM 2
/* Exported macro ------------------------------------------------------------*/
#define UART_DEBUG 0//��������Դ���
/*
 * ƴ��16λΪ32λ
 * H:16λ��λ
 * L:16λ��λ
*/
#define MAKEINT_32(H,L) (((int32_t)(H) << 16) | (uint16_t)(L))
/* Exported variables ---------------------------------------------------------*/
extern const agile_modbus_slave_util_map_t bit_maps[BIT_MAPS_NUM];
extern const agile_modbus_slave_util_map_t input_bit_maps[INPUT_BIT_MAPS_NUM];
extern const agile_modbus_slave_util_map_t register_maps[REGISTER_MAPS_NUM];
extern const agile_modbus_slave_util_map_t input_register_maps[INPUT_REGISTER_MAPS_NUM];
//�ҹ�ָ��
extern canopen_debug  mb_can;
extern modbus_tm      mb_tm;
/* Exported functions prototypes ---------------------------------------------*/
extern int addr_check(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info);
//������Դ
extern void modbus_mutex_lock(void);
extern void modbus_mutex_unlock(void);
//д����Դ
extern void modbus_slave_rw(void);
//���ּĴ���
extern uint16_t modbus_get_register(uint16_t index,uint16_t sub_index);
extern void modbus_set_register(uint16_t index,uint16_t sub_index,uint16_t data);
extern uint8_t modbus_reset_register(uint16_t start_index,uint16_t start_sub_index,uint16_t end_index,uint16_t end_sub_index);
//����Ĵ���
extern uint16_t modbus_get_input_register(uint16_t index,uint16_t sub_index);
//��Ȧ�Ĵ���
extern uint8_t modbus_get_bits(uint16_t index,uint16_t sub_index);
extern void modbus_set_bits(uint16_t index,uint16_t sub_index,uint16_t data);
//��ɢ������Ȧ�Ĵ���
extern uint8_t modbus_get_input_bits(uint16_t index,uint16_t sub_index);
/**************************��Ȧ��ֵ***********************************************/
//01D~10D ��������
#define MB_DEBUG_MOTOR_ENABLE_SET     modbus_set_bits(0,1,1) //���ʹ�ܿ�����һ
#define MB_DEBUG_MOTOR_ENABLE_RESET   modbus_set_bits(0,1,0) //���ʹ�ܿ�������
#define MB_DEBUG_MOTOR_DISABLE_SET    modbus_set_bits(0,2,1) //������ÿ�����һ
#define MB_DEBUG_MOTOR_DISABLE_RESET  modbus_set_bits(0,2,0) //������ÿ�������
//11D~20d �������
#define MB_TURN1_SET                  modbus_set_bits(0,11,1) //ת����[1]ʹ��
#define MB_TURN1_RESET                modbus_set_bits(0,11,0) //ת����[1]����
#define MB_WALK1_SET                  modbus_set_bits(0,15,1) //���ߵ��[1]ʹ��
#define MB_WALK1_RESET                modbus_set_bits(0,15,0) //���ߵ��[1]����

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_SLAVE_COMMON_H */