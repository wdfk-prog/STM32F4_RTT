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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "modbus_slave_common.h"
/* Private includes ----------------------------------------------------------*/
#include "master402_canopen.h"
#include "stm32f4xx_hal.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t _tab_registers[MODBUS_REG_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д������Ĵ���Ĭ��ֵ
  * @param  None
  * @retval None
  * @note   None
*/
int modbus_slave_register_default(void)
{
  //�ڵ��������
  _tab_registers[1] = 1;  //����������Ľڵ�ID
  //02D~10D CAN��������
  //�����������
  _tab_registers[11] = 0; //���õ��ģʽ
  _tab_registers[12] = 0; //ԭ��ƫ��ֵ��λ
  _tab_registers[13] = 0; //ԭ��ƫ��ֵ��λ
  _tab_registers[14] = 0; //��ԭ��ʽ
  _tab_registers[15] = 0; //Ѱ��ԭ�㿪���ٶ�
  _tab_registers[16] = 0; //Ѱ�� Z�����ٶ�
  _tab_registers[17] = 0; //����˶�����1
  _tab_registers[18] = 0; //����˶�����2
  _tab_registers[19] = 0; //����˶�����3
  _tab_registers[20] = 0; //����˶�����4
  _tab_registers[21] = 0; //����˶�����5
  //22D~30D �����������
  return RT_EOK;
}
/**
  * @brief  д�뱾�����������ּĴ�����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_register_write(void)
{

}
/**
  * @brief  ��ȡMODBUS���ּĴ�������
  * @param  index:��������
  * @param  index:����������
  * @retval None
  * @note   None
*/
uint16_t modbus_register_get(uint16_t index,uint16_t sub_index)
{
  return _tab_registers[sub_index];
}
/**
  * @brief  ����MODBUS���ּĴ�������
  * @param  index:��������
  * @param  index:����������
  * @param  data:��ֵ����
  * @retval None
  * @note   None
*/
void modbus_register_set(uint16_t index,uint16_t sub_index,uint16_t data)
{
  _tab_registers[sub_index] = data;
}
/**
  * @brief  ����MODBUS���ּĴ�����������
  * @param  start_index:��ʼ��������
  * @param  start_sub_index:��ʼ����������
  * @param  end_index:������������
  * @param  end_sub_index:��������������
  * @retval �ɹ�����0��ʧ�ܷ���0XFF
  * @note   None
*/
uint8_t modbus_register_reset(uint16_t start_index,uint16_t start_sub_index,uint16_t end_index,uint16_t end_sub_index)
{
  uint16_t *ptr = (uint16_t *)_tab_registers + start_sub_index;
  int16_t len = end_sub_index - start_sub_index;
  if(len <= 0)
    return 0XFF;
  else
  {
    rt_memset(ptr,0,len);
    return 0x00;
  }
}
/**
  * @brief  
  * @param  None
  * @retval None
  * @note   None
*/
static int get_map_buf(void *buf, int bufsz)
{
    uint16_t *ptr = (uint16_t *)buf;

    modbus_mutex_lock();
    rt_memcpy(ptr,_tab_registers + MODBUS_START_ADDR,sizeof(_tab_registers));
    modbus_mutex_unlock();
    return 0;
}
/**
  * @brief  
  * @param  None
  * @retval None
  * @note   None
*/
static int set_map_buf(int index, int len, void *buf, int bufsz)
{
    uint16_t *ptr = (uint16_t *)buf;

    modbus_mutex_lock();
    rt_memcpy(_tab_registers + MODBUS_START_ADDR + index,ptr + index,len * sizeof(uint16_t));
    modbus_mutex_unlock();
    return 0;
}
/**
  * @brief  
  * @param  None
  * @retval None
  * @note   None
*/
const agile_modbus_slave_util_map_t register_maps[REGISTER_MAPS_NUM] = 
{
   //��ʼ��ַ                     ������ַ                          ��ȡ�ӿ�   ���ýӿ� 
   {0,        sizeof(_tab_registers) / sizeof(_tab_registers[0]),    get_map_buf, set_map_buf},
};