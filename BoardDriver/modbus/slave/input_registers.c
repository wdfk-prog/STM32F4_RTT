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
#include "motor_control.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t _tab_input_registers[MODBUS_REG_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д������Ĵ���Ĭ��ֵ
  * @param  None
  * @retval None
  * @note   None
*/
int modbus_slave_input_register_default(void)
{
  //�ڵ��������
  _tab_input_registers[1]  = MAX_NODE_COUNT - 1;  //�ڵ�����
  _tab_input_registers[2]  = 0X0F;                //�ڵ�NMT״̬
  //��03D~06D
  nodeID_get_name((char *)&_tab_input_registers[3],
                   modbus_register_get(0,1));     //�ڵ�����
  _tab_input_registers[7]  = 0X00;                //�ڵ�������
  //08D~10D�ڵ�������
  _tab_input_registers[8]  = 0X00;                 //�ڵ�������
  _tab_input_registers[9]  = 0X00;                 //�ڵ�������
  _tab_input_registers[10] = 0X00;                 //�ڵ�������
  //�����������
  _tab_input_registers[11] = 0X00;                 //����ָ��
  _tab_input_registers[12] = 0X00;                 //״̬λ
  _tab_input_registers[13] = 0X00;                 //��ǰλ�õ�16λ
  _tab_input_registers[14] = 0X00;                 //��ǰλ�ø�16λ
  _tab_input_registers[15] = 0X00;                 //��ǰ�ٶȵ�16λ
  _tab_input_registers[16] = 0X00;                 //��ǰ�ٶȸ�16λ
  //17D~20D�����������
  return RT_EOK;
}

/**
  * @brief  д�뱾������������Ĵ�����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_input_register_write(void)
{
  uint8_t nodeID = modbus_register_get(0,1);        
  //�ڵ��������
  _tab_input_registers[2] = nodeID_get_nmt(nodeID);         //�ڵ�NMT״̬
   //��03D~06D
  nodeID_get_name((char *)&_tab_input_registers[3],nodeID); //�ڵ�����
  _tab_input_registers[7] = nodeID_get_errcode(nodeID);     //�ڵ�������
  //08D~10D�ڵ�������
  nodeID_get_errSpec((char *)&_tab_input_registers[8],nodeID);
  //�����������
  _tab_input_registers[11] = motor_get_controlword(nodeID);         //����ָ��
  _tab_input_registers[12] = motor_get_statusword(nodeID);          //״̬λ
  motor_get_position((INTEGER32 *)&_tab_input_registers[13],nodeID);//��ǰλ��
  motor_get_velocity((INTEGER32 *)&_tab_input_registers[15],nodeID);//��ǰ�ٶ�
}
/**
  * @brief  ��ȡMODBUS����Ĵ�������
  * @param  index:��������
  * @param  index:����������
  * @retval None
  * @note   None
*/
uint16_t modbus_input_register_get(uint16_t index,uint16_t sub_index)
{
  return _tab_input_registers[sub_index];
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
  //ʹ��memcpy�����鸳ֵ��15us����
  rt_memcpy(ptr,_tab_input_registers + MODBUS_START_ADDR,sizeof(_tab_input_registers));
  modbus_mutex_unlock();
  return 0;
}
/**
  * @brief  
  * @param  None
  * @retval None
  * @note   None
*/
const agile_modbus_slave_util_map_t input_register_maps[INPUT_REGISTER_MAPS_NUM] = 
{
  //��ʼ��ַ                          ������ַ                                  ��ȡ�ӿ�      ���ýӿ� 
  {0,         sizeof(_tab_input_registers) / sizeof(_tab_input_registers[0]),    get_map_buf,    NULL},
};