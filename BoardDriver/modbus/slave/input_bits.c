/**
 * @file input_bits.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022  
 * 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     first version
 */
/* Includes ------------------------------------------------------------------*/
#include "modbus_slave_common.h"
/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t _tab_input_bits[MODBUS_BIT_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
 * @brief Get the map buf object
 * @param  buf   Ŀ���ַ
 * @param  bufsz ��ȡ����
 * @retval int 
 * @note   None
 */
static int get_map_buf(void *buf, int bufsz)
{
    uint8_t *ptr = (uint8_t *)buf;

    modbus_mutex_lock();
    rt_memcpy(ptr,_tab_input_bits + MODBUS_START_ADDR,sizeof(_tab_input_bits));
    modbus_mutex_unlock();
    return 0;
}
/**
  * @brief  ��ȡMODBUS��ɢ������Ȧ�Ĵ�������
  * @param  index:��������
  * @param  index:����������
  * @retval ��ɢ������Ȧ�Ĵ�������
  * @note   None
*/
uint8_t modbus_input_bits_get(uint16_t index,uint16_t sub_index)
{
  return _tab_input_bits[sub_index];
}
/**
  * @brief  ��ɢ������Ȧ�Ĵ�������
  * @note   None
*/
const agile_modbus_slave_util_map_t input_bit_maps[INPUT_BIT_MAPS_NUM] = 
{
   //��ʼ��ַ           ������ַ                                   ��ȡ�ӿ�      ���ýӿ� 
    {0,       sizeof(_tab_input_bits) / sizeof(_tab_input_bits[0]), get_map_buf,    NULL}
};