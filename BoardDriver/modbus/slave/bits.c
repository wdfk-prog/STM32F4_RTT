/**
 * @file bits.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @attention 
 * @copyright Copyright (c) 2022
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
static uint8_t _tab_bits[MODBUS_BIT_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д����Ȧ�Ĵ���Ĭ��ֵ
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_bits_default(void)
{
  _tab_bits[1] = 0;  //�������ʹ��
  _tab_bits[2] = 1;  //������ƽ���
  //���Ա�������
}
/**
  * @brief  ��Ȧ�Ĵ�����ʼ��
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_bits_init(void)
{

}
/**
  * @brief  ��ȡ��Ȧ�Ĵ���������������
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_bits_read(void)
{

}
/**
 * @brief Get the map buf object
 * @param  buf   �ӿڵ�ַ      
 * @param  bufsz ����
 * @retval int 
 * @note   None
 */
static int get_map_buf(void *buf, int bufsz)
{
    uint8_t *ptr = (uint8_t *)buf;

    modbus_mutex_lock();
    rt_memcpy(ptr,_tab_bits + MODBUS_START_ADDR,sizeof(_tab_bits));
    modbus_mutex_unlock();
    return 0;
}
/**
 * @brief Set the map buf object
 * @param  index            ����
 * @param  len              ����
 * @param  buf              Ŀ���ַ
 * @param  bufsz            ���ó���
 * @retval int 
 */
static int set_map_buf(int index, int len, void *buf, int bufsz)
{
    uint8_t *ptr = (uint8_t *)buf;

    modbus_mutex_lock();
    rt_memcpy(_tab_bits + MODBUS_START_ADDR + index,ptr + index,len * sizeof(uint8_t));
    modbus_mutex_unlock();
    return 0;
}
/**
  * @brief  ��ȡMODBUS��Ȧ�Ĵ�������
  * @param  index:��������
  * @param  index:����������
  * @retval MODBUS��Ȧ�Ĵ�������
  * @note   None
*/
uint8_t modbus_get_bits(uint16_t index,uint16_t sub_index)
{
  return _tab_bits[sub_index];
}
/**
  * @brief  ����MODBUS��Ȧ�Ĵ�������
  * @param  index:��������
  * @param  index:����������
  * @param  data:��ֵ����
  * @retval None
  * @note   None
*/
void modbus_set_bits(uint16_t index,uint16_t sub_index,uint16_t data)
{
  _tab_bits[sub_index] = data;
}
/**
  * @brief  ��Ȧ�Ĵ�������
  * @note   ���鷶Χ��BIT_MAPS_NUM
*/
const agile_modbus_slave_util_map_t bit_maps[BIT_MAPS_NUM] = 
{
   //��ʼ��ַ               ������ַ                      ��ȡ�ӿ�   ���ýӿ� 
   {0,        sizeof(_tab_bits) / sizeof(_tab_bits[0]), get_map_buf, set_map_buf}
};