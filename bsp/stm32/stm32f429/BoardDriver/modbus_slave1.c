/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_savle1.c
  * @brief          : 
  * @date           : 2022.08.10
  ******************************************************************************
  * @attention  
  MB_SLAVE1 <-��д-> |           |  ʹ�û����������ƻ�
 ʹ�û����������ƻ�  | MB_Buffer |  <-��д-> MB_Handler
  MB_SLAVE2 <-��д-> |           |
  * @author HLY
  ******************************************************************************
  */
/* USER CODE END Header */
#include "mb.h"
#include "mbrtu.h"
#include "mbevent.h"
/* Private includes ----------------------------------------------------------*/
#include "sys.h"
#include <rtthread.h>
#include <drv_usart_v2.h>
#include "mb_handler.h"
#include "main.h"

#define UART_NAME       "uart1"      /* �����豸���� */
/*ulog include*/
#define LOG_TAG         UART_NAME
#define LOG_LVL         DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/
/* ���ڽ�����Ϣ�ṹ */
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};
/* Private define ------------------------------------------------------------*/
/*�˿ڶ���*/
#define ucRTU_DCB	     (mbrtu1)
#define ucMB_RTU_PORT	 (mbrtuport1)
/*����������*/
#define BAUD_RATE BAUD_RATE_115200
/*�ӻ���ַ����*/
#define SLAVE_ADDRESS 0X01
/*�����ж����ȼ�����*/
#define IRQ_PRIORITY  3
/* Private macro -------------------------------------------------------------*/
/* �߳����� */
#define THREAD_PRIORITY      9//�߳����ȼ�
#define THREAD_TIMESLICE     10//�߳�ʱ��Ƭ
#define THREAD_STACK_SIZE    1024//ջ��С
/* Private variables ---------------------------------------------------------*/
static int serial_receive(uint8_t *buf, int bufsz);
/* �����豸��� */
static rt_device_t serial = RT_NULL;
/* ��Ϣ���п��ƿ� */
static rt_mq_t rx_mq      = RT_NULL;
/* ָ�򻥳�����ָ�� */
rt_mutex_t mb_slave_mutex = RT_NULL;
/* MODBUS���� */
static void MBPortSerialInit(ULONG ulBaudRate, eMBParity eParity);
static void MBPortTimersInit( USHORT usTim1Timerout50us );
static void MBPortSerialGetByte(UCHAR *pucByte);
static void MBPortSerialPutByte(UCHAR ucByte);
static void MBPortTimersEnable(USHORT Timerout50us);
static void MBPortTimersDisable(void);
static void MBPortSerialEnable(BOOL xRxEnable,BOOL xTxEnable);
static void EnterCriticalSection(void);
static void ExitCriticalSection(void);
/* MODBUS�ṹ�� */
static _CONST MB_RTU_PORT mbrtuport1 = 
{
	MBPortSerialInit,
	MBPortTimersInit,
	MBPortSerialGetByte,
	MBPortSerialPutByte,
	MBPortTimersEnable,
	MBPortTimersDisable,
	MBPortSerialEnable,
	EnterCriticalSection,
	ExitCriticalSection,
};
/* MODBUS�ṹ��ָ�� */
static MB_RTU_DCB mbrtu1;
/* Private function prototypes -----------------------------------------------*/
/**
 * This function will print a formatted string on system console.
 *
 * @param fmt is the format parameters.
 *
 * @return The number of characters actually written to buffer.
 */
