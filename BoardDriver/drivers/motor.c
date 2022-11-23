/**
 * @file motor.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-21
 * @copyright Copyright (c) 2022
 * @attention 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-21 1.0     HLY     first version
 */
/* Includes ------------------------------------------------------------------*/
#include "motor.h"
/* Private includes ----------------------------------------------------------*/
#include "motor_control.h"
#include "modbus_slave_common.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
turn_motor_typeDef turn_motor[TURN_MOTOR_NUM];
/******************************���ߵ������**************************************/
/**
 * @brief ת����ʹ��
 * @param  p
 * @note  ��ʼ��Ϊλ�ù滮ģʽ
 */
uint8_t turn_motor_enable(turn_motor_typeDef* p)
{
  return motor_on_profile_position(p->nodeID);
}
/**
 * @brief ת��������
 * @param  p
 * @note  �رյ��ʹ��
 */
uint8_t turn_motor_disable(turn_motor_typeDef* p)
{
  return motor_off(p->nodeID);
}
/**
 * @brief  ת����ֹͣ�˶�����
 * @param  p        
 * @note   none
 */
uint8_t turn_motor_stop(turn_motor_typeDef* p)
{
  return 0;
}
/**
  * @brief  ת������ͣ���ȼ�.
  * @param  p
  * @retval None.
  * @note   
*/
static uint8_t motor_stop_priority(turn_motor_typeDef* p)
{
  if(p->stop_state != NO_STOP)
  {
    turn_motor_stop(p);
    return 1;
  }
  else
    return 0;
}
/**
 * @brief  ת������ʼ�˶�����
 * @param  position         
 * @param  speed ��λ��RPM
 * @param  p
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01
 * @note   �Ծ���λ��ģʽ�����̴���ָ���˶�      
 */
static uint8_t turn_motor_start(int32_t position,float speed,turn_motor_typeDef* p)
{
  if (motor_stop_priority(p))
    return 0X01;
  
  return motor_profile_position(position,speed,0,1,p->nodeID);
}
/**
 * @brief   �ǶȻ���
 * @param  angle            
 * @retval float 
 * @note   �ǶȻ��� ��Χ -180~180
 */
static float angle_conversion(float angle)
{
  angle = fmod(angle,360);
  if(angle == 180)
    angle = -180;
  else if(angle > 180)
    angle =  angle- 360;
  else if(angle < -180)
    angle = angle + 360;
  return angle;
}
/**
  * @brief  �жϵ���Ƕȷ�Χ
  * @param  SMove_AngleTypeDef ����ǶȽṹ��
  * @param  input ����Ҫ�ж��ĽǶ�
  * @retval None
  * @note   �ж��Ƿ񳬳�����������Ϊ�ٽ�ֵ
            ��û�����ýǶȷ�Χ�����жϷ�Χ���ƣ���������ֵ
*/
static float angle_range_judgment(turn_motor_typeDef *p,float angle)
{
  if(p->min_angle == 0 && p->max_angle == 0)
    return angle;
  if(p->min_angle <= angle && angle <= p->max_angle)
  {
    p->over_range = DISABLE;//����û�г����Ƕ�
  }
  else if(angle < p->min_angle)
  {
    angle = p->min_angle;
    p->over_range = ENABLE;//���볬���Ƕ�
    ulog_w("turn","Input beyond minimum Angle");
  }
  else if(angle > p->max_angle)
  {
    angle = p->max_angle;
    p->over_range = ENABLE;//���볬���Ƕ�
    ulog_w("turn","Input beyond maximum Angle");
  }
  return angle;
}
/**
 * @brief  ת�����Ƕȿ���
 * @param  angle       
 * @param  speed  ��λ RPM
 * @param  p                
 * @note   �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE
           ������ͣ����0X01,�Ƕ��ޱ仯����0X02
 * @note   �Ѿ��ԽǶ����
 */
uint8_t turn_motor_angle_control(float angle,float speed,turn_motor_typeDef* p)
{
  int32_t dest_position = 0;//Ŀ��λ��
  //�ǶȻ���
  angle = angle_conversion(angle);

  /*�Ƕȸ����ж�*/
  p->err  = angle - p->last;
  p->last = angle;
  
  if(p->err != 0)
  {
    //�Ƕ��޷�
    angle = angle_range_judgment(p,angle);
    //�ǶȻ���Ϊλ��
    dest_position = angle / 360 * p->cfg.numerator;
    //��ʼ�˶�
    return turn_motor_start(dest_position,speed,p);
  }
  return 0X02;
}
/**
  * @brief  ���ص���Ƕ�
  * @param  p
  * @retval �Ƕ�
  * @note   
*/
float turn_motor_get_angle(turn_motor_typeDef* p)
{
  long position = 0;
  float degree = 0;
  //��ȡ����λ��
  motor_get_position(&position,p->nodeID);
  //����Ϊ�Ƕ�
  degree = (float)position / p->cfg.numerator * 360;
  //�ǶȻ���
  degree = angle_conversion(degree);
  return degree;
}
/**
 * @brief  ��ȡת���������Ƕȱ�־
 * @param  p                
 * @retval true 
 * @retval false 
 */
bool turn_motor_get_over_range(turn_motor_typeDef* p)
{
  return p->over_range;
}
/**
 * @brief  ����ת�����Ƕȷ�Χ
 * @param  max              
 * @param  min              
 * @param  p                
 */
void turn_motor_set_angle_range(int16_t max,int16_t min,turn_motor_typeDef* p)
{
  p->max_angle = max;
  p->min_angle = min;
}
/******************************���ߵ������**************************************/

/******************************��������******************************************/
/**
 * @brief  �����ʼ���߳�
 * @param  p       
 * @note   1msһ�β�ѯ����ڵ��Ƿ���в���״̬�������ٽ��ж�Ӧ�ڵ��ʼ��
 */
static void motor_init_thread(void * p)
{
  turn_motor[0].nodeID = SERVO_NODEID_1;
  nodeID_get_config(&turn_motor[0].cfg,turn_motor[0].nodeID);

  while(1)
  {
    if(getNodeState(OD_Data,turn_motor[0].nodeID) == Operational)
    {
      MB_TURN_SET;
      return;
    }
    rt_thread_mdelay(1);
  }
}
/**
 * @brief �����ʼ��
 * @note  ��canopen��ȥ����ģʽ�����
 */
void motor_init(void)
{
  rt_err_t ret = RT_EOK;
  /* ���� MODBUS�ӻ��߳�*/
  rt_thread_t thread = rt_thread_create( "motor_init",      /* �߳����� */
                                         motor_init_thread, /* �߳���ں��� */
                                         RT_NULL,           /* �߳���ں������� */
                                         1024,              /* �߳�ջ��С */
                                         5,                 /* �̵߳����ȼ� */
                                         20);               /* �߳�ʱ��Ƭ */
  /* �����ɹ��������߳� */
  if (thread != RT_NULL)
  {
      rt_thread_startup(thread);
  }
  else
  {
      ret = RT_ERROR;
      LOG_E("motor_init created failed.");
  }
}