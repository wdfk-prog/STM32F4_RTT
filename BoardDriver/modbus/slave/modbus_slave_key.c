/**
 * @file modbus_slave_key.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @attention 
 * @copyright Copyright (c) 2022
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     mb��Ȧ����Ϊ����
 * 2022-11-23 1.1     HLY     �ӻ���������������Դ��ͨѶЧ��û�����Ա仯
 */
/* includes ------------------------------------------------------------------*/

/* private includes ----------------------------------------------------------*/
#include "modbus_slave_common.h"
#include "master402_canopen.h"
#include "motor_control.h"
#include "motor.h"
/*ulog include*/
#define log_tag              "mb key"
#define log_lvl              dbg_info
#include <ulog.h>
/* private typedef -----------------------------------------------------------*/
/** 
  * @brief  ����ע���
  */
typedef enum
{
	KEY_MOTOR_ENABLE,                //���ʹ��
  KEY_MOTOR_DISABLE,               //���ʹ��
  KEY_TURN_CONTROL,                //ת��������
  KEY_WALK_CONTROL,                //���ߵ������
  MBKEY_NUM,// ����Ҫ�еļ�¼��ť���������������
}mbkey_list;
/** 
  * @brief  ����״̬��������״̬
  */ 
typedef enum
{
  MBKEY_ENABLE,   // ʹ�ܣ���ֵΪ1
	MBKEY_DISABLE,  // ʧ�ܣ���ֵΪ0
	MBKEY_PRESS,    // ��������
	MBKEY_RAISE,    // ����̧��
}mbkey_status;
/** 
  * @brief  ��Ȧ�Ĵ���״̬
  */  
typedef enum 
{
  LOW_LEVEL = 0u, 
  HIGH_LEVEL = !LOW_LEVEL
} bits_status;
/** 
  * @brief  �������α�־
  */  
typedef enum 
{
  MB_DISABLE = 0u, 
  MB_ENABLE = !MB_DISABLE
} mbkey_enable_status;
/** @defgroup gpio_pull_define gpio pull define
 * @brief gpio pull-up or pull-down activation
 * @{
 */  
typedef enum
{
  PULLUP       = 0x00000001u,   //��һ�����¼�
  PULLDOWN     = 0x00000002u,   //��������¼�
}bits_activation;
/** 
  * @brief  ״̬����ʼ������
  */  
typedef struct
{
	uint32_t gpio_pull;		//�����ƽ
	uint16_t index;	      //����
	uint16_t sub_index;	  //������
	uint8_t  key_nox;
}config;
/** 
  * @brief  ״̬����
  */
typedef struct
{
    mbkey_enable_status 	shield; 		//�������Σ�disable(0):���Σ�enable(1):������
    uint8_t             	timecount;  //������������
    bits_status 	        flag;       //��־�������±�־
    bits_status 	        down_level; //����ʱ������ioʵ�ʵĵ�ƽ
    mbkey_status          key_status; //����״̬
    mbkey_status          key_event;  //�����¼�
    bits_status           (*read_pin)(config bits);//��io��ƽ����
}components;
/** 
  * @brief  ������
  */
