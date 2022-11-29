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
#include <rtthread.h>
#include <rtdevice.h>
/* Private includes ----------------------------------------------------------*/
#include "sys.h"
#include "Smove.h"
#include "turn_motor.h"
/*ulog include*/
#define LOG_TAG              "timer14" 
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/
struct stm32_hwtimer
{
    rt_hwtimer_t time_device;
    TIM_HandleTypeDef    tim_handle;
    IRQn_Type tim_irqn;
    char *name;
};
/* Private define ------------------------------------------------------------*/
#define HWTIMER_DEV_NAME   "timer14"     /* ��ʱ������ */
/* Private macro -------------------------------------------------------------*/
#define IRQ_PRIORITY  2
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ��ʱ����ʱ�ص�����
  * @param  None
  * @retval None
  * @note   None
*/
static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
    SMove_Handler(&turn.motor.Ctrl);
    return 0;
}
/**
  * @brief  ��ʱ����ʱ�ص�����
  * @param  None
  * @retval None
  * @note   None
*/
static int hwtimer14_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* ��ʱ����ʱֵ */
    rt_device_t hw_dev = RT_NULL;   /* ��ʱ���豸��� */
    rt_hwtimer_mode_t mode;         /* ��ʱ��ģʽ */
    rt_uint32_t freq = 1000000;     /* ����Ƶ�� */

    /* ���Ҷ�ʱ���豸 */
    hw_dev = rt_device_find(HWTIMER_DEV_NAME);
    if (hw_dev == RT_NULL)
    {
        LOG_E("hwtimer sample run failed! can't find %s device!", HWTIMER_DEV_NAME);
        return RT_ERROR;
    }

    /* �Զ�д��ʽ���豸 */
    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("open %s device failedn", HWTIMER_DEV_NAME);
        return ret;
    }

    /* ���ó�ʱ�ص����� */
    rt_device_set_rx_indicate(hw_dev, timeout_cb);

    /* ���ü���Ƶ��(��δ���ø��Ĭ��Ϊ1Mhz �� ֧�ֵ���С����Ƶ��) */
    rt_device_control(hw_dev, HWTIMER_CTRL_FREQ_SET, &freq);
    /* ����ģʽΪ�����Զ�ʱ������δ���ã�Ĭ����HWTIMER_MODE_ONESHOT��*/
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        LOG_E("set mode failed! ret is :%d", ret);
        return ret;
    }

    /* ���ö�ʱ����ʱֵΪ5s��������ʱ�� */
    timeout_s.sec  = 0;      /* �� */
    timeout_s.usec = 5;      /* ΢�� */
    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        LOG_E("set timeout value failed");
        return RT_ERROR;
    }
#ifdef IRQ_PRIORITY
    struct stm32_hwtimer *tim_device = RT_NULL;
    tim_device = rt_container_of(hw_dev, struct stm32_hwtimer, time_device);
    
    HAL_NVIC_SetPriority(tim_device->tim_irqn,IRQ_PRIORITY, 0);
#endif
    return ret;
}
/* ������ msh �����б��� */
INIT_DEVICE_EXPORT(hwtimer14_init);