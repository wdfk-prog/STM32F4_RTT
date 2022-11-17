/**
 * @file input_registers.c
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
#include "master402_canopen.h"
#include "motor_control.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t _tab_input_registers[MODBUS_REG_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д������Ĵ���Ĭ��ֵ
  * @param  None
  * @retval int
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
  //������Ϣ
  //21D~25D
  rt_memcpy((char *)(_tab_input_registers+21),VERSION,sizeof(VERSION));              //��ӡ�汾��Ϣ
  _tab_input_registers[24] = ((uint32_t)(YEAR*10000+(MONTH + 1)*100+DAY) & 0xffff);  //���ڵ�16bti
	_tab_input_registers[25] = ((uint32_t)(YEAR*10000+(MONTH + 1)*100+DAY)>>16);		   //���ڸ�16bti
	_tab_input_registers[26] = ((uint32_t)(HOUR*10000+MINUTE*100+SEC)&0xffff);         //ʱ���16bti
	_tab_input_registers[27] = ((uint32_t)(HOUR*10000+MINUTE*100+SEC)>>16);		         //ʱ���16bti
  //ID��������
  _tab_input_registers[28] =  HAL_GetUIDw0();
  _tab_input_registers[29] =  HAL_GetUIDw0() >> 16;
  _tab_input_registers[30] =  HAL_GetUIDw1();
  _tab_input_registers[31] =  HAL_GetUIDw1() >> 16;
  _tab_input_registers[32] =  HAL_GetUIDw2();
  _tab_input_registers[33] =  HAL_GetUIDw2() >> 16;
  _tab_input_registers[34] =  HAL_GetHalVersion();
  _tab_input_registers[35] =  HAL_GetHalVersion() >> 16;
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
  //оƬ����ʱ��
  _tab_input_registers[36] =  rt_tick_get_millisecond();
  _tab_input_registers[37] =  rt_tick_get_millisecond() >> 16;
}
/**
  * @brief  ��ȡMODBUS����Ĵ�������
  * @param  index:��������
  * @param  index:����������
  * @retval uint16_t
  * @note   None
*/
uint16_t modbus_input_register_get(uint16_t index,uint16_t sub_index)
{
  return _tab_input_registers[sub_index];
}
/**
 * @brief Get the map buf object
 * @param  buf              ��ַ
 * @param  bufsz            ����
 * @retval int 
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
 * @brief ����Ĵ���
 */
const agile_modbus_slave_util_map_t input_register_maps[INPUT_REGISTER_MAPS_NUM] = 
{
  //��ʼ��ַ                          ������ַ                                  ��ȡ�ӿ�      ���ýӿ� 
  {0,         sizeof(_tab_input_registers) / sizeof(_tab_input_registers[0]),    get_map_buf,    NULL},
};