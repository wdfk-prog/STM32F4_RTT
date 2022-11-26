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
#include "motor.h"
#include "monitor.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t _tab_input_bits[MODBUS_BIT_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д����ɢ����Ĵ���Ĭ��ֵ
  * @param  None
  * @retval int
  * @note   None
*/
void modbus_slave_input_bits_default(void)
{
  //01D~04D������������
  //06D~10Dת��������
}
/**
  * @brief  ��ɢ����Ĵ�����ʼ��
  * @param  None
  * @retval int
  * @note   None
*/
void modbus_slave_input_bits_init(void)
{
  //01D~04D������������
  debug_beat.flag = &_tab_input_bits[1];
  //06D~10Dת��������
  turn_motor[0].over_range = &_tab_input_bits[6]; //ת����[0]�Ƕȳ�����Χ��־
  //11D~15D���ߵ������
  walk_motor[0].over_range = &_tab_input_bits[11]; //ת����[0]�Ƕȳ�����Χ��־
}
/**
  * @brief  д�뱾����������ɢ����Ĵ�����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_input_bits_write(void)
{

}
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
uint8_t modbus_get_input_bits(uint16_t index,uint16_t sub_index)
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