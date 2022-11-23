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
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ���Դ����������.
  * @param  None.
  * @retval None.
  * @note   01 06 01 F3 12 34 75 72
*/
static void debug_beat_monitor(void)
{
  uint16_t beat = *debug_beat.value;
  if(USER_GET_BIT(beat,15) == true)
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
    *debug_beat.value = 0;
  }
  else
  {
    *debug_beat.flag = true;
    if(debug_beat.button != 2)
    {
      debug_beat.button  = 2;
    }
  }
}
/**
  * @brief  �������.
  * @param  None.
  * @retval None.
  * @note   1.2s���һ��
*/
static void beat_monitor(void *p)
{
  debug_beat_monitor();
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

  return RT_EOK;
}