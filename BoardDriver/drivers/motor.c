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
#include "master402_canopen.h"
#include "modbus_slave_common.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
turn_motor_typeDef turn_motor[TURN_MOTOR_NUM];
walk_motor_typeDef walk_motor[TURN_MOTOR_NUM];
/******************************���ߵ������**************************************/
/**
  * @brief  ����Ƕȶ�λ�Ƕ�����
  * @param  None.
  * @retval None.
  * @note   ��֤�ٴν���Ƕȶ�λ�������̽��ж�λ��
*/
void turn_motor_reentrant(turn_motor_typeDef* p)
{
    //���»���,��֤��ʼ�����ʱ�Ƕ�׼ȷ
    p->last = turn_motor_get_angle(p);
    *p->mb.angle_h = (int32_t)(p->last * 1000) >> 16;
    *p->mb.angle_l = (int32_t)(p->last * 1000);
}
/**
  * @brief  ת������ͣ���ȼ�.
  * @param  p
  * @retval None.
  * @note   
*/
static uint8_t turn_motor_stop_priority(turn_motor_typeDef* p)
{
  if(*p->stop_state != NO_STOP)
  {
    turn_motor_stop(p);
    turn_motor_reentrant(p);
    ulog_w("turn","turn motor stop,code is 0X%4.4x",*p->stop_state);
    return 1;
  }
  else
    return 0;
}
/**
 * @brief ת����ʹ��
 * @param  p
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01
 * @note  ��ʼ��Ϊλ�ù滮ģʽ
 */
uint8_t turn_motor_enable(turn_motor_typeDef* p)
{
  return motor_on_profile_position(p->nodeID);
}
/**
 * @brief ת��������
 * @param  p
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01
 * @note  �رյ��ʹ��
 */
uint8_t turn_motor_disable(turn_motor_typeDef* p)
{
  return motor_off(p->nodeID);
}
/**
 * @brief  ת����ֹͣ�˶�����
 * @param  p   
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01     
 * @note   none
 */
