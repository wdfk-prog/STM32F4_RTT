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
#include "rtthread.h"
#include <drv_usart_v2.h>
#include "modbus_slave_common.h"
/* Private includes ----------------------------------------------------------*/
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
/*����������*/
#define BAUD_RATE BAUD_RATE_115200
/*�ӻ���ַ����*/
#define SLAVE_ADDRESS 0X01
/*�����ж����ȼ�����*/
#define IRQ_PRIORITY  3

#define RegisterCount   10
/* Private macro -------------------------------------------------------------*/
/* �߳����� */
#define THREAD_PRIORITY      3//�߳����ȼ�
#define THREAD_TIMESLICE     10//�߳�ʱ��Ƭ
#define THREAD_STACK_SIZE    2048//ջ��С

//#define RS485_SLAVE_RE_PIN GET_PIN(E, 15)
//#define RS485_SLAVE_TX_EN() rt_pin_write(RS485_SLAVE_RE_PIN, PIN_HIGH)
//#define RS485_SLAVE_RX_EN() rt_pin_write(RS485_SLAVE_RE_PIN, PIN_LOW)
/* Private variables ---------------------------------------------------------*/
static rt_device_t serial = RT_NULL;
static rt_sem_t _rx_sem = RT_NULL;
static const agile_modbus_slave_util_t _slave_util = 
{
    bit_maps,
    sizeof(bit_maps) / sizeof(bit_maps[0]),
    input_bit_maps,
    sizeof(input_bit_maps) / sizeof(input_bit_maps[0]),
    register_maps,
    sizeof(register_maps) / sizeof(register_maps[0]),
    input_register_maps,
    sizeof(input_register_maps) / sizeof(input_register_maps[0]),
    addr_check,
    NULL,
    NULL
};
/* Private function prototypes -----------------------------------------------*/
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
    if (size > 0) 
    {
        rt_sem_release(_rx_sem);
    }
    return RT_EOK;
}
/**
  * @brief  ���ڷ��ͺ���..
  * @param  buf:���ͻ���
  * @param  len:���ͻ����С
  * @retval ���ͳ���
  * @note   None.
*/
static int serial_send(uint8_t *buf, int len)
{
//    RS485_SLAVE_TX_EN();
    rt_device_write(serial, 0, buf, len);
//    RS485_SLAVE_RX_EN();
    return len;
}
/**
  * @brief  ���ڳ�ʼ��.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static int serial_init(void)
{
    _rx_sem = rt_sem_create("rs485_slave", 0, RT_IPC_FLAG_FIFO);
    if(_rx_sem == RT_NULL)
    {
        LOG_E("create rs485_slave_sem failed.");
        return -RT_ERROR;
    }
    /*step1�����Ҵ����豸 */
    serial = rt_device_find(UART_NAME);
    if (!serial)
    {
        rt_sem_delete(_rx_sem);
        LOG_E("find %s failed!", UART_NAME);
        return -RT_ERROR;
    }
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */
    /* step2���޸Ĵ������ò��� */
    config.baud_rate = BAUD_RATE;             // �޸Ĳ�����
    config.data_bits = DATA_BITS_8;           // ����λ 8
    config.stop_bits = STOP_BITS_1;           // ֹͣλ 1
    config.rx_bufsz  = BSP_UART1_RX_BUFSIZE;  // �޸Ļ����� rx buff size
    config.tx_bufsz  = BSP_UART1_TX_BUFSIZE;  // �޸Ļ����� tx buff size
    /* step3�����ƴ����豸��ͨ�����ƽӿڴ�����������֣�����Ʋ��� */
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
//    rt_pin_mode(RS485_SLAVE_RE_PIN, PIN_MODE_OUTPUT);
//    RS485_SLAVE_TX_EN();
    /* step4�����ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_input);
    /* step5��  open serial device */
    rt_device_open(serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_NON_BLOCKING);
#ifdef IRQ_PRIORITY
    struct stm32_uart *uart;
    uart = rt_container_of(serial, struct stm32_uart, serial);
    /* parameter check */
    RT_ASSERT(uart != RT_NULL);
    HAL_NVIC_SetPriority(uart->config->irq_type,IRQ_PRIORITY, 0);
#endif    
//    RS485_SLAVE_RX_EN();
    return RT_EOK;
}
/**
  * @brief  ���ڽ��մ�����.
  * @param  buf��������ָ��
  * @param  bufsz:��������С
  * @param  timeout.δ���ӳ�ʱʱ��
  * @param  bytes_timeout ���Ӻ�ʱʱ��.
  * @retval None.
  * @note   None.
*/
int serial_receive(uint8_t *buf, int bufsz, int timeout, int bytes_timeout)
{
    int len = 0;

    while(1)
    {
        rt_sem_control(_rx_sem, RT_IPC_CMD_RESET, RT_NULL);

        int rc = rt_device_read(serial, 0, buf + len, bufsz);
        if(rc > 0)
        {
            timeout = bytes_timeout;
            len += rc;
            bufsz -= rc;
            if(bufsz == 0)
                break;

            continue;
        }

        if(rt_sem_take(_rx_sem, rt_tick_from_millisecond(timeout)) != RT_EOK)
            break;
        timeout = bytes_timeout;
    }

    return len;
}
/**
  * @brief  MODBUS�߳�
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void modbus_thread(void* p)
{
  if(serial_init() < 0)
  {
    LOG_E("%s init failed",UART_NAME);
  }
  uint8_t ctx_send_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
  uint8_t ctx_read_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
  agile_modbus_rtu_t ctx_rtu;
  agile_modbus_t *ctx = &ctx_rtu._ctx;
  agile_modbus_rtu_init(&ctx_rtu, ctx_send_buf, sizeof(ctx_send_buf), ctx_read_buf, sizeof(ctx_read_buf));
  agile_modbus_set_slave(ctx, SLAVE_ADDRESS);

  while(1)
  {
      int read_len = serial_receive(ctx->read_buf, ctx->read_bufsz, 1000, 20);
      if (read_len == 0)
      {
          LOG_W("Receive timeout.");
          continue;
      }

      int rc = agile_modbus_slave_handle(ctx, read_len, 0, agile_modbus_slave_util_callback, &_slave_util, NULL);

      if (rc < 0)
      {
          LOG_W("Receive failed.");
          if (rc != -1)
          {
              LOG_W("Error code:%d", -128 - rc);
          }
          continue;
      }
      serial_send(ctx->send_buf, rc);
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