static int kprintf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;

    rt_device_write(serial, 0, rt_log_buf, length);
    va_end(args);
    return length;
}
/**
  * @brief  ATָ���
  * @param  None
  * @retval None
  * @note   ����AT������OK
            ���ͺ��������顣û�з���Ӱ��
*/
static void AT(void)
{
  if(rt_strstr((char *)ucRTU_DCB.ucBuffer,"AT\r\n") != NULL)
  {
    rt_memset((void *)ucRTU_DCB.ucBuffer, 0, sizeof ucRTU_DCB.ucBuffer);
    kprintf("\n \\ | /\n");
    kprintf("- HLY -    Version F4_Laser_forklif V%s \n",VERSION);
    kprintf(" / | \\     build %s %s\n",__DATE__,__TIME__);
    kprintf("device is %s",UART_NAME);
  }
}
/**
  * @brief  MODBUS�߳�
  * @param  p:���ã������崴���߳��о���
  * @retval None.
  * @note   None.
*/
static void modbus_thread(void* p)
{
  //��ʼ��modbusRTU����վ��ַΪ0x01,������Ϊ115200����У�顣
  eMBRTUInit(&ucRTU_DCB,&ucMB_RTU_PORT,SLAVE_ADDRESS,BAUD_RATE,MB_PAR_NONE); 
  eMBRTUStart(&ucRTU_DCB);
  //F4ʹ�ܿ����жϲ�����������жϺ�������״̬ת�����ֶ��л�
  ucRTU_DCB.eRcvState = STATE_RX_IDLE;
  while(1)
  {
    struct rx_msg msg;
    rt_memset(&msg, 0, sizeof(msg));
    /* ����Ϣ�����ж�ȡ��Ϣ */
    rt_err_t result = rt_mq_recv(rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        /* �Ӵ��ڶ�ȡ���� */
        rt_uint32_t rx_length = rt_device_read(msg.dev, 0,(CHAR *)ucRTU_DCB.ucBuffer, msg.size);
        if(rx_length != msg.size)
        {
          LOG_W("Incorrect receive length");
          continue;
        }
        else
        {
          ucRTU_DCB.ucBufferCount = rx_length;              //д���ȡ��С
          xMBPortEventPost(&ucRTU_DCB,EV_FRAME_RECEIVED);   //�ı��¼�״̬Ϊ�����¼�
          rt_mutex_take(mb_slave_mutex, RT_WAITING_FOREVER);//�����ڴ�
          AT();
          Modbus_Handler();//��ȡ
          eMBRTUPoll(&ucRTU_DCB);                           //��д�ڴ�
          Modbus_Handler();//д��
          HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
          rt_mutex_release(mb_slave_mutex);                 //��������ڴ�
        }
    }
  }
}
/*******************************************�����շ�������*********************************************************************/
/**
 * @brief This function will set the reception indication callback function. This callback function
 *        is invoked when this device receives data.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param rx_ind is the indication callback function.
 *
 * @return RT_EOK
 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(rx_mq, &msg, sizeof(msg));
    if (result == -RT_EFULL)
    {
        /* ��Ϣ������ */
        LOG_E("message queue full!");
    }
    return 1;
}
/**
  * @brief  ���ڽ��պ���.
  * @param  buf:���ջ���
  * @param  bufsz:���ջ����С
  * @retval ���ճ��ȡ�-1��ʾ����ʧ��
  * @note   ��Ϣ�����������գ�һֱ�ȴ���Ϣ����
*/
static int serial_receive(uint8_t *buf, int bufsz)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* ����Ϣ�����ж�ȡ��Ϣ */
        result = rt_mq_recv(rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            /* �Ӵ��ڶ�ȡ���� */
            rx_length = rt_device_read(msg.dev, 0, buf, msg.size);
            /* ��ӡ���� */
//            LOG_HEX(UART_NAME,8,buf,rx_length);
            return rx_length;
        }
    }
}
/**
  * @brief  ���ڷ��ͺ���..
  * @param  buf:���ͻ���
  * @param  bufsz:���ͻ����С
  * @retval �ɹ���1��ʧ�ܣ�0
  * @note   None.
*/
static int serial_send(uint8_t *buf, int len)
{
  rt_size_t send_len = 0; 
  /* ͨ�������豸 serial �����ȡ������Ϣ */
  send_len = rt_device_write(serial, 0, buf, len);
  if(send_len == len)
  {
    return 1;
  }
  else
  {
    LOG_E("send failed");
    return 0;
  }
}
/**
  * @brief  modbus �ӻ�1 ��ʼ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
static int Modbus_Slave1_Init(void)
{
    rt_err_t ret = RT_EOK;
    struct rx_msg msg;
    /* ��ʼ����Ϣ���� */
    rx_mq = rt_mq_create("mbs1ave1",
                         sizeof(struct rx_msg),    /* һ����Ϣ����󳤶� */
                         BSP_UART1_RX_BUFSIZE,     /* �����Ϣ�Ļ�������С */
                         RT_IPC_FLAG_FIFO); 

    /* ���Ҵ����豸 */
    serial = rt_device_find(UART_NAME);
    if (!serial)
    {
        rt_mq_detach(rx_mq);
        LOG_E("find %s failed!", UART_NAME);
        return RT_ERROR;
    }
               
    /* ����һ����̬������ */
    mb_slave_mutex = rt_mutex_create("mb slave", RT_IPC_FLAG_PRIO);
    if (mb_slave_mutex == RT_NULL)
    {
        LOG_E("create mb_slave_mutex failed.");
        return -1;
    }
    
    /* ���� MODBUS�ӻ��߳�*/
    rt_thread_t thread = rt_thread_create( UART_NAME,         /* �߳����� */
                                           modbus_thread,     /* �߳���ں��� */
                                           RT_NULL,           /* �߳���ں������� */
                                           THREAD_STACK_SIZE, /* �߳�ջ��С */
                                           THREAD_PRIORITY,   /* �̵߳����ȼ� */
                                           THREAD_TIMESLICE); /* �߳�ʱ��Ƭ */
    /* �����ɹ��������߳� */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
        LOG_E("modbus slave created failed.");
    }

    return ret;
}
/* ������ msh �����б��� */
INIT_COMPONENT_EXPORT(Modbus_Slave1_Init);
/*******************************************FREEMODBUS�ӿں���*************************/
/**
  * @brief  MB���ڳ�ʼ��
  * @param  �����ʡ���żУ��
  * @retval None
  * @note   
  */