uint8_t turn_motor_stop(turn_motor_typeDef* p)
{
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
  if (turn_motor_stop_priority(p))
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
  p->max_angle = *p->mb.max_angle;//���Ƕ�
  p->min_angle = *p->mb.min_angle;//��С�Ƕ�

  if(p->min_angle == 0 && p->max_angle == 0)
    return angle;
  if(p->min_angle <= angle && angle <= p->max_angle)
  {
    *p->over_range = false;//����û�г����Ƕ�
  }
  else if(angle < p->min_angle)
  {
    angle = p->min_angle;
    *p->over_range = true;//���볬���Ƕ�
    ulog_w("turn","Input beyond minimum Angle");
  }
  else if(angle > p->max_angle)
  {
    angle = p->max_angle;
    *p->over_range = true;//���볬���Ƕ�
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
/******************************���ߵ������**************************************/
/**
  * @brief  
  * @param  None.
  * @retval None.
  * @note   
*/
void walk_motor_reentrant(walk_motor_typeDef* p)
{

}
/**
  * @brief  ���ߵ����ͣ���ȼ�.
  * @param  p
  * @retval None.
  * @note   
*/
static uint8_t walk_motor_stop_priority(walk_motor_typeDef* p)
{
  if(*p->stop_state != NO_STOP)
  {
    walk_motor_stop(p);
    walk_motor_reentrant(p);
    ulog_w("walk","walk motor stop,code is 0X%4.4x",*p->stop_state);
    return 1;
  }
  else
    return 0;
}
/**
 * @brief ���ߵ��ʹ��
 * @param  p
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01
 * @note  ��ʼ��Ϊ�ٶȹ滮ģʽ
 */
uint8_t walk_motor_enable(walk_motor_typeDef* p)
{
  return motor_on_profile_velocity(p->nodeID);
}
/**
 * @brief ���ߵ������
 * @param  p
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01
 * @note  �رյ��ʹ��
 */
uint8_t walk_motor_disable(walk_motor_typeDef* p)
{
  return motor_off(p->nodeID);
}
/**
 * @brief  ���ߵ��ֹͣ�˶�����
 * @param  p   
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01     
 * @note   none
 */
uint8_t walk_motor_stop(walk_motor_typeDef* p)
{
  return 0;
}
/**
 * @brief  ���ߵ����ʼ�˶�����
    
 * @param  speed ��λ��RPM
 * @param  p
 * @retval �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE.
           ������ͣ����0X01
 * @note   �ٶȹ滮ģʽ    
 */
static uint8_t walk_motor_start(int16_t speed,walk_motor_typeDef* p)
{
  if (walk_motor_stop_priority(p))
    return 0X01;
  
  return motor_profile_velocity(speed,p->nodeID);
}
/**
  * @brief  �жϵ���ٶȷ�Χ
  * @param  SMove_AngleTypeDef ����ٶȽṹ��
  * @param  input ����Ҫ�ж����ٶ�
  * @retval None
  * @note   �ж��Ƿ񳬳�����������Ϊ�ٽ�ֵ
            ��û�������ٶȷ�Χ�����жϷ�Χ���ƣ���������ֵ
*/
static float speed_range_judgment(walk_motor_typeDef *p,float input)
{
  float speed = fabs(input);

  p->max_speed = *p->mb.max_speed;//�������
  p->min_speed = 0;

  if(p->min_speed == 0 && p->max_speed == 0)
    return speed;

  if(p->min_speed <= speed && speed <= p->max_speed)
  {
    *p->over_range = false;//����û�г���
  }
  else if(speed < p->min_speed)
  {
    speed = p->min_speed;
    *p->over_range = true;//���볬��
    ulog_w("walk","Input beyond minimum speed");
  }
  else if(speed > p->max_speed)
  {
    speed = p->max_speed;
    *p->over_range = true;//���볬��
    ulog_w("walk","Input beyond maximum speed");
  }
  return speed;
}
/**
 * @brief  ���ߵ���ٶȿ���  
 * @param  speed  ��λ RPM
 * @param  p                
 * @note   �ɹ�����0X00,ģʽ���󷵻�0XFF.��ʱ����0XFE
           ������ͣ����0X01,�ٶ��ޱ仯����0X02
 * @note   
 */
uint8_t walk_motor_speed_control(float speed,walk_motor_typeDef* p)
{
  /*�Ƕȸ����ж�*/
  p->err  = speed - p->last;
  p->last = speed;
  
  if(p->err != 0)
  {
    //�ٶ��޷�
    speed = speed_range_judgment(p,speed);
    //��ʼ�˶�
    return walk_motor_start(speed,p);
  }
  return 0X02;
}
/**
  * @brief  ���ص���ٶ�
  * @param  p
  * @retval �ٶ� RPM
  * @note   
*/
float walk_motor_get_speed(walk_motor_typeDef* p)
{
  long speed;
  //��ȡ�ٶ�
  motor_get_velocity(&speed,p->nodeID);
  return (float)speed / 10;//ע�ⵥλΪ0.1rpm
}
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
  walk_motor[0].nodeID = SERVO_NODEID_2;
  nodeID_get_config(&walk_motor[0].cfg,walk_motor[0].nodeID);

  uint16_t event = 0;
  USER_SET_BIT(event,1); 
  USER_SET_BIT(event,2);
  while(1)
  {
    if(getNodeState(OD_Data,turn_motor[0].nodeID) == Operational)
    {
      MB_TURN1_SET;
      USER_CLEAR_BIT(event,1); 
    }
    if(getNodeState(OD_Data,walk_motor[0].nodeID) == Operational)
    {
      MB_WALK1_SET;
      USER_CLEAR_BIT(event,2); 
    }
    if(event == 0)
    {
      rt_kprintf("All the motors have been powered on and enabled\n");
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