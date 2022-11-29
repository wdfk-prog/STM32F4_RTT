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
#include "led.h"
/* Private includes ----------------------------------------------------------*/
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "signal_led.h"
#include "user_math.h"
#include "main.h"
#include "mb_handler.h"
#include "monitor.h"
/* Private typedef -----------------------------------------------------------*/
/** 
  * @brief LEDģʽ����
  */  
typedef enum
{
  NORMAL_MODE = 0X00,//����ģʽ
  IPC_BEAT_MODE,     //���ػ������쳣
  MOTOR_ALM_MODE,    //��������쳣
  VOLTAGE_MODE,      //��ѹ����
}LED_MODE;
/* Private define ------------------------------------------------------------*/
#define LED_PORT      LED_GPIO_Port
#define LED_PIN       LED_Pin
#define BRIGHT_STATE  GPIO_PIN_RESET

#define BEEP_PORT     Beep_GPIO_Port
#define BEEP_PIN      Beep_Pin
#define BEEP_STATE    GPIO_PIN_SET
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
//�����ڴ���������ӿ�
led_mem_opreation_t led_mem_opreation;

LED_MODE led_alm_state = NORMAL_MODE;
/* Private function prototypes -----------------------------------------------*/
#if (BEEP_ENABLE == 1)
/*************************������**************************************************/
//�����źŵƶ�����
led_t *beep =  NULL;
/*  �����źŵ�һ�������ڵ���˸ģʽ
 *  ��ʽΪ �������������������� ���������� ���Ȳ���
 *  ע�⣺  �����õ�λΪ���룬�ұ������ ��LED_TICK_TIME�� �꣬��Ϊ����������Ϊ������������ȡ������
 *          ������Ӣ�Ķ���Ϊ���������Ӣ�Ķ��Ž�β���ַ�����ֻ���������ּ����ţ������������ַ�����
 */
char *beep_blink_mode_0 = "500,500,";                        //1Hz��˸
char *beep_blink_mode_1 = "200,200,100,500";                 //������˸����
char *beep_blink_mode_2 = "200,200,200,200,200,500";         //������˸����
char *beep_blink_mode_3 = "200,200,200,200,200,200,200,500"; //�Ĵ���˸����
char *beep_blink_mode_off = "0,100,";   //����
char *beep_blink_mode_on = "100,0,";    //����
/**
  * @brief  �û�����ģʽ
  * @param  None.
  * @retval None.
  * @note   -1��û�д�������
  ����ʱ�� =  ���� *ģʽʱ�䣻ms
*/
static void User_BEEP_Setmode(LED_MODE mode)
{
  switch(mode)
  {
    case NORMAL_MODE://����
      led_set_mode(beep, LOOP_PERMANENT, beep_blink_mode_off);
      break;
    case IPC_BEAT_MODE: //�����쳣
       led_set_mode(beep, LOOP_PERMANENT, beep_blink_mode_1);
      break;
    case MOTOR_ALM_MODE://��������쳣
       led_set_mode(beep, LOOP_PERMANENT, beep_blink_mode_2);
      break;
    case VOLTAGE_MODE://��ѹ�쳣���¶��쳣��
       led_set_mode(beep, LOOP_PERMANENT, beep_blink_mode_3);
      break;
    default:
      break;
  }
  led_start(beep);
}
/**
  * @brief  ���忪�ƺ���.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void beep_switch_on(void* p)
{
  HAL_GPIO_WritePin(BEEP_PORT,BEEP_PIN, BEEP_STATE);
}
/**
  * @brief  ����صƺ���.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void beep_switch_off(void* p)
{
  HAL_GPIO_WritePin(BEEP_PORT,BEEP_PIN,GPIO_TURN(BEEP_STATE));
}
/**
  * @brief  �ص�����.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void beep_over_callback(led_t *led_handler)
{
  led_set_mode(led_handler, LOOP_PERMANENT, beep_blink_mode_off);
  led_start(led_handler);
}
#endif
/*************************LED**************************************************/
//�����źŵƶ�����
led_t *led =  NULL;
/*  �����źŵ�һ�������ڵ���˸ģʽ
 *  ��ʽΪ �������������������� ���������� ���Ȳ���
 *  ע�⣺  �����õ�λΪ���룬�ұ������ ��LED_TICK_TIME�� �꣬��Ϊ����������Ϊ������������ȡ������
 *          ������Ӣ�Ķ���Ϊ���������Ӣ�Ķ��Ž�β���ַ�����ֻ���������ּ����ţ������������ַ�����
 */
