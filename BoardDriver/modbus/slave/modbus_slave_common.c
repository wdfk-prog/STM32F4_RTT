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
#include "modbus_slave_common.h"
/* Private includes ----------------------------------------------------------*/
#include "gpio.h"

/*ulog include*/
#define LOG_TAG              "mb_s_com" 
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define MAX_MUTEX_WAIT_TIME 10      //�������ȴ�ʱ��
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
rt_mutex_t modbus_mutex = RT_NULL;
/* Private function prototypes -----------------------------------------------*/
extern int modbus_slave_register_default(void);
extern int modbus_slave_input_register_default(void);

extern void modbus_slave_input_register_write(void);
extern void modbus_slave_register_write(void);
/**
  * @brief  д��MODBUSĬ��ֵ
  * @param  None
  * @retval None
  * @note   None
*/
static int modbus_default(void)
{
  modbus_slave_register_default();
  modbus_slave_input_register_default();

  modbus_mutex = rt_mutex_create("modbus",RT_IPC_FLAG_PRIO);

  return RT_EOK;
}
INIT_COMPONENT_EXPORT(modbus_default);
/**
  * @brief  ����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_mutex_lock(void)
{
	if(rt_mutex_take(modbus_mutex, MAX_MUTEX_WAIT_TIME) != RT_EOK) 
  {
		LOG_E("canfestival take mutex failed!");
	}
}
/**
  * @brief  ����
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_mutex_unlock(void)
{
	if(rt_mutex_release(modbus_mutex) != RT_EOK) 
  {
		LOG_E("canfestival release mutex failed!");
	}
}
/**
  * @brief  �Թ����ַд�뱾������
  * @param  None
  * @retval None
  * @note   None
*/
void modbus_slave_write(void)
{
    modbus_slave_input_register_write();
    modbus_slave_register_write();
    HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
}
#ifdef RT_USING_MSH
/**
  * @brief  ��ӡMODBUS�б�
  * @param  ��һ��������int�͵�argc��Ϊ���ͣ�����ͳ�Ƴ�������ʱ���͸�main�����������в����ĸ���
  * @retval �ڶ���������char*�͵�argv[]��Ϊ�ַ������飬�������ָ����ַ���������ָ�����飬ÿһ��Ԫ��ָ��һ��������
  * @note   None.
*/
static void modbus_get(int argc, char**argv)
{
#define MODBUS_LIST                   0
#define MODBUS_EG                     1
#define MODBUS_TYPE                   2
#define MODBUS_EN                     3
#define MODBUS_HEX                    4
  int16_t len,start,cnt = 0;
  uint8_t flag = 0,hex = 0;
  const char* help_info[] =
    {
        [MODBUS_LIST]           = "Please input'modbus_get [type|start_addr|lenth] <en> <hex>'",
        [MODBUS_EG]             = "       e.g : modbus_get  hold    100       10.",
        [MODBUS_TYPE]           = "      type : hold | coil | input | in_bit.",
        [MODBUS_EN]             = "      en   : EN is Only look at not zero data.",
        [MODBUS_HEX]            = "      hex  : Data is displayed in hexadecimal format.",
    };

  if (argc < 3)
  {
      rt_kprintf("Usage:\n");
      for (size_t i = 0; i < sizeof(help_info) / sizeof(char*); i++)
      {
          rt_kprintf("%s\n", help_info[i]);
      }
      rt_kprintf("\n");
      return;
  }

  if(strspn(argv[2], "0123456789") == strlen(argv[2]))//�жϴ����ַ����Ƿ�ȫ����
  {
    start = atoi(argv[2]);
  }
  else
  {
    rt_kprintf("Please enter only Nonnegative\n");
    return;
  }
  
  if(strspn(argv[3], "0123456789") == strlen(argv[3]))//�жϴ����ַ����Ƿ�ȫ����
  {
    len = atoi(argv[3]);
  }
  else
  {
    rt_kprintf("Please enter only Nonnegative\n");
    return;
  }
  if(argv[4])
  {
    if(!rt_strcmp(argv[4],"en"))
    {
      flag = 1;
    }
    else
    {
      rt_kprintf("Please enter en\n");
      return;
    }
  }
  if(argv[5])
  {
    if(!rt_strcmp(argv[5],"hex"))
    {
      hex = 1;
    }
    else
    {
      rt_kprintf("Please enter hex\n");
      return;
    }
  }
  if(!rt_strcmp(argv[1], "hold"))
  {
    if(start+len > MODBUS_REG_MAX_NUM)
    {
      rt_kprintf("Input out of index range,The value ranges from 1 to %d\n",MODBUS_REG_MAX_NUM);
      return;
    }
    for(uint16_t i = start;i < start+len;i++)
    {
      if(flag == 1)
      {
        if(modbus_register_get(0,i) != 0)
        {
          if(cnt % 5 == 0)
          {
            rt_kprintf("\r\n");
            cnt = 0;
          }
          cnt++;
          if(hex == 0)
            rt_kprintf("num:%3d|hold:%5d#",i,modbus_register_get(0,i));
          else
            rt_kprintf("num:%3d|hold:0X%04X#",i,modbus_register_get(0,i));
        }
      }
      else
      {
        if(cnt % 5 == 0)
        {
          rt_kprintf("\r\n");
          cnt = 0;
        }
        cnt++;
        if(hex == 0)
          rt_kprintf("num:%3d|hold:%5d#",i,modbus_register_get(0,i));
        else
          rt_kprintf("num:%3d|hold:0X%04X#",i,modbus_register_get(0,i));
      }
    }
    rt_kprintf("\r\n");
  }
  else if(!rt_strcmp(argv[1], "input"))
  {
    if(start+len > MODBUS_REG_MAX_NUM)
    {
      rt_kprintf("Input out of index range,The value ranges from 1 to %d\n",MODBUS_REG_MAX_NUM);
      return;
    }
    for(uint16_t i = start;i < start+len;i++)
    {
      if(flag == 1)
      {
        if(modbus_input_register_get(0,i) != 0)
        {
          if(cnt % 5 == 0)
          {
            rt_kprintf("\r\n");
            cnt = 0;
          }
          cnt++;
          if(hex == 0)
            rt_kprintf("num:%3d|input:%5d#", i, modbus_input_register_get(0,i));
          else
            rt_kprintf("num:%3d|input:0X%04X#", i, modbus_input_register_get(0,i));
        }
      }
      else
      {
        if(cnt % 5 == 0)
        {
          rt_kprintf("\r\n");
          cnt = 0;
        }
        cnt++;
        if(hex == 0)
          rt_kprintf("num:%3d|input:%5d#", i, modbus_input_register_get(0,i));
        else
          rt_kprintf("num:%3d|input:0X%04X#", i, modbus_input_register_get(0,i));
      }
    }
    rt_kprintf("\r\n");
  }
  else if (!rt_strcmp(argv[1], "coil"))
  {
    if(start+len > MODBUS_BIT_MAX_NUM)
    {
      rt_kprintf("Input out of index range,The value ranges from 1 to %d\n",MODBUS_BIT_MAX_NUM);
      return;
    }
    for(uint16_t i = start;i < start+len;i++)
    {
      if(flag == 1)
      {
        if( modbus_bits_get(0,i)!=0)
        {
          if(cnt % 7 == 0)
          {
            rt_kprintf("\r\n");
            cnt = 0;
          }
          cnt++;
          rt_kprintf("num:%3d|coil:%d#", i, modbus_bits_get(0,i));
        }
      }
      else
      {
        if(cnt % 7 == 0)
        {
          rt_kprintf("\r\n");
          cnt = 0;
        }
        cnt++;
        rt_kprintf("num:%3d|coil:%d#", i, modbus_bits_get(0,i));
      }
    }
    rt_kprintf("\r\n");
  }
  else if (!rt_strcmp(argv[1], "in_bit"))
  {
    if(start+len > MODBUS_BIT_MAX_NUM)
    {
      rt_kprintf("Input out of index range,The value ranges from 1 to %d\n",MODBUS_BIT_MAX_NUM);
      return;
    }
    for(uint16_t i = start;i < start+len;i++)
    {
      if(flag == 1)
      {
        if( modbus_input_bits_get(0,i)!=0)
        {
          if(cnt % 7 == 0)
          {
            rt_kprintf("\r\n");
            cnt = 0;
          }
          cnt++;
          rt_kprintf("num:%3d|bits:%d#", i, modbus_input_bits_get(0,i));
        }
      }
      else
      {
        if(cnt % 7 == 0)
        {
          rt_kprintf("\r\n");
          cnt = 0;
        }
        cnt++;
        rt_kprintf("num:%3d|bits:%d#", i, modbus_input_bits_get(0,i));
      }
    }
    rt_kprintf("\r\n");
  }
  else
  {
      rt_kprintf("Usage:\n");
      for (size_t i = 0; i < sizeof(help_info) / sizeof(char*); i++)
      {
          rt_kprintf("%s\n", help_info[i]);
      }
      rt_kprintf("\n");
      return;
  }
}
MSH_CMD_EXPORT_ALIAS(modbus_get,modbus_get,modbus_get <type|start_addr|lenth>);
/**
  * @brief  ����MODBUS����
  * @param  ��һ��������int�͵�argc��Ϊ���ͣ�����ͳ�Ƴ�������ʱ���͸�main�����������в����ĸ���
  * @retval �ڶ���������char*�͵�argv[]��Ϊ�ַ������飬�������ָ����ַ���������ָ�����飬ÿһ��Ԫ��ָ��һ��������
  * @note   None.
*/
static void modbus_set(int argc, char**argv)
{
  int16_t num,addr;
  if (argc < 3)
  {
      rt_kprintf("Please input'modbus_set [type|addr|num]'\n");
      rt_kprintf("       e.g : modbus_set  hold 100  10\n");
      rt_kprintf("       type: hold | coil\r\n");
      return;
  }
  if(rt_strcmp(argv[1], "hold") && rt_strcmp(argv[1], "coil"))
  {
     rt_kprintf("Input out of index range,The value ranges from 1 to %d\n",MODBUS_REG_MAX_NUM);
    return;
  }
  if(strspn(argv[2], "0123456789") == strlen(argv[2]))//�жϴ����ַ����Ƿ�ȫ����
  {
    addr = atoi(argv[2]);
  }
  else
  {
    rt_kprintf("Please enter only Nonnegative\n");
    return;
  }
  
  if(strspn(argv[3], "0123456789") == strlen(argv[3]))//�жϴ����ַ����Ƿ�ȫ����
  {
    num = atoi(argv[3]);
  }
  else
  {
    rt_kprintf("Please enter only Nonnegative\n");
    return;
  }
  if(!rt_strcmp(argv[1], "hold"))
  {
    if(addr > MODBUS_REG_MAX_NUM || addr < 0)
    {
      rt_kprintf("Input out of index range\r\n");
      return;
    }
    modbus_register_set(0,addr,num);
    rt_kprintf("usRegHoldingBuf[%d] set value:%d\r\n",addr,num);
  }
  else if (!rt_strcmp(argv[1], "coil"))
  {
    if(addr > MODBUS_BIT_MAX_NUM || addr < 0)
    {
      rt_kprintf("Input out of index range,The value ranges from 1 to %d\n",MODBUS_BIT_MAX_NUM);
      return;
    }
    if(num != 0 && num != 1)
    {
      rt_kprintf("Please enter 0 or 1\n");
    }
    modbus_bits_set(0,addr,num);
    rt_kprintf("ucRegCoilsBuf[%d] set value:%d\r\n",addr,num);
  }
  else
  {
      rt_kprintf("Please input'modbus_set <type|addr|num>'\n");
      rt_kprintf("       e.g : modbus_set  hold 100  10\n");
      rt_kprintf("       type: hold | coil\r\n");
      return;
  }
}
MSH_CMD_EXPORT_ALIAS(modbus_set,modbus_set,modbus_set <type|addr|num>);
#endif