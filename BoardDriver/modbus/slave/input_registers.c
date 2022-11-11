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
/* Private typedef -----------------------------------------------------------*/
enum input_registers_name
{
  node_num = 0X01,
  nmt_state,
  node_name,
};
/* Private define ------------------------------------------------------------*/
#define INPUT_REG_START 0x00
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t _tab_input_registers[125];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д������Ĵ���Ĭ��ֵ
  * @param  None
  * @retval None
  * @note   None
*/
int modbus_slave_input_register_default(void)
{
  _tab_input_registers[node_num]  = MAX_NODE_COUNT - 1; //�ڵ�����
  _tab_input_registers[nmt_state] = 0X0F;               //�ڵ�NMT״̬
  nodeID_get_name((char *)&_tab_input_registers[node_name],modbus_register_get(0,1));//�ڵ�����
  return RT_EOK;
}
INIT_DEVICE_EXPORT(modbus_slave_input_register_default);
/**
  * @brief  д�뱾������������Ĵ�����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_input_register_write(void)
{
    uint8_t nodeID = modbus_register_get(0,1);
    _tab_input_registers[nmt_state] = nodeID_get_nmt(nodeID);
    nodeID_get_name((char *)&_tab_input_registers[node_name],nodeID);//�ڵ�����
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
    //ʹ��memcpy�����鸳ֵ��15us����
    rt_memcpy(ptr,_tab_input_registers + MODBUS_START_ADDR,sizeof(_tab_input_registers));
    return 0;
}
/**
  * @brief  
  * @param  None
  * @retval None
  * @note   None
*/
const agile_modbus_slave_util_map_t input_register_maps[1] = 
{
   //��ʼ��ַ           ������ַ                                                                        ��ȡ�ӿ�      ���ýӿ� 
    {INPUT_REG_START, INPUT_REG_START + sizeof(_tab_input_registers) / sizeof(_tab_input_registers[0]), get_map_buf,    NULL}
};