static void MBPortSerialInit(ULONG ulBaudRate, eMBParity eParity)
{
    /* ���Ҵ����豸 */
    serial = rt_device_find(UART_NAME);
    if (!serial)
    {
        rt_mq_detach(rx_mq);
        LOG_E("find %s failed!", UART_NAME);
    }
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */
    /* step2���޸Ĵ������ò��� */
    config.baud_rate = ulBaudRate;            // �޸Ĳ�����
    config.data_bits = DATA_BITS_8;           // ����λ 8
    config.stop_bits = STOP_BITS_1;           // ֹͣλ 1
    config.rx_bufsz  = BSP_UART1_RX_BUFSIZE;  // �޸Ļ����� rx buff size Ϊ 128
    config.tx_bufsz  = BSP_UART1_TX_BUFSIZE;  // �޸Ļ����� rx buff size Ϊ 128
    switch(eParity)
    {
      case MB_PAR_ODD:// ��У��
          config.parity    = PARITY_ODD;
          config.data_bits = DATA_BITS_9;//����У������λΪ9bits
          break;
      case MB_PAR_EVEN://żУ��
          config.parity    = PARITY_EVEN;
          config.data_bits = DATA_BITS_9;//��żУ������λΪ9bits
          break;
      default://��У��
          config.parity    = PARITY_NONE;
          config.data_bits = DATA_BITS_8;//����żУ������λΪ8bits
          break;
    }
    /* step3�����ƴ����豸��ͨ�����ƽӿڴ�����������֣�����Ʋ��� */
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    /* �� DMA ���ռ���ѯ���ͷ�ʽ�򿪴����豸 */
    rt_device_open(serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_NON_BLOCKING);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_input);
#ifdef IRQ_PRIORITY
    struct stm32_uart *uart;
    uart = rt_container_of(serial, struct stm32_uart, serial);
    /* parameter check */
    RT_ASSERT(uart != RT_NULL);
    HAL_NVIC_SetPriority(uart->config->irq_type,IRQ_PRIORITY, 0);
#endif
}
/**
  * @brief  MB��ʱ����ʼ��
  * @param  ��ʱʱ��
  * @retval None
  * @note   �ֿ�ʹ�ò�ͬ��ʱ�������׵��ڲ�����
            �̶�ʹ�ò����ʣ�����һ����ʱ�������ñȽ�ʱ��ģʽ
  */
static void MBPortTimersInit( USHORT usTim1Timerout50us )
{

}
/**
  * @brief  MB���ڶ�ȡһ������
  * @param  ��ȡ��ַ
  * @retval None
  * @note   ����FIFO+DMA��
            ʵ�ʴ�FIFO�����ж�ȡ
  */
static void MBPortSerialGetByte(UCHAR *pucByte)
{

}
/**
  * @brief  MBд�봮��һ������
  * @param  ����ֵ
  * @retval None
  * @note   ����FIFO+DMA��
            ʵ�ʷ�����FIFO������
  */
static void MBPortSerialPutByte(UCHAR ucByte)
{
   rt_device_write(serial, 0,(uint8_t *)ucRTU_DCB.pucBuffer,ucRTU_DCB.ucBufferCount);
}
/**
  * @brief  MB��ʱ��ʹ��
  * @param  ��ʱʱ��
  * @retval None
  * @note   ����19200���̶�3.5US*50=1750US
            ������м���
  */
static void MBPortTimersEnable(USHORT Timerout50us)
{
}
/**
  * @brief  MB��ʱ������
  * @param  None
  * @retval None
  * @note   None
  */
static void MBPortTimersDisable(void)
{

}
/**
  * @brief  MB���ڷ��ͽ���ʹ�ܡ�����
  * @param  None
  * @retval None
  * @note   None
  */
static void MBPortSerialEnable(BOOL xRxEnable,BOOL xTxEnable)
{
  
}
/**
  * @brief  �����ж�
  * @param  None
  * @retval None
  * @note   None
  */
static void EnterCriticalSection(void)
{

}
/**
  * @brief  �˳��ж�
  * @param  None
  * @retval None
  * @note   None
  */
static void ExitCriticalSection(void)
{

}