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
#include "main.h"
#include "master402_canopen.h"
#include "filesystem.h"
#include "motor_control.h"
#include "lifter_motor.h"
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
void modbus_slave_input_register_default(void)
{
  //01D~10D�ڵ��������
  _tab_input_registers[1]  = MAX_NODE_COUNT - 1;    //�ڵ�����
  _tab_input_registers[2]  = 0X0F;                  //�ڵ�NMT״̬
  nodeID_get_name((char *)&_tab_input_registers[3], //03D~06D �ڵ�����
                   modbus_get_register(0,1));       //�ڵ�����
  _tab_input_registers[7]  = 0X00;                  //�ڵ�������
  _tab_input_registers[8]  = 0X00;                  //�ڵ�������
  _tab_input_registers[9]  = 0X00;                  //�ڵ�������
  _tab_input_registers[10] = 0X00;                  //�ڵ�������
  //11D~20D�����������
  _tab_input_registers[11] = 0X00;  //����ָ��
  _tab_input_registers[12] = 0X00;  //״̬λ
  _tab_input_registers[13] = 0X00;  //��ǰλ�õ�16λ
  _tab_input_registers[14] = 0X00;  //��ǰλ�ø�16λ
  _tab_input_registers[15] = 0X00;  //��ǰ�ٶȵ�16λ
  _tab_input_registers[16] = 0X00;  //��ǰ�ٶȸ�16λ
  //21D~40DоƬ��������
  rt_memcpy((char *)(_tab_input_registers+21),VERSION,sizeof(VERSION));              //��ӡ�汾��Ϣ 21D~27D��������
  _tab_input_registers[24] = ((uint32_t)(YEAR*10000+(MONTH + 1)*100+DAY) & 0xffff);  //���ڵ�16bti
	_tab_input_registers[25] = ((uint32_t)(YEAR*10000+(MONTH + 1)*100+DAY)>>16);		   //���ڸ�16bti
	_tab_input_registers[26] = ((uint32_t)(HOUR*10000+MINUTE*100+SEC)&0xffff);         //ʱ���16bti
	_tab_input_registers[27] = ((uint32_t)(HOUR*10000+MINUTE*100+SEC)>>16);		         //ʱ���16bti
  _tab_input_registers[28] =  HAL_GetUIDw0();
  _tab_input_registers[29] =  HAL_GetUIDw0() >> 16;
  _tab_input_registers[30] =  HAL_GetUIDw1();
  _tab_input_registers[31] =  HAL_GetUIDw1() >> 16;
  _tab_input_registers[32] =  HAL_GetUIDw2();
  _tab_input_registers[33] =  HAL_GetUIDw2() >> 16;
  _tab_input_registers[34] =  HAL_GetHalVersion();
  _tab_input_registers[35] =  HAL_GetHalVersion() >> 16;
  _tab_input_registers[38] =  boot_count_read();    //��λ����
  //41D~60Dת��������
}
/**
  * @brief  ����Ĵ�����ʼ��
  * @param  None
  * @retval int
  * @note   None
*/
void modbus_slave_input_register_init(void)
{
  //01D~10D�ڵ��������
  //11D~20D�����������
  //21D~40DоƬ��������
  //41D~60D ת��������
  turn_motor[0].stop_state = &_tab_input_registers[53]; //ת����[0]ֹͣ����
  //61D~70D ���ߵ������
  walk_motor[0].stop_state = &_tab_input_registers[65]; //���ߵ��[0]ֹͣ����
  //71D~80D �����������
  lifter_motor.stop_state  = &_tab_input_registers[71]; //�������ֹͣ����
}
/**
  * @brief  д�뱾������������Ĵ�����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_input_register_write(void)
{
  static int now_time = 0,last_time = 0;

  last_time = now_time;
  now_time = rt_tick_get_millisecond();

  uint8_t nodeID = modbus_get_register(0,1);
  //01D~10D�ڵ��������
  _tab_input_registers[2] = nodeID_get_nmt(nodeID);           //�ڵ�NMT״̬
  nodeID_get_name((char *)&_tab_input_registers[3],nodeID);   //�ڵ����� 03D~06D
  _tab_input_registers[7] = nodeID_get_errcode(nodeID);       //�ڵ�������
  nodeID_get_errSpec((char *)&_tab_input_registers[8],nodeID);//�ڵ������� 08D~10D
  //11D~20D�����������
  _tab_input_registers[11] = motor_get_controlword(nodeID);         //����ָ��
  _tab_input_registers[12] = motor_get_statusword(nodeID);          //״̬λ
  motor_get_position((INTEGER32 *)&_tab_input_registers[13],nodeID);//��ǰλ��
  motor_get_velocity((INTEGER32 *)&_tab_input_registers[15],nodeID);//��ǰ�ٶ� ��λ 0.1RPM
  //21D~40DоƬ��������
  _tab_input_registers[36] =  now_time;
  _tab_input_registers[37] =  now_time >> 16;                   //оƬ����ʱ��
  _tab_input_registers[39] =  (uint16_t)(now_time - last_time); //modbusͨ������ ��λms
  //41D~60D ת��������
  _tab_input_registers[41] =  turn_motor_get_angle(&turn_motor[0]) * 1000;        //ת����[0]�Ƕȷ���
  _tab_input_registers[42] =  (int32_t)(turn_motor_get_angle(&turn_motor[0]) * 1000) >> 16;
  motor_get_velocity((INTEGER32 *)&_tab_input_registers[49],turn_motor[0].nodeID);//ת����[0]��ǰ�ٶ� ��λ 0.1RPM
  //61D~70D ���ߵ������
  _tab_input_registers[61] =  walk_motor_get_speed(&walk_motor[0]) * 10;        //���ߵ��[0] ��λ:0.1RPM
}
/**
  * @brief  ��ȡMODBUS����Ĵ�������
  * @param  index:��������
  * @param  index:����������
  * @retval uint16_t
  * @note   None
*/
uint16_t modbus_get_input_register(uint16_t index,uint16_t sub_index)
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