typedef struct
{
	config 				      board; // �̳г�ʼ������
	components 	        status; // �̳�״̬������
}mbkey;
/* private define ------------------------------------------------------------*/
/* �߳����� */
#define THREAD_PRIORITY      6//�߳����ȼ�
#define THREAD_TIMESLICE     10//�߳�ʱ��Ƭ
#define THREAD_STACK_SIZE    1024//ջ��С
/* private macro -------------------------------------------------------------*/
/**
  * @brief  ���ּĴ���д����ֵ
  * @param  index:����
  * @param  sub_index:������
  * @param  value:ֵ
  * @param  factor:�Ŵ�ϵ��
  * @retval none.
  * @note   ��ǰ���ּĴ���Ϊ0��д��value�����򽫱��ּĴ���ֵд��value��
*/
#define REG_WRITE_VALUE(index,sub_index,value,factor)                 \
if(modbus_get_register((index),(sub_index)) == 0){                    \
modbus_set_register((index),(sub_index),(value) * (factor));}         \
else{(value) = modbus_get_register((index),(sub_index)) / (factor);}  \
/* private variables ---------------------------------------------------------*/
static mbkey mbkey_buf[MBKEY_NUM];	// ������������
/* private function prototypes -----------------------------------------------*/
/** 
  * @brief  ���ʹ�ܿ���
  * @param  event      
  * @note   д��Trueʹ�ܵ������,���ȼ�С�ڽ��õ����
  * @attention 
ע����һ��,�Ե��ģʽ����ǰ����ע�����˶������Ƿ��޸�Ϊ����ֵ��
����˶��������������ģʽ�����ã������н�������ƴ�Ӳ�����
*/ 
static void key_motor_enable(mbkey_status *event)
{
  if(*mb_can.nodeID == MASTER_NODEID || *mb_can.nodeID > MAX_NODE_COUNT || *mb_can.nodeID == 0)
  {
    MB_DEBUG_MOTOR_ENABLE_RESET;
    return;
  }
  switch(*event)
  {
  case MBKEY_ENABLE:  //���´����¼�
  {
    switch(*mb_can.motor_mode)
    {
    case PROFILE_POSITION_MODE://λ�ù滮ģʽ
    {
      int16_t speed     = 60;
      int32_t position  = MAKEINT_32(modbus_get_register(0,18),
                                      modbus_get_register(0,17));
      
      REG_WRITE_VALUE(0,19,speed,1);
      bool abs_rel      = modbus_get_register(0,20);
      bool immediately  = modbus_get_register(0,21);

      if(motor_profile_position(position,speed,abs_rel,immediately,*mb_can.nodeID) == 0XFF)
      {
        motor_on_profile_position(*mb_can.nodeID);
      }
      else
      {
        MB_DEBUG_MOTOR_ENABLE_RESET;
      }
    }
    break;
    case PROFILE_VELOCITY_MODE://�ٶȹ滮ģʽ
    {
      int16_t speed = modbus_get_register(0,17);
      
      if(motor_profile_velocity(speed,*mb_can.nodeID) == 0xFF)
      {
        motor_on_profile_velocity(*mb_can.nodeID);
      }
    }
    break;
    case PROFILE_TORQUE_MODE://Ť�ع滮ģʽ
    break;
    case HOMING_MODE://ԭ�㸴��ģʽ 
    {
      int16_t speed = 60;
      bool zero_flag = modbus_get_register(0,17);
      REG_WRITE_VALUE(0,18,speed,1);

      if(motor_homing_mode(zero_flag,speed,*mb_can.nodeID) >= 0XFD)//��һ�����û�����Ҫ����δ����ƫ��ֵ��������
      {
        int32_t offset = MAKEINT_32(*mb_can.offset_h,*mb_can.offset_l);
        motor_on_homing_mode(offset,*mb_can.method,*mb_can.switch_speed,*mb_can.zero_speed,*mb_can.nodeID);
      }
      else
      {
        MB_DEBUG_MOTOR_ENABLE_RESET;
      }
    }
    break;
    case INTERPOLATED_POSITION_MODE://�岹λ��ģʽ
//        motor_interpolation_position();
    break;
    }
  }
  break;
  case MBKEY_DISABLE: //�ɿ������¼�
  break;
  case MBKEY_PRESS:   //�ɿ��������¼�
  break;
  case MBKEY_RAISE:   //���µ��ɿ��¼�
  {
    if(*mb_can.motor_mode == PROFILE_VELOCITY_MODE)
    {
      motor_profile_velocity(0,*mb_can.nodeID);
    }
  }
  break;    
  }
}
/** 
  * @brief  ������ÿ���
  * @param  event 
  * @note   д��True���õ������,���ȼ�����ʹ�õ�������õ�����رյ��ʹ��״̬��
  */ 
static void key_motor_disable(mbkey_status *event)
{
  if(*mb_can.nodeID == MASTER_NODEID || *mb_can.nodeID > MAX_NODE_COUNT || *mb_can.nodeID == 0)
  {
    MB_DEBUG_MOTOR_DISABLE_SET;
    return;
  }
  switch(*event)
  {
  case MBKEY_ENABLE:  //���´����¼�
    MB_DEBUG_MOTOR_ENABLE_RESET;//ǿ���˳����ʹ�ܿ���
  break;
  case MBKEY_DISABLE: //�ɿ������¼�
  break;
  case MBKEY_PRESS:   //�ɿ��������¼�
    motor_off(*mb_can.nodeID);
    modbus_reset_register(0,17,0,30);//�������
  break;
  case MBKEY_RAISE:   //���µ��ɿ��¼�
  break;
  }
}
/** 
  * @brief  ת����[0]����
  * @param  event 
  * @note   д��1ʹ�ܵ���󣬿ɽ��е�����ơ�
  *         д��0���õ�������õ�����رյ��ʹ��״̬��
  */ 
