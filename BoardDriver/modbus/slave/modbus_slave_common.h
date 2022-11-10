/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : 
  * @brief          : 
  * @date           :
  ******************************************************************************
  * @attention
  ��ʼ��ַ�ͽ�����ַ�����ļĴ������������ơ����ĺ����ڲ� map_buf �����С��ʹ����
  bit �Ĵ��� < 250
  register �Ĵ��� < 125

  �ӿں���Ϊ NULL���Ĵ�����Ӧ�Ĺ���������Ӧ��Ϊ�ɹ���

  get �ӿ�
  ����ַ���ڵ�����ȫ�������� buf �С�

  set �ӿ�
  index: ��ַ���ڵ�ƫ��
  len: ����
  ���� index �� len �޸����ݡ�
  * @author
  ******************************************************************************
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
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern const agile_modbus_slave_util_map_t bit_maps[1];
extern const agile_modbus_slave_util_map_t input_bit_maps[1];
extern const agile_modbus_slave_util_map_t register_maps[1];
extern const agile_modbus_slave_util_map_t input_register_maps[1];
/* Exported functions prototypes ---------------------------------------------*/
static int addr_check(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info)
{
    int slave = slave_info->sft->slave;
    if ((slave != ctx->slave) && (slave != AGILE_MODBUS_BROADCAST_ADDRESS) && (slave != 0xFF))
        return -AGILE_MODBUS_EXCEPTION_UNKNOW;

    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_SLAVE_COMMON_H */