/**
 * @file registers.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @attention 
 * @copyright Copyright (c) 2022
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     first version
 * 2022-11-23 1.1     HLY     ָ����ؼĴ�����ַ�����ٸ�ֵʱ��
 */
/* Includes ------------------------------------------------------------------*/
#include "modbus_slave_common.h"
/* Private includes ----------------------------------------------------------*/
#include "motor.h"
#include "lifter_motor.h"
#include "monitor.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t _tab_registers[MODBUS_REG_MAX_NUM];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  д������Ĵ���Ĭ��ֵ
  * @param  None
  * @retval int
  * @note   None
*/
void modbus_slave_register_default(void)
{
  //01D~10D �ڵ��������
  _tab_registers[1]   = 1;                            //�ڵ�ID
  //02D~10D CAN��������
  //11D~30D �����������
  _tab_registers[11]  = 0;                            //���ģʽ
  _tab_registers[12]  = 0;                            
  _tab_registers[13]  = 0;                            //ԭ��ƫ��ֵ ��λPUU
  _tab_registers[14]  = 34;                           //��ԭ��ʽ
  _tab_registers[15]  = 100;                          //Ѱ��ԭ�㿪���ٶ� ��λ0.1rpm
  _tab_registers[16]  = 20;                           //Ѱ�� Z�����ٶ�   ��λ0.1rpm
  //17D~21D �����������
  //31D~40D����ʱ���������
  //41D~60D ת��������
  _tab_registers[49] = TURN_MOTOR0_SPEED_DEFAULT;    //ת����[0]�ٶ����� ��λ0.1RPM
  _tab_registers[53] = TURN_MOTOR0_MAX_ANGLE_DEFAULT;//ת����[0]���Ƕ�
  _tab_registers[54] = TURN_MOTOR0_MIN_ANGLE_DEFAULT;//ת����[0]��С�Ƕ�
  //31D~40D ����ʱ���������
  //41D~60D ת��������
  //61D~70D ���ߵ������
  _tab_registers[65] = WALK_MOTOR0_MAX_SPEED_DEFAULT;//���ߵ��[0]����ٶ� ��λRPM
  //71D~80D �����������
  
}
/**
  * @brief  ���ּĴ�����ʼ��
  * @param  None
  * @retval int
  * @note   None
*/
void modbus_slave_register_init(void)
{
  //01D~10D �ڵ��������
  mb_can.nodeID       = &_tab_registers[1];   //�ڵ�ID
  //02D~10D CAN��������
  //11D~30D �����������
  mb_can.motor_mode   = &_tab_registers[11];  //���ģʽ
  mb_can.offset_l     = &_tab_registers[12];  //ԭ��ƫ��ֵ ��λPUU
  mb_can.offset_h     = &_tab_registers[12];  //ԭ��ƫ��ֵ ��λPUU
  mb_can.method       = &_tab_registers[14];  //��ԭ��ʽ
  mb_can.switch_speed = &_tab_registers[15];  //Ѱ��ԭ�㿪���ٶ� ��λrpm
  mb_can.zero_speed   = &_tab_registers[16];  //Ѱ�� Z�����ٶ�   ��λrpm
  //17D~21D �����������
  //31D~40D ����ʱ���������
  /* update date. */
  mb_tm.year        = &_tab_registers[31];
  mb_tm.mon         = &_tab_registers[32];
  mb_tm.mday        = &_tab_registers[33];     
  /* update time. */
  mb_tm.hour        = &_tab_registers[34];
  mb_tm.min         = &_tab_registers[35];
  mb_tm.sec         = &_tab_registers[36];

  debug_beat.value  = &_tab_registers[38];  //���Դ�������
  //41D~60D ת��������
  turn_motor[0].mb.angle_l   = &_tab_registers[41];
  turn_motor[0].mb.angle_h   = &_tab_registers[42];//ת����[0]�Ƕ����� ��λ:0.001��
  turn_motor[0].mb.speed     = &_tab_registers[49];//ת����[0]�ٶ����� ��λ:0.1RPM
  turn_motor[0].mb.max_angle = &_tab_registers[53];//ת����[0]���Ƕ�
  turn_motor[0].mb.min_angle = &_tab_registers[54];//ת����[0]��С�Ƕ�
  //61D~70D ���ߵ������
  walk_motor[0].mb.speed     = (int16_t*)&_tab_registers[61];//���ߵ��[0]�ٶ����� ��λ:0.1RPM
  walk_motor[0].mb.max_speed = &_tab_registers[65];//���ߵ��[0]����ٶ� ��λ:RPM
  //71D~80D �����������
  lifter_motor.target        = (int16_t*)&_tab_registers[71];//���������λ�߶� ��λ:mm

  lifter_motor.feedback      = (int16_t*)&_tab_registers[80];//���������λ�߶� ��λ:mm 
}
/**
  * @brief  ��ȡ���ּĴ���������������
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_register_read(void)
{
  //01D~10D �ڵ��������
  //11D~30D �����������
  //31D~40D ����ʱ���������
  //41D~60D ת��������
  //61D~70D ���ߵ������
}
/**********************************************************************************/
/**
  * @brief  ��ȡMODBUS���ּĴ�������
  * @param  index:��������
  * @param  index:����������
  * @retval uint16_t
  * @note   None
*/
uint16_t modbus_get_register(uint16_t index,uint16_t sub_index)
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
void modbus_set_register(uint16_t index,uint16_t sub_index,uint16_t data)
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
uint8_t modbus_reset_register(uint16_t start_index,uint16_t start_sub_index,uint16_t end_index,uint16_t end_sub_index)
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
 * @brief Get the map buf object
 * @param  buf              
 * @param  bufsz            
 * @retval int 
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
 * @brief Set the map buf object
 * @param  index            
 * @param  len              
 * @param  buf              
 * @param  bufsz            
 * @retval int 
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
 * @brief ���ּĴ���
 */
const agile_modbus_slave_util_map_t register_maps[REGISTER_MAPS_NUM] = 
{
   //��ʼ��ַ                     ������ַ                          ��ȡ�ӿ�   ���ýӿ� 
   {0,        sizeof(_tab_registers) / sizeof(_tab_registers[0]),    get_map_buf, set_map_buf},
};