/**
 * @file monitor.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-23
 * @copyright Copyright (c) 2022
 * @attention 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-23 1.0     HLY     first version
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "monitor.h"
/* Private includes ----------------------------------------------------------*/
#include <rtthread.h>
#include "user_math.h"
#include "motor.h"
#include <string.h>
/*ulog include*/
#define LOG_TAG              "Monitor"
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define BEAT_PERIOD_TIME 1000 //����������� ��λms
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
Beat_TypeDef debug_beat;
bool beat_enable = true;
/* Private function prototypes -----------------------------------------------*/
/**
 * @brief �����ص�����
 * @param  value      
 * @note   
 */
static void debug_beat_callback(uint8_t value)
{
  static uint16_t err_cnt = 0;
  if(value == true)
  {
    USER_CLEAR_BIT(*turn_motor[0].stop_state,BEAT_STOP);
    if(err_cnt)
    {
      LOG_I("Heartbeat communication recovery");
    }
    err_cnt = 0;
  }
  else
  {
    USER_SET_BIT(*turn_motor[0].stop_state,BEAT_STOP); 
    if(!(++err_cnt % (5000 / BEAT_PERIOD_TIME)) && err_cnt != 0)
    {
      LOG_W("Abnormal heartbeat,cnt = %d",err_cnt);
    }
  }
}
/**
  * @brief  ���Դ����������.
  * @param  None.
  * @retval None.
  * @note   01 06 01 F3 12 34 75 72
*/
static void debug_beat_monitor(void)
{
  uint16_t beat = *debug_beat.value;
 
  if(USER_GET_BIT(beat,7) == true)
  {
    beat = beat & 0X7F;//������λ
    if(beat != 0)
    {
      if(debug_beat.button == 0)
      {
        debug_beat.button = 1;
      }
      *debug_beat.flag = true;
    }
    else
    {
      *debug_beat.flag = false;
      debug_beat.button = 0;
    }
    *debug_beat.value = 0x80;
  }
  else
  {
    *debug_beat.flag = true;
    if(debug_beat.button != 2)
    {
      debug_beat.button  = 2;
    }
  }
  debug_beat_callback(*debug_beat.flag);
}
/**
  * @brief  �������.
  * @param  None.
  * @retval None.
  * @note   1.2s���һ��
*/
static void beat_monitor(void *p)
{
  if(beat_enable)
  {
    debug_beat_monitor();
  }
}
/**
  * @brief  ��غ�����ʼ��
  * @param  None
  * @retval None
  * @note   None
*/
int monitor_init(void)
{
  static rt_timer_t timer;
  /* ������ʱ�� 1  ���ڶ�ʱ�� */
  timer = rt_timer_create("beat", 
                          beat_monitor,
                          RT_NULL, rt_tick_from_millisecond(BEAT_PERIOD_TIME),
                          RT_TIMER_FLAG_PERIODIC);
  /* ������ʱ�� 1 */
  if (timer != RT_NULL)
  {
     rt_timer_start(timer);
  }
  else
  {
      LOG_E("beat timer init falsed");
  }
  return RT_EOK;
}
#ifdef RT_USING_MSH
/**
  * @brief  ������������
  * @param  None
  * @retval None
  * @note   None
*/

int beat_cmd(uint8_t argc, char **argv)
{
#define BEAT_CMD_ON                    0
#define BEAT_CMD_OFF                   1

  size_t i = 0;

  const char* help_info[] =
  {
    [BEAT_CMD_ON]             = "beat_cmd on  --beat on",
    [BEAT_CMD_OFF]            = "beat_cmd off --beat off",
  };

  if (argc < 2)
  {
      rt_kprintf("Usage:\n");
      for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
      {
          rt_kprintf("%s\n", help_info[i]);
      }
      rt_kprintf("\n");
  }
  else
  {
      const char *operator = argv[1];

      if (!strcmp(operator, "on"))
      {
        beat_enable = true;
      }
      else if (!strcmp(operator, "off"))
      {
        beat_enable = false;
      }
      else
      {
          rt_kprintf("Usage:\n");
          for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
          {
              rt_kprintf("%s\n", help_info[i]);
          }
          rt_kprintf("\n");
      }
  }
  return RT_EOK;
}
MSH_CMD_EXPORT(beat_cmd,beat_cmd);
/**
  * @brief  ֹͣ�����ѯ
  * @param  None
  * @retval None
  * @note   None
*/

int motor_get_stopcode(uint8_t argc, char **argv)
{
  rt_kprintf("Turn motor stop code is 0X%04X\n"  ,*turn_motor[0].stop_state);
  return RT_EOK;
}
MSH_CMD_EXPORT(motor_get_stopcode,motor stop code get);
#endif /*RT_USING_MSH*/