char *led_blink_mode_0 = "500,500,";                        //1Hz��˸
char *led_blink_mode_1 = "200,200,100,500";                 //������˸����
char *led_blink_mode_2 = "200,200,200,200,200,500";         //������˸����
char *led_blink_mode_3 = "200,200,200,200,200,200,200,500"; //�Ĵ���˸����
char *led_blink_mode_off = "0,100,";   //����
char *led_blink_mode_on = "100,0,";    //����
/**
  * @brief  �û�����ģʽ
  * @param  None.
  * @retval None.
  * @note   -1��û�д�������
  ����ʱ�� =  ���� *ģʽʱ�䣻ms
*/
static void User_Led_Setmode(LED_MODE mode)
{
  switch(mode)
  {
    case NORMAL_MODE://����
      led_set_mode(led, LOOP_PERMANENT, led_blink_mode_0);
      break;
    case IPC_BEAT_MODE:   //�����쳣
       led_set_mode(led, LOOP_PERMANENT, led_blink_mode_1);
      break;
    case MOTOR_ALM_MODE://��������쳣
       led_set_mode(led, LOOP_PERMANENT, led_blink_mode_2);
      break;
    case VOLTAGE_MODE://��ѹ�쳣���¶��쳣��
       led_set_mode(led, LOOP_PERMANENT, led_blink_mode_3);
      break;
    default:
      break;
  }
  led_start(led);
}
/**
  * @brief  ���忪�ƺ���.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void led_switch_on(void* p)
{
  HAL_GPIO_WritePin(LED_PORT,LED_PIN, BRIGHT_STATE);
}
/**
  * @brief  ����صƺ���.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void led_switch_off(void* p)
{
  HAL_GPIO_WritePin(LED_PORT,LED_PIN,GPIO_TURN(BRIGHT_STATE));
}
/**
  * @brief  �ص�����.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void blink_over_callback(led_t *led_handler)
{
    led_set_mode(led_handler, LOOP_PERMANENT, led_blink_mode_0);
    led_start(led_handler);
}
/**
  * @brief  LED�����߳�
  * @param  None
  * @retval None
  * @note   
*/
static void led_run(void *parameter)
{
    while(1)
    {
      led_ticks();
      rt_thread_mdelay(LED_TICK_TIME);
    }
}
/**
  * @brief  LED��ʼ��
  * @param  None
  * @retval None
  * @note   
*/
static int rt_led_timer_init(void)
{
/*�Զ����ڴ�����ӿ�
 *ע�⣺��Ҫ�����Զ����ڴ����������Ҫ�ڵ����κ�������ڽӿ�֮ǰ�����ã�
 *      �������ֲ������ϵĴ��󣡣���
 */
    led_mem_opreation.malloc_fn = (void* (*)(size_t))rt_malloc;
    led_mem_opreation.free_fn = rt_free;
    led_set_mem_operation(&led_mem_opreation);
    
    //��ʼ���źŵƶ���
    led = led_create(led_switch_on, led_switch_off, NULL);
    User_Led_Setmode(NORMAL_MODE);
//    //�����źŵ���˸�����ص�����
//    led_set_blink_over_callback(led,blink_over_callback);
    //�����źŵ�
    led_start(led);
#if (BEEP_ENABLE == 1)
    //��ʼ������������
    beep = led_create(beep_switch_on, beep_switch_off, NULL);
    led_set_mode(beep, 1, beep_blink_mode_0);
    //�����źŵ���˸�����ص�����
    led_set_blink_over_callback(beep,beep_over_callback);
    //�����źŵ�
    led_start(beep);
#endif
    rt_thread_t tid = RT_NULL;
    tid = rt_thread_create("signal_led",
                            led_run, 
                            RT_NULL,
                            512,
                            RT_THREAD_PRIORITY_MAX/2,
                            100);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(rt_led_timer_init);
/**
  * @brief  �쳣���
  * @param  None.
  * @retval None.
  * @note   --led��˸+����������.
*/
void LED_Abnormal_Alarm(void)
{
  static LED_MODE last = NORMAL_MODE;
  if(TURN_ALARM_GET == 1 || Walk_ALARM_GET == 1 || CRASH_ALARM_GET == 1)//������� ������˸����
  {
    led_alm_state = MOTOR_ALM_MODE;
  }
  else if((IPC_Beat.flag == false  && IPC_Beat.EN == 1) 
  || (Weinview_Beat.flag == false  && Weinview_Beat.EN == 1))//���ػ������쳣 ������˸����
  {
    led_alm_state = IPC_BEAT_MODE;
  }
  else if(ADC_ALARM_GET == 1)//��ѹ���� �Ĵ���˸����
  {
    led_alm_state = VOLTAGE_MODE;
  }
  else
  {
    led_alm_state = NORMAL_MODE;
  }
  if(last != led_alm_state)
  {
    User_Led_Setmode(led_alm_state);
#if (BEEP_ENABLE == 1)
    User_BEEP_Setmode(led_alm_state);
#endif
    last = led_alm_state;
  }
}