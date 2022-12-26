/**
 * @file user_can.c
 * @brief CAN����ʹ��
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @copyright Copyright (c) 2022
 * @attention 
 * @par �޸���־:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     first version
 */
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include <rtthread.h>
#include "rtdevice.h"

#define CAN_DEV_NAME        "can1"      /* CAN �豸���� */
/*ulog include*/
#define LOG_TAG             CAN_DEV_NAME
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* ����߳����� */
#define THREAD_PRIORITY      12  //�߳����ȼ�
#define THREAD_TIMESLICE     10  //�߳�ʱ��Ƭ
#define THREAD_STACK_SIZE    1024//ջ��С
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static struct rt_semaphore rx_sem;     /* ���ڽ�����Ϣ���ź��� */
static rt_device_t can_dev;            /* CAN �豸��� */
/**
  * @brief  �������ݻص�����
  * @param  None
  * @retval None
  * @note   
*/
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN ���յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}
/**
  * @brief  can�����߳�
  * @param  None
  * @retval None
  * @note   
*/
static void can_rx_thread(void *parameter)
{
    int i;
    rt_err_t res;
    struct rt_can_msg rxmsg = {0};

    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(can_dev, can_rx_call);

#ifdef RT_CAN_USING_HDR
    struct rt_can_filter_item items[5] =
    {
        RT_CAN_FILTER_ITEM_INIT(0x100, 0, 0, 0, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x100~0x1ff��hdr Ϊ - 1������Ĭ�Ϲ��˱� */
        RT_CAN_FILTER_ITEM_INIT(0x300, 0, 0, 0, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x300~0x3ff��hdr Ϊ - 1 */
        RT_CAN_FILTER_ITEM_INIT(0x211, 0, 0, 0, 0x7ff, RT_NULL, RT_NULL), /* std,match ID:0x211��hdr Ϊ - 1 */
        RT_CAN_FILTER_STD_INIT(0x486, RT_NULL, RT_NULL),                  /* std,match ID:0x486��hdr Ϊ - 1 */
        {0x555, 0, 0, 0, 0x7ff, 7,}                                       /* std,match ID:0x555��hdr Ϊ 7��ָ������ 7 �Ź��˱� */
    };
    struct rt_can_filter_config cfg = {5, 1, items}; /* һ���� 5 �����˱� */
    /* ����Ӳ�����˱� */
    res = rt_device_control(can_dev, RT_CAN_CMD_SET_FILTER, &cfg);
    RT_ASSERT(res == RT_EOK);
#endif /*RT_CAN_USING_HDR*/

    while (1)
    {
        /* hdr ֵΪ - 1����ʾֱ�Ӵ� uselist �����ȡ���� */
        rxmsg.hdr_index = -1;
        /* �����ȴ������ź��� */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        /* �� CAN ��ȡһ֡���� */
        rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));
        /* ��ӡ���� ID ������ */
        rt_kprintf("\n");
        rt_kprintf("ID:%x:", rxmsg.id);
        for (i = 0; i < 8; i++)
        {
            rt_kprintf("%2x", rxmsg.data[i]);
        }

        rt_kprintf("\n");
    }
}
/**
  * @brief  CAN��ʼ������
  * @param  None
  * @retval None
  * @note   
*/
int can1_init(void)
{
    struct rt_can_msg msg = {0};
    rt_err_t res;
    rt_size_t  size;
    /* ���� CAN �豸 */
    can_dev = rt_device_find(CAN_DEV_NAME);
    if (!can_dev)
    {
        LOG_E("find %s failed", CAN_DEV_NAME);
        return RT_ERROR;
    }

    /* ��ʼ�� CAN �����ź��� */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    /* ���жϽ��ռ����ͷ�ʽ�� CAN �豸 */
    res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);
    /* �����߳� ������� ���ⶨʱ����Ϊ��ʼ��*/
    rt_thread_t thread = rt_thread_create( "can_rx",           /* �߳����� */
                                           can_rx_thread,  /* �߳���ں��� */
                                           RT_NULL,               /* �߳���ں������� */
                                           THREAD_STACK_SIZE,     /* �߳�ջ��С */
                                           THREAD_PRIORITY,       /* �̵߳����ȼ� */
                                           THREAD_TIMESLICE);     /* �߳�ʱ��Ƭ */
    /* �����ɹ��������߳� */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("Monitor thread created failed.");
        res =  RT_ERROR;
    }

    msg.id = 0x78;              /* ID Ϊ 0x78 */
    msg.ide = RT_CAN_STDID;     /* ��׼��ʽ */
    msg.rtr = RT_CAN_DTR;       /* ����֡ */
    msg.len = 8;                /* ���ݳ���Ϊ 8 */
    /* �����͵� 8 �ֽ����� */
    msg.data[0] = 0x00;
    msg.data[1] = 0x11;
    msg.data[2] = 0x22;
    msg.data[3] = 0x33;
    msg.data[4] = 0x44;
    msg.data[5] = 0x55;
    msg.data[6] = 0x66;
    msg.data[7] = 0x77;
//    /* ����һ֡ CAN ���� */
//    size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
//    if (size == 0)
//    {
//        LOG_E("can dev write data failed!");
//    }

    return res;
}
//INIT_APP_EXPORT(can1_init);
/**
  * @brief  CAN���Ͳ���
  * @param  None
  * @retval None
  * @note   
*/
void can_send_test(void)
{
    struct rt_can_msg msg = {0};
    rt_size_t  size;
    static rt_uint8_t num = 0;
    /* ���� CAN �豸 */
    can_dev = rt_device_find(CAN_DEV_NAME);
    if (!can_dev)
    {
        LOG_E("find %s failed", CAN_DEV_NAME);
    }
    msg.id = 0x78;              /* ID Ϊ 0x78 */
    msg.ide = RT_CAN_STDID;     /* ��׼��ʽ */
    msg.rtr = RT_CAN_DTR;       /* ����֡ */
    msg.len = 8;                /* ���ݳ���Ϊ 8 */

    /* �����͵� 8 �ֽ����� */
    msg.data[0] = 0x00;
    msg.data[1] = num++;
    msg.data[2] = 0x22;
    msg.data[3] = 0x33;
    msg.data[4] = num++;
    msg.data[5] = 0x55;
    msg.data[6] = 0x66;
    msg.data[7] = 0x77;
    /* ����һ֡ CAN ���� */
    size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
    if (size == 0)
    {
        rt_kprintf("can dev write data failed!\n");
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(can_send_test, can send test);