static void key_turn0_control(mbkey_status *event)
{
  turn_motor_typeDef *p = &turn_motor[0];

  if(p->nodeID  == MASTER_NODEID || p->nodeID  > MAX_NODE_COUNT || p->nodeID  == 0)
  {
    return;
  }
  if(getNodeState(OD_Data,p->nodeID) != Operational)
  {
    return;
  }

  switch(*event)
  {
  case MBKEY_ENABLE:  //���´����¼�
  {
    int32_t angle = MAKEINT_32(*p->mb.angle_h,*p->mb.angle_l) / 1000.0f;//��λ:0.001��
    float   speed = *p->mb.speed / 10;//��λ:0.1RPM
    turn_motor_angle_control(angle,speed,p);
  }
    break;
  case MBKEY_DISABLE: //�ɿ������¼�
    turn_motor_stop(p);
    break;
  case MBKEY_PRESS:   //�ɿ��������¼�
    turn_motor_enable(p);
    //���»���,��֤��ʼ�����ʱ�Ƕ�׼ȷ
    turn_motor_reentrant(p);
    break;
  case MBKEY_RAISE:   //���µ��ɿ��¼�
    turn_motor_disable(p);
    break;    
  }
}
/** 
  * @brief  ���ߵ��[0]����
  * @param  event 
  * @note   д��1ʹ�ܵ���󣬿ɽ��е�����ơ�
  *         д��0���õ�������õ�����رյ��ʹ��״̬��
  */ 
static void key_walk0_control(mbkey_status *event)
{
  walk_motor_typeDef *p = &walk_motor[0];

  if(p->nodeID  == MASTER_NODEID || p->nodeID  > MAX_NODE_COUNT || p->nodeID  == 0)
  {
    return;
  }
  if(getNodeState(OD_Data,p->nodeID) != Operational)
  {
    return;
  }

  switch(*event)
  {
  case MBKEY_ENABLE:  //���´����¼�
  { 
    float   speed = *p->mb.speed / 10;//��λ:0.1RPM
    walk_motor_speed_control(speed,p);
  }
    break;
  case MBKEY_DISABLE: //�ɿ������¼�
    break;
  case MBKEY_PRESS:   //�ɿ��������¼�
    walk_motor_enable(p);
    walk_motor_reentrant(p);
    break;
  case MBKEY_RAISE:   //���µ��ɿ��¼�
    walk_motor_disable(p);
    break;    
  }
}
/**************************״̬��**********************************************/
/**
  * @brief  ����ָ������.
  * @param  none.
  * @retval none.
  * @note   
ע�⺯����˳�������Խ������
https://blog.csdn.net/feimeng116/article/details/107515317
*/
static void (*operation[MBKEY_NUM])(mbkey_status *event) = 
{
  key_motor_enable,
  key_motor_disable,
  key_turn0_control,
  key_walk0_control,
};
/** 
  * @brief  ��ȡio��ƽ�ĺ���
  ������ȡ����
  */  
