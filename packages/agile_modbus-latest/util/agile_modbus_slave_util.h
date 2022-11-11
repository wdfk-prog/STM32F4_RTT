/**
 * @file    agile_modbus_slave_util.h
 * @brief   Agile Modbus ������ṩ�ļ��״ӻ�����ͷ�ļ�
 * @author  ����ΰ (2544047213@qq.com)
 * @date    2022-07-28
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#ifndef __PKG_AGILE_MODBUS_SLAVE_UTIL_H
#define __PKG_AGILE_MODBUS_SLAVE_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @addtogroup UTIL
 * @{
 */

/** @addtogroup SLAVE_UTIL
 * @{
 */

/** @defgroup SLAVE_UTIL_Exported_Types Slave Util Exported Types
 * @{
 */

/**
 * @brief   �ӻ��Ĵ���ӳ��ṹ��
 */
typedef struct agile_modbus_slave_util_map {
    int start_addr;                                       /**< ��ʼ��ַ */
    int end_addr;                                         /**< ������ַ */
    int (*get)(void *buf, int bufsz);                     /**< ��ȡ�Ĵ������ݽӿ� */
    int (*set)(int index, int len, void *buf, int bufsz); /**< ���üĴ������ݽӿ� */
} agile_modbus_slave_util_map_t;

/**
 * @brief   �ӻ����ܽṹ��
 */
typedef struct agile_modbus_slave_util {
    const agile_modbus_slave_util_map_t *tab_bits;                                            /**< ��Ȧ�Ĵ����������� */
    int nb_bits;                                                                              /**< ��Ȧ�Ĵ�������������Ŀ */
    const agile_modbus_slave_util_map_t *tab_input_bits;                                      /**< ��ɢ������Ĵ����������� */
    int nb_input_bits;                                                                        /**< ��ɢ������Ĵ�������������Ŀ */
    const agile_modbus_slave_util_map_t *tab_registers;                                       /**< ���ּĴ����������� */
    int nb_registers;                                                                         /**< ���ּĴ�������������Ŀ */
    const agile_modbus_slave_util_map_t *tab_input_registers;                                 /**< ����Ĵ����������� */
    int nb_input_registers;                                                                   /**< ����Ĵ�������������Ŀ */
    int (*addr_check)(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info);       /**< ��ַ���ӿ� */
    int (*special_function)(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info); /**< ���⹦���봦��ӿ� */
    int (*done)(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info, int ret);    /**< ��������ӿ� */
} agile_modbus_slave_util_t;

/**
 * @}
 */

/** @addtogroup SLAVE_UTIL_Exported_Functions
 * @{
 */
int agile_modbus_slave_util_callback(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info, const void *data);
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __PKG_AGILE_MODBUS_SLAVE_UTIL_H */
