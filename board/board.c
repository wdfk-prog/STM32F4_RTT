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
#include "board.h"
/* Private includes ----------------------------------------------------------*/
#include "main.h"

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
//#define FINSH_IRQ_PRIORITY  
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
#ifdef FINSH_IRQ_PRIORITY
/**
  * @brief  ����FINSH�����ж����ȼ�
  * @param  None
  * @retval None
  */
static int Set_FINSH_IRQ(void)
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

  uart = rt_container_of(serial, struct stm32_uart, serial);
  /* parameter check */
  RT_ASSERT(uart != RT_NULL);
  HAL_NVIC_SetPriority(uart->config->irq_type,FINSH_IRQ_PRIORITY, 0);
  return ret;
}
INIT_COMPONENT_EXPORT(Set_FINSH_IRQ);
#endif
/**
  * @brief  ����RTCʱ��
  * @param  None
  * @retval ulogʱ���׼ȷ
  */
static int Set_RTC_Time(void)
{
#include <time.h>
  rt_err_t ret = RT_EOK;
  time_t now;

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

  /* �������� */
  ret = set_date(YEAR, MONTH + 1, DAY);
  if (ret != RT_EOK)
  {
    LOG_E("set RTC date failed");
  }
  /* ����ʱ�� */
  ret = set_time(HOUR, MINUTE, SEC + BURN_TIME);
  if (ret != RT_EOK)
  {
    LOG_E("set RTC time failed");
  }
  return ret;
}
INIT_COMPONENT_EXPORT(Set_RTC_Time);
/*********************************������******************************************/
#ifdef PVD_ENABLE
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
static int PVD_Init(void)
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
INIT_APP_EXPORT(PVD_Init);
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
#endif