static bits_status readpin(config bits)
{
  return (bits_status)modbus_get_bits(bits.index,bits.sub_index);
}
/**
  * @brief  ��ȡ����ֵ
  * @param  none.
  * @retval none.
  * @note   ����ʵ�ʰ��°�ť�ĵ�ƽȥ�������������Ľ��
*/
static void get_level(void)
{
    for(uint8_t i = 0;i < MBKEY_NUM;i++)
    {
        if(mbkey_buf[i].status.shield == DISABLE)	//��������򲻽��а���ɨ��
            continue;
        if(mbkey_buf[i].status.read_pin(mbkey_buf[i].board) == mbkey_buf[i].status.down_level)
            mbkey_buf[i].status.flag = LOW_LEVEL;
        else
            mbkey_buf[i].status.flag = HIGH_LEVEL;
    }
}
/**
  * @brief  ������������
  * @param  mbkey_init
  * @retval none.
  * @note   ������������
*/
static void creat_key(config* init)
{
  for(uint8_t i = 0;i < MBKEY_NUM;i++)
	{
		mbkey_buf[i].board = init[i]; // mbkey_buf��ť����ĳ�ʼ�����Ը�ֵ

		mbkey_buf[i].board.key_nox = i;
		// ��ʼ����ť�����״̬������
		mbkey_buf[i].status.shield = MB_ENABLE;
		mbkey_buf[i].status.timecount = 0;	
		mbkey_buf[i].status.flag = LOW_LEVEL;
    
		if(mbkey_buf[i].board.gpio_pull == PULLUP) // ����ģʽ���и�ֵ
			mbkey_buf[i].status.down_level = LOW_LEVEL;
		else
			mbkey_buf[i].status.down_level = HIGH_LEVEL;
    
		mbkey_buf[i].status.key_status = 	MBKEY_DISABLE;
		mbkey_buf[i].status.key_event	= 	MBKEY_DISABLE;
		mbkey_buf[i].status.read_pin 	= 	readpin;	//��ֵ������ȡ����
	}
}
/**
  * @brief  ��ȡ����
  * @param  none.
  * @retval none.
  * @note   ״̬����״̬ת��
*/
static void read_status(void)
{
  get_level();
  for(uint8_t i = 0;i < MBKEY_NUM;i++)
  {
    switch(mbkey_buf[i].status.key_status)
    {
    //״̬0�������Ϳ�
    case MBKEY_DISABLE:
      if(mbkey_buf[i].status.flag == LOW_LEVEL)
      {
          mbkey_buf[i].status.key_status = MBKEY_DISABLE;  //ת��״̬3
          mbkey_buf[i].status.key_event  = MBKEY_DISABLE;  //���¼�
      }
      else
      {
        mbkey_buf[i].status.key_status = MBKEY_PRESS;        //ת��״̬1
        mbkey_buf[i].status.key_event 	= MBKEY_PRESS;        //�����¼�
      }
    break;
    //״̬1����������
    case MBKEY_ENABLE:
      if(mbkey_buf[i].status.flag == HIGH_LEVEL)
      {
          mbkey_buf[i].status.key_status = MBKEY_ENABLE;     //ת��״̬3
          mbkey_buf[i].status.key_event  = MBKEY_ENABLE;     //���¼�
      }
      else
      {
        mbkey_buf[i].status.key_status = MBKEY_RAISE;        //ת��״̬0
        mbkey_buf[i].status.key_event  = MBKEY_RAISE;        //�����¼�
      }
    break;
    //״̬2����������[�Ϳ�������]
    case MBKEY_PRESS:
      if(mbkey_buf[i].status.flag == HIGH_LEVEL)
      {
          mbkey_buf[i].status.key_status = MBKEY_ENABLE;     //ת��״̬3
          mbkey_buf[i].status.key_event  = MBKEY_ENABLE;     //���¼�
      }
      else
      {
          mbkey_buf[i].status.key_status = MBKEY_DISABLE;  //ת��״̬3
          mbkey_buf[i].status.key_event  = MBKEY_DISABLE;  //���¼�
      }
    break;
    //״̬1����������[���µ��Ϳ�]
    case MBKEY_RAISE:
      if(mbkey_buf[i].status.flag == LOW_LEVEL)          //�����ͷţ��˿ڸߵ�ƽ
      {
          mbkey_buf[i].status.key_status = MBKEY_DISABLE;  //ת��״̬3
          mbkey_buf[i].status.key_event  = MBKEY_DISABLE;  //���¼�
      }
      else
      {
          mbkey_buf[i].status.key_status = MBKEY_ENABLE;     //ת��״̬3
          mbkey_buf[i].status.key_event = MBKEY_ENABLE;      //���¼�
      }
    break;
    }
  }
}
/**
  * @brief  ������
  * @param  p.
  * @retval none.
  * @note   ���ڶ�ʱ��1msһ��
            ����ʼ��ֵ����mbģʽ������
            ɨ�谴���󣬶�ȡ����״̬���¼������ж�����
*/
void mbkey_handler(void *p)
{
	uint8_t i;
  while(1)
  {
    rt_thread_mdelay(10);
    modbus_mutex_lock();
    read_status();
    for(i = 0;i < MBKEY_NUM;i++)
    {
        if(operation[i] != RT_NULL)
          operation[i](&mbkey_buf[i].status.key_event);
    }
    modbus_mutex_unlock();
  }
}
/**
  * @brief  mbkey�������β���
  * @param  num:mbkey_listע�����ѡ��
  * @param  option: enable  �����á�
                    disable �����á�
  * @retval none.
  * @note   ���û�������
*/
void mbkey_shield_operate(uint8_t num,mbkey_enable_status option)
{
  mbkey_buf[num].status.shield       = option;
  mbkey_buf[num].status.key_event    = MBKEY_DISABLE;//�˳��¼�
}
/**
  * @brief  io��ʼ����ʼ��
  * @param  none.
  * @retval int.
  * @note   ״̬����ʼ��
gpio_pullup����ʼ���ߣ��˿ڣ�λ��
*/
int mbkey_init(void)
{ 
  rt_err_t ret = RT_EOK; 
  config init[MBKEY_NUM]=
  { 
    //�����ƽ ���� ������
    {PULLUP,    0,    1},  //���ʹ��
    {PULLUP,    0,    2},  //�������
    {PULLUP,    0,    11}, //ת����[1]����
    {PULLUP,    0,    15}, //���ߵ��[1]����
  };
  creat_key(init);// ���ð�����ʼ������
  /* ���� MODBUS�߳�*/
  rt_thread_t thread = rt_thread_create( "mb_key",    /* �߳����� */
                                         mbkey_handler,/* �߳���ں��� */
                                         RT_NULL,       /* �߳���ں������� */
                                         THREAD_STACK_SIZE, /* �߳�ջ��С */
                                         THREAD_PRIORITY,   /* �̵߳����ȼ� */
                                         THREAD_TIMESLICE); /* �߳�ʱ��Ƭ */
  /* �����ɹ��������߳� */
  if (thread != RT_NULL)
  {
      rt_thread_startup(thread);
  }
  else
  {
      ret = RT_ERROR;
      LOG_E("modbus slave created failed.");
  }
  return ret;
}
