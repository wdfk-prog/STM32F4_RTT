/**
 * @file board.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-18
 * @copyright Copyright (c) 2022
 * @attention 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-18 1.0     HLY     first version
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "board.h"
/* Private includes ----------------------------------------------------------*/
#include <time.h>
#include "main.h"
#include "modbus_slave_common.h"
#include "filesystem.h"
#ifdef RT_USING_SERIAL
#ifdef RT_USING_SERIAL_V2
#include "drv_usart_v2.h"
#else
#include "drv_usart.h"
#endif /* RT_USING_SERIAL */
#endif /* RT_USING_SERIAL_V2 */
/*ulog include*/
#define LOG_TAG              "board" 
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define RTC_NAME       "rtc"
/*�����ж����ȼ�����*/
#define FINSH_IRQ_PRIORITY 3
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
#ifdef FINSH_IRQ_PRIORITY
/**
  * @brief  ����FINSH�����ж����ȼ�
  * @param  None
  * @retval None
  */
static int set_finsh_irq(void)
{
  rt_err_t ret = RT_EOK;
  /* �����豸��� */
  rt_device_t serial;
  /* ���Ҵ����豸 */
  serial = rt_device_find(RT_CONSOLE_DEVICE_NAME);
  if (!serial)
  {
      LOG_E("find %s failed!", RT_CONSOLE_DEVICE_NAME);
      ret = -RT_ERROR;
  }
  struct stm32_uart *uart;

  rt_device_control(serial,RT_DEVICE_CTRL_SET_INT_PRIORITY,(void *)FINSH_IRQ_PRIORITY);
  return ret;
}
INIT_COMPONENT_EXPORT(set_finsh_irq);
#endif /*FINSH_IRQ_PRIORITY*/
/**
 * @brief  ��ȡ����ʱ���
 * @param  None.              
 * @retval time_t ����ʱ���
 * @note   None.
 */
static time_t rtc_get_build(void)
{
  struct tm tm_new = {0};
  /* update date. */
  tm_new.tm_year  = YEAR - 1900;// years since 1900
  tm_new.tm_mon   = MONTH;//tm_mon: 0~11
  tm_new.tm_mday  = DAY;
  /* update time. */
  tm_new.tm_hour = HOUR;
  tm_new.tm_min  = MINUTE;
  tm_new.tm_sec  = SEC;

  return mktime(&tm_new);   
}
/**
 * @brief  ����rtcʱ��
 * @param  t                
 * @retval time_t ����ʱ���
 * @note   ��δ��ȡ��ʱ�䣬���� 0;
 */
static time_t rtc_update(void)
{
  struct tm tm_new = {0};
  /* update date. */
  tm_new.tm_year  = *mb_tm.year - 1900;//years since 1900
  tm_new.tm_mon   = *mb_tm.mon  - 1;   //tm_mon: 0~11
  tm_new.tm_mday  = *mb_tm.mday;
  /* update time. */
  tm_new.tm_hour = *mb_tm.hour;
  tm_new.tm_min  = *mb_tm.min;
  tm_new.tm_sec  = *mb_tm.sec;
  /* converts the local time into the calendar time. */
  if(tm_new.tm_mon < 0 || !(tm_new.tm_year + tm_new.tm_mday + tm_new.tm_hour + tm_new.tm_min + tm_new.tm_sec))
    return 0;
  else
  {
    return mktime(&tm_new);
  }     
}
/**
  * @brief  rtc_update�߳�.
  * @param  None.
  * @retval None.
  * @note   1���ȡһ��ͬ��ʱ�䡣
*/
static void rtc_update_thread_entry(void* parameter)
{
  time_t old_time = 0,new_time = 0,err_time = 0;

  while(1)
  {
    rt_thread_mdelay(1000);
    new_time = rtc_update();

    err_time = new_time - old_time;
    if(err_time == 0)//ʱ��ֵû�и���
    {
      ulog_d("RTC","Out of time synchronization");
    }
    else
    {
      set_timestamp(new_time);//����ʱ�䣬����ͬ��
      
    }
    rtc_time_write();
    old_time = new_time;
  }
}
/**
 * @brief   rtc_update_init
 * @retval  int 
 * @note    ��ʼ������rtcʱ��
 */
