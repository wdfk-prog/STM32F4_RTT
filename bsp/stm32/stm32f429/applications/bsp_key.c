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
#include "bsp_key.h"
/* Private includes ----------------------------------------------------------*/
#include <ipc/ringbuffer.h>
#include <rtthread.h>
#include <board.h>
#include "mfbd.h"

#include "mb_handler.h"
#include "turn_motor.h"
#include "walk_motor.h"
/*ulog include*/
#define LOG_TAG              "bsp key" 
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/
/* ���ζ���GPIO */
typedef struct
{
	GPIO_TypeDef* gpio;
	uint16_t pin;
	uint8_t ActiveLevel;	/* �����ƽ */
}X_GPIO_T;
/* ����ID*/
typedef enum
{
  KID_Stop_Button = 0X00,
  KID_WK_UP,
  KID_Before_Radar_1,
  KID_Before_Radar_2,
  KID_Before_Radar_3,
  KID_Start_Button,
  KID_Article_Crash,
  KID_Turn_ALM,
  KID_Walk_ALM,
  KID_Lift_SQP,
  KID_Lift_Reflection,
  KID_Lower_Limit,
  HARD_KEY_NUM,// ʵ�尴������
}KEY_ID;
/*
	�����ֵ����, ���밴���´���ʱÿ�����İ��¡�����ͳ����¼�

	�Ƽ�ʹ��enum, ����#define��ԭ��
	(1) ����������ֵ,�������˳��ʹ���뿴���������
	(2) �������ɰ����Ǳ����ֵ�ظ���
*/
typedef enum
{
	KEY_NONE = 0X00,			      /* 0 ��ʾ�����¼� */
  
  Stop_Button_DOWN_CODE,				   
  Stop_Button_UP_CODE,				    
  Stop_Button_LONG_CODE,				   
  
  WK_UP_DOWN_CODE,				         
  WK_UP_UP_CODE,				           
  WK_UP_LONG_CODE,				         
  
  Before_Radar_1_DOWN_CODE,				
  Before_Radar_1_UP_CODE,				  
  Before_Radar_1_LONG_CODE,				

  Before_Radar_2_DOWN_CODE,				
  Before_Radar_2_UP_CODE,				 
  Before_Radar_2_LONG_CODE,				
  
  Before_Radar_3_DOWN_CODE,			
  Before_Radar_3_UP_CODE,				
  Before_Radar_3_LONG_CODE,			
  
  Start_Button_DOWN_CODE,				 
  Start_Button_UP_CODE,				   
  Start_Button_LONG_CODE,				 
  
  Article_Crash_DOWN_CODE,				
  Article_Crash_UP_CODE,				 
  Article_Crash_LONG_CODE,				

  Turn_ALM_DOWN_CODE,				      
  Turn_ALM_UP_CODE,				        
  Turn_ALM_LONG_CODE,				       
  
  Walk_ALM_DOWN_CODE,				        
  Walk_ALM_UP_CODE,				          
  Walk_ALM_LONG_CODE,				        
  
  Lift_SQP_DOWN_CODE,				    
  Lift_SQP_UP_CODE,				      
  Lift_SQP_LONG_CODE,				    

  Lift_Reflection_DOWN_CODE,				      
  Lift_Reflection_UP_CODE,				       
  Lift_Reflection_LONG_CODE,				      
  
  Lower_Limit_DOWN_CODE,				       
  Lower_Limit_UP_CODE,				         
  Lower_Limit_LONG_CODE,				       
}KEY_CODE;
/* GPIO��PIN���� */
static const X_GPIO_T s_gpio_list[HARD_KEY_NUM] = 
{
	{Stop_Button_GPIO_Port,           Stop_Button_Pin,          GPIO_PIN_SET},	  /* ��ͣ��ť*/
  {WK_UP_GPIO_Port,                 WK_UP_Pin,                GPIO_PIN_SET},    /* ���Ѱ�ť*/
  {Before_Radar_1_GPIO_Port,        Before_Radar_1_Pin,       GPIO_PIN_SET},    /* ǰ�״�1 */
  {Before_Radar_2_GPIO_Port,        Before_Radar_2_Pin,       GPIO_PIN_SET},    /* ǰ�״�2 */
  {Before_Radar_3_GPIO_Port,        Before_Radar_3_Pin,       GPIO_PIN_SET},    /* ǰ�״�3 */
  {Start_Button_GPIO_Port,          Start_Button_Pin,         GPIO_PIN_RESET},  /*������ť */
  {Article_Crash_GPIO_Port,         Article_Crash_Pin,        GPIO_PIN_RESET},  /*��ײ��   */
  {Turn_ALM_GPIO_Port     ,         Turn_ALM_Pin,             GPIO_PIN_SET},    /*ת�򱨾� */
  {Walk_ALM_GPIO_Port,              Walk_ALM_Pin,             GPIO_PIN_RESET},  /*���߱��� */
  {Lift_SQP_GPIO_Port,              Lift_SQP_Pin,             GPIO_PIN_RESET},  /*��۽ӽ�����      ��Ӱ������޸�*/
  {Lift_Reflection_GPIO_Port ,      Lift_Reflection_Pin,      GPIO_PIN_RESET},  /*��������䴫����  ��Ӱ������޸�*/
  {Lower_Limit_GPIO_Port,           Lower_Limit_Pin,          GPIO_PIN_RESET},  /*�������λ        ��Ӱ������޸�*/
};	
/* Private define ------------------------------------------------------------*/
#define RINGBUFFER_LEN       10//ringbuffer��������С
#define SCAN_PERIOD          10//ɨ������
/* Private macro -------------------------------------------------------------*/
/*֧��Button�����������Ĳ�������֧�ֶ����������*/
#if MFBD_USE_NORMAL_BUTTON//�ظ�ʱ���볤��ʱ��������˲�ʱ��
/*                        ����,             ��������,          �˲�ʱ��,        �ظ�ʱ�䣬     ����ʱ��*/
MFBD_NBTN_DEFAULT_DEFINE(Stop_Button,     KID_Stop_Button,    50/SCAN_PERIOD,100/SCAN_PERIOD, 100/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(WK_UP,           KID_WK_UP,          50/SCAN_PERIOD,100/SCAN_PERIOD, 1000/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(Before_Radar_1,  KID_Before_Radar_1, 50/SCAN_PERIOD,0/SCAN_PERIOD,   0/SCAN_PERIOD); //����һ��
MFBD_NBTN_DEFAULT_DEFINE(Before_Radar_2,  KID_Before_Radar_2, 50/SCAN_PERIOD,0/SCAN_PERIOD,   0/SCAN_PERIOD); //����һ��
MFBD_NBTN_DEFAULT_DEFINE(Before_Radar_3,  KID_Before_Radar_3, 50/SCAN_PERIOD,0/SCAN_PERIOD,   0/SCAN_PERIOD); //����һ��
MFBD_NBTN_DEFAULT_DEFINE(Start_Button,    KID_Start_Button  , 50/SCAN_PERIOD,0/SCAN_PERIOD,   1000/SCAN_PERIOD);//һ�γ���
MFBD_NBTN_DEFAULT_DEFINE(Article_Crash,   KID_Article_Crash,  50/SCAN_PERIOD,0/SCAN_PERIOD,   100/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(Turn_ALM,        KID_Turn_ALM,       50/SCAN_PERIOD,100/SCAN_PERIOD, 100/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(Walk_ALM,        KID_Walk_ALM,       50/SCAN_PERIOD,100/SCAN_PERIOD, 100/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(Lift_SQP,        KID_Lift_SQP,       50/SCAN_PERIOD,100/SCAN_PERIOD, 100/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(Lift_Reflection, KID_Lift_Reflection,50/SCAN_PERIOD,100/SCAN_PERIOD, 100/SCAN_PERIOD);//������ȡ
MFBD_NBTN_DEFAULT_DEFINE(Lower_Limit,     KID_Lower_Limit,    50/SCAN_PERIOD,0/SCAN_PERIOD, 0/SCAN_PERIOD);//����һ��
#endif /* MFBD_USE_NORMAL_BUTTON */
#if MFBD_USE_NORMAL_BUTTON
MFBD_NBTN_ARRAYLIST(nbtn_list, 
                    &Stop_Button, 
                    &WK_UP,
                    &Before_Radar_1,
                    &Before_Radar_2,
                    &Before_Radar_3,
                    &Start_Button,
                    &Article_Crash,
                    &Turn_ALM,
                    &Walk_ALM,
                    &Lift_SQP,
                    &Lift_Reflection,
                    &Lower_Limit
                    );
#endif /* MFBD_USE_NORMAL_BUTTON */
/* Private variables ---------------------------------------------------------*/
static void bsp_btn_value_report(mfbd_btn_code_t btn_value);
static unsigned char bsp_btn_check(mfbd_btn_index_t btn_index);
//���λ�����ָ��
struct rt_ringbuffer * rb;
const mfbd_group_t btn_group =
{
    bsp_btn_check,
    bsp_btn_value_report,

#if MFBD_USE_TINY_BUTTON
    test_tbtn_list,
#endif /* MFBD_USE_TINY_BUTTON */

#if MFBD_USE_NORMAL_BUTTON
    nbtn_list,
#endif /* MFBD_USE_NORMAL_BUTTON */

#if MFBD_USE_MULTIFUCNTION_BUTTON
    test_mbtn_list,
#endif /* MFBD_USE_MULTIFUCNTION_BUTTON */

#if MFBD_PARAMS_SAME_IN_GROUP

#if MFBD_USE_TINY_BUTTON || MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON
    3,
#endif /*  MFBD_USE_TINY_BUTTON || MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON */

#if MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON
    30,
    150,
#endif /* MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON */

#if MFBD_USE_MULTIFUCNTION_BUTTON
    75,
#endif /* MFBD_USE_MULTIFUCNTION_BUTTON */

#endif /*MFBD_PARAMS_SAME_IN_GROUP*/

};
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ���������Ϣ.
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Remove_Alarm(void)
{
  REMOVE_ALARM_RESET;
  /*******************������Ϣ����********************************/
  CRASH_ALARM_RESET;
  TURN_ALARM_RESET;
  Walk_ALARM_RESET;
  TURN_LIMIT_ALARM_RESET;
  TURN_ZERO_ALARM_RESET;
}
/*
*********************************************************************************************************
*	�� �� ��: ����������
*	����˵��: ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Key_Handle(mfbd_btn_code_t ucKeyCode)
{
  if (ucKeyCode != KEY_NONE)
  {
    switch (ucKeyCode)
    {
      case Stop_Button_UP_CODE:
        LOG_I("stop button UP");
        ReadEMG_RESET;
        SOFT_STOP_RESET;//��ͣ��ť�ɿ�ʱһ���ɿ���ͣ
        USER_CLEAR_BIT(turn.Stop_state,HARD_STOP);
        USER_CLEAR_BIT(walk.Stop_state,HARD_STOP);
        break;
      case Stop_Button_DOWN_CODE: 
        ReadEMG_SET;
        SOFT_STOP_SET;//��ͣ��ť����ʱһ��������ͣ
        USER_SET_BIT(turn.Stop_state,HARD_STOP);
        USER_SET_BIT(walk.Stop_state,HARD_STOP);
        break;
      case Stop_Button_LONG_CODE:
        LOG_W("stop button trigger");
        break;
      case WK_UP_UP_CODE: 
        LOG_I("WK_UP UP");
        break;
      case WK_UP_DOWN_CODE:
        LOG_I("WK_UP DOWN");
        break;
      case WK_UP_LONG_CODE:
        LOG_I("WK_UP LONG");
        break;
      case Before_Radar_1_DOWN_CODE:
        BEFORE_RADAR_1_SET;
        break;
      case Before_Radar_1_UP_CODE:
        BEFORE_RADAR_1_RESET;
        break;
      case Before_Radar_2_DOWN_CODE:
        BEFORE_RADAR_2_SET;
        break;
      case Before_Radar_2_UP_CODE:
        BEFORE_RADAR_2_RESET;
        break;
      case Before_Radar_3_DOWN_CODE:
        BEFORE_RADAR_3_SET;
        break;
      case Before_Radar_3_UP_CODE:
        BEFORE_RADAR_3_RESET;
        break;
      case Start_Button_DOWN_CODE:
        LOG_I("Press the start button");
        TURN_ZERO_SET;
        START_SET;
        break;
      case Start_Button_UP_CODE:
        START_RESET;
        break;
      case Start_Button_LONG_CODE:
        rt_kprintf("\033[31;22m[I/KEY] Long pressthe start button,MCU restart");
        LOG_W("Long pressthe start button,MCU restart");
        rt_hw_cpu_reset();//����������
        break;
        break;
      case Article_Crash_DOWN_CODE://����������״̬
        CRASH_SET;
        CRASH_ALARM_SET;
        USER_SET_BIT(turn.Stop_state,CRASH_STOP);
        USER_SET_BIT(walk.Stop_state,CRASH_STOP);
        break;
      case Article_Crash_UP_CODE:
        CRASH_RESET;//����������״̬
        break;
      case Article_Crash_LONG_CODE:
        LOG_E("The article crash snesor touch");
        break;
      case Turn_ALM_DOWN_CODE:
        TURN_ALARM_SET;
        turn.ALM.state = SENSOR_TOUCH;   //���ñ�־
        USER_SET_BIT(turn.Stop_state,ALM_STOP);
        break;
      case Turn_ALM_UP_CODE:
        turn.ALM.state = SENSOR_LEAVE;   //���ñ�־
        break;
      case Turn_ALM_LONG_CODE:
        LOG_E("turn motor alarm  snesor touch");
        break;
      case Walk_ALM_DOWN_CODE:
        Walk_ALARM_SET;
        walk.ALM.state = SENSOR_TOUCH;   //���ñ�־
        USER_SET_BIT(walk.Stop_state,ALM_STOP);
        break;
      case Walk_ALM_UP_CODE:
        walk.ALM.state = SENSOR_LEAVE;   //���ñ�־
        break;
      case Walk_ALM_LONG_CODE:
        LOG_E("walk motor alarm  snesor touch");
        break;
      case Lift_SQP_DOWN_CODE:
        LIFT_SQP_SET;
        break;
      case Lift_SQP_UP_CODE:
        LIFT_SQP_RESET;
        break;
      case Lift_SQP_LONG_CODE:
        LOG_I("lift SQP snesor touch");
        break;
      case Lift_Reflection_DOWN_CODE:
        LIFT_REFLECTION_SET;
        break;       
      case Lift_Reflection_UP_CODE:		
        LIFT_REFLECTION_RESET;
        break; 
      case Lift_Reflection_LONG_CODE:
        LOG_W("lift Reflection snesor touch");
        break;
      case Lower_Limit_DOWN_CODE:	
        LIFT_LOWER_SET;
        break;
      case Lower_Limit_UP_CODE:
        LIFT_LOWER_RESET;
        break;
      default:
        /* �����ļ�ֵ������ */
        break;
    }
  }
}
/******************************��������********************************************/
/**
  * @brief  KeyPinActive.
  * @param  None.
  * @retval ����ֵ1 ��ʾ����(��ͨ����0��ʾδ���£��ͷţ�.
  * @note   �жϰ����Ƿ���.
*/
static uint8_t KeyPinActive(mfbd_btn_index_t _id)
{
	uint8_t level;
	
	if ((s_gpio_list[_id].gpio->IDR & s_gpio_list[_id].pin) == 0)
	{
		level = 0;
	}
	else
	{
		level = 1;
	}

	if (level == s_gpio_list[_id].ActiveLevel)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/**
  * @brief  �������.
  * @param  None.
  * @retval None.
  * @note   STM32 HAL��ר��.
*/
static unsigned char bsp_btn_check(mfbd_btn_index_t btn_index)
{
    return KeyPinActive(btn_index);
}
/**
  * @brief  ��ֵ����.
  * @param  None.
  * @retval None.
  * @note   None
*/
static void bsp_btn_value_report(mfbd_btn_code_t btn_value)
{
  LOG_D("%d", btn_value);
  //�ýӿڻ����д��ʣ������ݣ������µ����ݸ��Ǿɵ����ݡ�
  rt_ringbuffer_put_force(rb,(rt_uint8_t *)&btn_value,sizeof(mfbd_btn_code_t));
}
/**
  * @brief  ����ɨ���߳�.
  * @param  None.
  * @retval None.
  * @note   None
*/
static void mfbd_scan(void *arg)
{
  while (1)
  {
    mfbd_group_scan(&btn_group); /* scan button group */
    rt_thread_mdelay(SCAN_PERIOD); /* scan period: 10ms */
  }
}
/**
  * @brief  ���������߳�.
  * @param  None.
  * @retval None.
  * @note   None
*/
static void mfbd_handler(void *p)
{
  while(1)
  {
    mfbd_btn_code_t recv_data;
    rt_size_t recv = rt_ringbuffer_get(rb, (rt_uint8_t *)&recv_data, sizeof(recv_data));
    if(recv == sizeof(recv_data))
      Key_Handle(recv_data);    
    rt_thread_mdelay(50); /* handler period: 50ms */
  }
}
/**
  * @brief  ������ʼ��
  * @param  None.
  * @retval None.
  * @note   None
*/
static void user_button_init(void)
{
  //MX_GPIO_Init();�Ѿ���ʼ��
}
/**
  * @brief  �����̳߳�ʼ��
  * @param  None.
  * @retval None.
  * @note   None
*/
int mfbd_main(void)
{
  rt_err_t ret = RT_EOK;
  user_button_init();
                                                  //length	�������ֽڴ�С
  rb = rt_ringbuffer_create(sizeof(mfbd_btn_code_t) * RINGBUFFER_LEN);
  RT_ASSERT(rb != RT_NULL);

  /* �����߳�*/
  rt_thread_t thread = rt_thread_create( "mfbd scan",    /* �߳����� */
                                         mfbd_scan,/* �߳���ں��� */
                                         RT_NULL,       /* �߳���ں������� */
                                         512, /* �߳�ջ��С */
                                         8,   /* �̵߳����ȼ� */
                                         10); /* �߳�ʱ��Ƭ */
  /* �����ɹ��������߳� */
  if (thread != RT_NULL)
  {
      rt_thread_startup(thread);
  }
  else
  {
      ret = RT_ERROR;
      LOG_E("mf scan created failed.");
  }
  /* �����߳�*/
             thread = rt_thread_create(  LOG_TAG,           /* �߳����� */
                                         mfbd_handler,      /* �߳���ں��� */
                                         RT_NULL,           /* �߳���ں������� */
                                         1024,              /* �߳�ջ��С */
                                         11,                /* �̵߳����ȼ� */
                                         10);               /* �߳�ʱ��Ƭ */
  /* �����ɹ��������߳� */
  if (thread != RT_NULL)
  {
      rt_thread_startup(thread);
  }
  else
  {
      ret = RT_ERROR;
      LOG_E("mf hand created failed.");
  }
  return ret;
}
INIT_APP_EXPORT(mfbd_main);