int rtc_update_init(void)
{
    rt_thread_t tid;
    rt_err_t ret = RT_EOK;
    time_t now,build;


    rt_device_t device = RT_NULL;
    /*Ѱ���豸*/
    device = rt_device_find(RTC_NAME);
    if (!device)
    {
        LOG_E("find %s failed!", RTC_NAME);
        return RT_ERROR;
    }
    /*��ʼ��RTC�豸*/
    if(rt_device_open(device, 0) != RT_EOK)
    {
        LOG_E("open %s failed!", RTC_NAME);
        return RT_ERROR;
    }
    
    build = rtc_get_build();//��ȡ����ʱ��
    now = rtc_time_read();  //��ȡflash����ʱ��

    if(build < now)
    {
      set_timestamp(now);
      LOG_I("Set the timestamp to the time in the flash");
    }
    else
    {
      set_timestamp(build);
      LOG_I("Set the timestamp to compile time");
    }

    tid = rt_thread_create("rtc", rtc_update_thread_entry, RT_NULL,
                          4096, 29, 0);
    if(tid == RT_NULL)
    {
      LOG_E("rtc thread start failed!");
    }
    else
    {
      rt_thread_startup(tid);
    }
    return RT_EOK;
}
/*********************************������******************************************/
#if (PVD_ENABLE == 1)
/* ��������ƿ� */
static struct rt_completion pvd_completion;
/**
  * @brief  None.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void pvd_thread_entry(void* parameter)
{
  while(1)
  {
      rt_completion_wait(&pvd_completion, RT_WAITING_FOREVER);
      /* ����ǰ�Ľ������� */
      ulog_flush();
      rt_kprintf("Flush ULOG buffer complete\n");
  }
}
/**
 * @brief pvd��ʼ��
 * @retval int 
 */
static int pvd_init(void)
{
    /*##-1- Enable Power Clock #################################################*/
    __HAL_RCC_PWR_CLK_ENABLE();           /* ʹ��PVD */
 
    /*##-2- Configure the NVIC for PVD #########################################*/
    HAL_NVIC_SetPriority(PVD_IRQn, 0, 0); /* ����PVD�ж����ȼ� */
    HAL_NVIC_EnableIRQ(PVD_IRQn);         /* ʹ��PVD�ж� */
 
    /* Configure the PVD Level to 3 and generate an interrupt on rising and falling
       edges(PVD detection level set to 2.5V, refer to the electrical characteristics
       of you device datasheet for more details) */
    PWR_PVDTypeDef sConfigPVD;
    sConfigPVD.PVDLevel = PWR_PVDLEVEL_6;     /* PVD��ֵ3.1V */
    sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING; /* ������ */
    HAL_PWR_ConfigPVD(&sConfigPVD);
 
    /* Enable the PVD Output */
    HAL_PWR_EnablePVD();

    /* ��ʼ����������� */
    rt_completion_init(&pvd_completion);
    rt_thread_t tid;
    tid = rt_thread_create("PVD", pvd_thread_entry, RT_NULL,
                          512, 0, 20);
    if(tid == RT_NULL)
    {
      LOG_E("PVD thread start failed!");
    }
    else
    {
      rt_thread_startup(tid);
    }
    return RT_EOK;
}
INIT_APP_EXPORT(pvd_init);
/**
  * @brief  PWR PVD interrupt callback
  * @retval None
  */
void HAL_PWR_PVDCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_PWR_PVDCallback could be implemented in the user file
   */
   if(__HAL_PWR_GET_FLAG( PWR_FLAG_PVDO ))    /* 1ΪVDDС��PVD��ֵ,������� */
  {
      rt_completion_done(&pvd_completion);
      ulog_i("PVD","Voltage below 3.1V was detected");
  }
}
#endif /*(PVD_ENABLE == 1)*/