/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : mb_handler.c
  * @brief          : MODBUS����
  * @date           :2022.08.26
  ******************************************************************************
  * @attention https://www.amobbs.com/thread-5491615-1-1.html
  
  �ӻ���ַ	������	��ʼ��ַ��λ	��ʼ��ַ��λ	�Ĵ���������λ	�Ĵ���������λ	CRC��λ	CRC��λ
  01        03          00          01            00              01              D5      CA
  
  MB_SLAVE1 <-��д-> |           |  ʹ�û����������ƻ�
 ʹ�û����������ƻ�  | MB_Buffer |  <-��д-> MB_Handler
  MB_SLAVE2 <-��д-> |           |

  * @author hly
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "mb_handler.h"
#include "mb.h"
#include "mbutils.h"
#include <string.h>
#include <stdlib.h>
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "mb_key.h"
#include "adc_dma.h"
#include "monitor.h"
#include "turn_motor.h"
#include "walk_motor.h"
/*ulog include*/
#define LOG_TAG         "mb_handl"
#define LOG_LVL         DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define MB_OFFSET 0//��Ļƫ�Ƶ�ַ����ʼ��ַ��1��ʼ
/* Private macro -------------------------------------------------------------*/
/* �߳����� */
#define THREAD_PRIORITY      11//�߳����ȼ�
#define THREAD_TIMESLICE     10//�߳�ʱ��Ƭ
#define THREAD_STACK_SIZE    4096//ջ��С

/* Variables' number */
#define NB_OF_VAR             ((uint8_t)20)
/* Private variables ---------------------------------------------------------*/
//����Ĵ�������
uint16_t usRegInputBuf[REG_INPUT_NREGS];
//����Ĵ�����ʼ��ַ
int16_t usRegInputStart = REG_INPUT_START + MB_OFFSET;
//���ּĴ�������
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
//���ּĴ�����ʼ��ַ
int16_t usRegHoldingStart = REG_HOLDING_START + MB_OFFSET;
//��Ȧ״̬
uint8_t  ucRegCoilsBuf[REG_COILS_SIZE / 8];
//��Ȧ��ʼ��ַ
int16_t ucRegCoilsStart = REG_COILS_START;
//��������״̬
uint8_t  ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8];
//������ʼ��ַ
int16_t ucRegDiscreteStart = REG_DISCRETE_START;
/* Private function prototypes -----------------------------------------------*/
extern int Flash_KVDB_Init(void);
/**
  * @brief  MODBUS--���ݳ�ʼ��
  * @param  None
  * @retval None
  * @note   ������ݴ��봫��
*/
void Modbus_Data_Init(void)
{
  /*��Ȧ��ʼ��*/
  TURN_ENABLE_SET;
  WALK_ENABLE_SET;
  /*���ּĴ�����ʼ��*/
  /*ת������������*/
  usRegHoldingBuf[501] = (ushort)Turn_Cycle_Pulse_Num;                                          //����ת��������������
  usRegHoldingBuf[502] = (ushort)Turn_REDUCTION_RATIO;                                          //����ת�������ٱ�
  usRegHoldingBuf[503] = (ushort)Turn_CLOCKWISE;                                                //˳ʱ�뷽���Ӧ��ֵ
  usRegHoldingBuf[504] = (ushort)Turn_ANGLE_MAX;                                                //�������Ƕ�
  usRegHoldingBuf[505] = (ushort)Turn_ANGLE_MIN;                                                //������С�Ƕ�
  usRegHoldingBuf[506] = (ushort)Turn_SPEED_MAX;                                                //��������ٶ�
  usRegHoldingBuf[507] = (ushort)Turn_SPEED_MIN;                                                //������С�ٶ�
  usRegHoldingBuf[508] = (ushort)Turn_SPEED_JOG;                                                //����ת������λ�Ƕ�
  usRegHoldingBuf[509] = (ushort)Turn_SANGLE_INIT;                                              //����ת������λ�Ƕ�
  usRegHoldingBuf[510] = (ushort)Turn_BACK_HIGH_SPEED;                                          //��ԭ����
  usRegHoldingBuf[511] = (ushort)Turn_BACK_LOW_SPEED;                                           //��ԭ����
  /*���ߵ����������*/
  usRegHoldingBuf[521] = (ushort)Walk_Cycle_Pulse_Num;                                          //���õ������������
  usRegHoldingBuf[522] = (ushort)Walk_REDUCTION_RATIO;                                          //���õ�����ٱ�
  usRegHoldingBuf[523] = (ushort)Walk_CLOCKWISE;                                                //˳ʱ�뷽���Ӧ��ֵ
  usRegHoldingBuf[524] = (ushort)Walk_SPEED_MAX;                                                //�������
  usRegHoldingBuf[525] = (ushort)Walk_SPEED_MIN;                                                //��С����
  usRegHoldingBuf[526] = (ushort)Walk_STEP;                                                     //�Ӽ��ٲ���
  usRegHoldingBuf[527] = (ushort)Walk_TIME;                                                     //�Ӽ�������
  
  usRegHoldingBuf[699] = (ushort)0;                                                             //ADC��������
  usRegHoldingBuf[700] = (ushort)2200;                                                          //adc��ֵ
  /*������Ϣ*/
  strcpy((char *)(usRegHoldingBuf+900),VERSION);                                        //��ӡ�汾��Ϣ
  usRegHoldingBuf[988] = (ushort)((uint32_t)(YEAR*10000+(MONTH + 1)*100+DAY) & 0xffff); //���ڵ�16bti
	usRegHoldingBuf[989] = (ushort)((uint32_t)(YEAR*10000+(MONTH + 1)*100+DAY)>>16);		  //���ڸ�16bti
	usRegHoldingBuf[990] = (ushort)((uint32_t)(HOUR*10000+MINUTE*100+SEC)&0xffff);        //ʱ���16bti
	usRegHoldingBuf[991] = (ushort)((uint32_t)(HOUR*10000+MINUTE*100+SEC)>>16);		        //ʱ���16bti
  /*ID��������[�����]*/
  usRegHoldingBuf[992] =  HAL_GetUIDw0();
  usRegHoldingBuf[993] =  HAL_GetUIDw0() >> 16;
  usRegHoldingBuf[994] =  HAL_GetUIDw1();
  usRegHoldingBuf[995] =  HAL_GetUIDw1() >> 16;
  usRegHoldingBuf[996] =  HAL_GetUIDw2();
  usRegHoldingBuf[997] =  HAL_GetUIDw2() >> 16;
  usRegHoldingBuf[998] =  HAL_GetHalVersion();
  usRegHoldingBuf[999] =  HAL_GetHalVersion() >> 16;
}
/**
  * @brief  MODBUS--���ݴ�������ʼ��
  * @param  None
  * @retval None
  * @note   ������ݴ��봫��
*/
void Modbus_Handler_Init(void)
{
  Modbus_Data_Init();
  /*MB������ʼ��*/
  MBKEY_Init();
  /*���ݿ��ʼ��*/
  Flash_KVDB_Init();
}
/**
  * @brief  MODBUS--���ݴ�����
  * @param  None
  * @retval None
  * @note   ������ݴ��봫��
*/
void Modbus_Handler(void)
{
/**********************���ػ������ַ[0-500]**************************************/
/*******����λ********************************************************************/ 

/*******����λ********************************************************************/

/*******����Ĵ���[1-100]****************************************************************/
  /*ת������������*/
  turn.set_radian                          = _float(usRegHoldingBuf[1])/1000.0f;                 //����ת������1���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  //  turn_motor[1].set_radian                 = (_short)usRegHoldingBuf[2]/1000.0f;              //����ת������2���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  //  turn_motor[2].set_radian                 = (_short)usRegHoldingBuf[3]/1000.0f;              //����ת������3���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  //  turn_motor[3].set_radian                 = (_short)usRegHoldingBuf[4]/1000.0f;              //����ת������4���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  /*���ߵ����������*/
  walk.motor.set_speed                     = (_short)usRegHoldingBuf[5];                         //�������ߵ����1��ת�١���λ��RPM��������ǰ�������RPM���١�
//  walk_motor[1].set_speed                  = (_short)usRegHoldingBuf[6];                        //�������ߵ����2��ת�١���λ��RPM��
//  walk_motor[2].set_speed                  = (_short)usRegHoldingBuf[7];                        //�������ߵ����3��ת�١���λ��RPM��
//  walk_motor[3].set_speed                  = (_short)usRegHoldingBuf[8];                        //�������ߵ����4��ת�١���λ��RPM��
  /*������������*/
  IPC_Beat.Value                           = (ushort)usRegHoldingBuf[199];                       //���ػ���������1sд��һ�Ρ�1.2s���һ��
  IPC_Beat.EN                              = (ushort)usRegHoldingBuf[200];                       //���ػ�������⿪��                  
/*******�����Ĵ���[201-500]****************************************************************/
  /*ת������ѯ����*/
  usRegHoldingBuf[201]                     = (_short)(turn.get_radian*1000);                     //��ѯת������1���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  //  usRegHoldingBuf[202]                  = (_short)(turn_motor[1].get_radian*1000);             //��ѯת������2���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  //  usRegHoldingBuf[203]                  = (_short)(turn_motor[2].get_radian*1000);             //��ѯת������3���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  //  usRegHoldingBuf[204]                  = (_short)(turn_motor[3].get_radian*1000);             //��ѯת������4���Ƕȡ���λ�����ȡ����Ŵ�1000������ȷС�����3λ��
  /*���߲�ѯ�ٶ�����*/
  usRegHoldingBuf[205]                     = (_short)walk.motor.get_speed;                       //��ѯ���ߵ����1��ת�١���λ��RPM ��
//  usRegHoldingBuf[206]                     = (_short)walk_motor[1].get_speed;                    //��ѯ���ߵ����2��ת�١���λ��RPM ��
//  usRegHoldingBuf[207]                     = (_short)walk_motor[2].get_speed;                    //��ѯ���ߵ����3��ת�١���λ��RPM ��
//  usRegHoldingBuf[208]                     = (_short)walk_motor[3].get_speed;                    //��ѯ���ߵ����4��ת�١���λ��RPM ��
  usRegHoldingBuf[500]                     = (ushort)(voltage.AD.vaule*ADC_RATIO*100);           //��ص�ѹ
/**********************���Դ����ַ[501-999]**************************************/
/*******����λ********************************************************************/ 

/*******����λ********************************************************************/

/*******����Ĵ���[501-700]****************************************************************/
  /*ת������������*/
  turn.motor.Ctrl.cycle_pulse_num       = (ushort)usRegHoldingBuf[501];                          //���õ������������
  turn.motor.Ctrl.reduction_ratio       = (ushort)usRegHoldingBuf[502];                          //���õ�����ٱ�
  turn.motor.Ctrl.clockwise             = (ushort)usRegHoldingBuf[503];                          //˳ʱ�뷽���Ӧ��ֵ
  turn.motor.Abs.max_angle              = (ushort)usRegHoldingBuf[504];                          //�������Ƕ�
  turn.motor.Abs.min_angle              = (ushort)usRegHoldingBuf[505];                          //������С�Ƕ�
  turn.motor.V.max_speed                = (ushort)usRegHoldingBuf[506],                          //��������ٶ�
  turn.motor.V.min_speed                = (ushort)usRegHoldingBuf[507];                          //������С�ٶ�
  turn.motor.V.set_speed                = (ushort)usRegHoldingBuf[508];                          //���õ㶯�ٶ�
  turn.motor.Do.init_freq               =  _float(usRegHoldingBuf[509])/1000.0f;                 //����ת������λ�Ƕ�
  turn.motor.Do.speed_high              = (_short)usRegHoldingBuf[510];                          //��ԭ����
  turn.motor.Do.speed_low               = (_short)usRegHoldingBuf[511];                          //��ԭ����
  /*���ߵ����������*/
  walk.motor.cycle_pulse_num            = (ushort)usRegHoldingBuf[521];                          //���õ������������
  walk.motor.reduction_ratio            = (ushort)usRegHoldingBuf[522];                          //���õ�����ٱ�
  walk.motor.clockwise                  = (ushort)usRegHoldingBuf[523];                          //˳ʱ�뷽���Ӧ��ֵ
  walk.motor.max_speed                  = (ushort)usRegHoldingBuf[524];                          //�������
  walk.motor.min_speed                  = (ushort)usRegHoldingBuf[525];                          //��С����
  walk.motor.AND.step                   = (ushort)usRegHoldingBuf[526];                          //�Ӽ��ٲ���
  walk.motor.AND.time                   = (ushort)usRegHoldingBuf[527];                          //�Ӽ�������
  
  WDT_key                               = (ushort)usRegHoldingBuf[687];                          //���Ź���������
  Weinview_Beat.Value                   = (ushort)usRegHoldingBuf[688];                          //����ͨ��������1sд��һ�Ρ�1.2s���һ��
  Weinview_Beat.EN                      = (ushort)usRegHoldingBuf[689];                          //����ͨ������⿪��                  
  voltage.AD.Compensation               = _float(usRegHoldingBuf[699])/9.0f/100;                 //ADC��������
  voltage.VPT                           = _float(usRegHoldingBuf[700])/100.0f;                   //adc��ֵ
/*******�����Ĵ���[701-999]****************************************************************/
  /*ת������ѯ��������*/
  usRegHoldingBuf[701]                  = (ushort)turn.state.MOTOR;                              //��ѯ���״̬
  usRegHoldingBuf[702]                  = (ushort)(turn.set_freq / 10);                          //��ѯ�����Ƶ��
  usRegHoldingBuf[703]                  = (_short)(turn.get_freq / 10);                          //��ѯ�����Ƶ��
  usRegHoldingBuf[704]                  = (ushort)turn.PIN.Dir.level;                            //�����־
  usRegHoldingBuf[705]                  = (ushort)turn.PIN.Brake.level;                          //ɲ����־
  usRegHoldingBuf[706]                  = (ushort)turn.PIN.EN.level;                             //ʹ�ܱ�־
  usRegHoldingBuf[707]                  = (_short)turn.motor.Ctrl.CurrentPosition_Pulse;         //��еλ��
  usRegHoldingBuf[708]                  = (ushort)turn.motor.Rel.dest_position;                  //��������
  usRegHoldingBuf[709]                  = (ushort)turn.motor.Abs.over_range;                     //�����Ƿ񳬳��Ƕ�
  usRegHoldingBuf[710]                  = (_short)turn.motor.V.get_speed;                        //������ٶ�
  usRegHoldingBuf[711]                  = (ushort)turn.motor.Do.flag;                            //���㺯��״̬
  usRegHoldingBuf[712]                  = (ushort)turn.motor.Do.init_state;                      //�����ʼ��״̬
  usRegHoldingBuf[713]                  = (ushort)turn.limit_mode;                               //�����λģʽ
  usRegHoldingBuf[714]                  = (ushort)turn.Stop_state;                              //���ֹͣ����
  /*���ߵ����ѯ��������*/
  usRegHoldingBuf[721]                  = (ushort)walk.state.MOTOR;                              //��ѯ���״̬
  usRegHoldingBuf[722]                  = (ushort)(walk.motor.set_freq / 10);                    //��ѯ�����Ƶ��
  usRegHoldingBuf[723]                  = (ushort)walk.PIN.Brake.level;                          //ɲ����־
  usRegHoldingBuf[724]                  = (ushort)walk.PIN.Dir.level;                            //�����־
  usRegHoldingBuf[725]                  = (ushort)walk.PIN.EN.level;                             //ʹ�ܱ�־
  usRegHoldingBuf[726]                  = (ushort)walk.limit_mode;                               //�����λģʽ
  usRegHoldingBuf[727]                  = (ushort)walk.Stop_state;                               //���ֹͣ����
  /*������ѯ��������*/
  
  usRegHoldingBuf[802]                  = (ushort)RestFlag;                                      //��λ��־
  usRegHoldingBuf[803]                  = (_short)Weinview_Beat.Error;                           //��ѯ����ͨ�������ʱ��
  usRegHoldingBuf[804]                  = (_short)IPC_Beat.Error;                                //��ѯ���ػ��������ʱ��
}
/**
  * @brief  modbus��ʼ��
  * @param  None.
  * @retval None.
  * @note   ����MB�����̣߳�����ʼ���ź�����MB�����ж�ΪMODBUS������ʱ�������ݶ�д
*/
static int Modbus_Init(void)
{
  rt_err_t ret = RT_EOK;    
  Modbus_Handler_Init();
  Turn_Motor_Init();
  Walk_Motor_Init();
  /* ���� MODBUS�߳�*/
  rt_thread_t thread = rt_thread_create( "mb key",    /* �߳����� */
                                         MBKey_Handler,/* �߳���ں��� */
                                         RT_NULL,       /* �߳���ں������� */
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
INIT_ENV_EXPORT(Modbus_Init);
/**
  * @brief  ��ӡMODBUS�б�
  * @param  ��һ��������int�͵�argc��Ϊ���ͣ�����ͳ�Ƴ�������ʱ���͸�main�����������в����ĸ���
  * @retval �ڶ���������char*�͵�argv[]��Ϊ�ַ������飬�������ָ����ַ���������ָ�����飬ÿһ��Ԫ��ָ��һ��������
  * @note   None.
*/
static void Modbus_list(int argc, char**argv)
{
  int16_t len,start,cnt = 0,flag = 0;
  if (argc < 3)
  {
      rt_kprintf("Please input'Modbus_list <type|start_addr|lenth>'\n");
      rt_kprintf("       e.g : Modbus_list  hold    100       10\n");
      rt_kprintf("      type : hold | coil\r\n");
      rt_kprintf("      EN   : EN is Only look at not zero data\r\n");
      return;
  }
  if(strspn(argv[2], "0123456789") == strlen(argv[2]))//�жϴ����ַ����Ƿ�ȫ����
  {
    start = atoi(argv[2]);
  }
  else
  {
    rt_kprintf("Please enter only digits\n");
    return;
  }
  
  if(strspn(argv[3], "0123456789") == strlen(argv[3]))//�жϴ����ַ����Ƿ�ȫ����
  {
    len = atoi(argv[3]);
  }
  else
  {
    rt_kprintf("Please enter only digits\n");
    return;
  }
  if(argv[4])
  {
    if(!rt_strcmp(argv[4],"EN"))
    {
      flag = 1;
    }
  }
  if(!rt_strcmp(argv[1], "hold"))
  {
    if(start+len > REG_INPUT_NREGS)
    {
      rt_kprintf("Input out of index range\r\n");
      return;
    }
    for(uint16_t i = start;i < start+len;i++)
    {
      if(flag == 1)
      {
        if(usRegHoldingBuf[i] != 0)
        {
          if(cnt % 5 == 0)
          {
            rt_kprintf("\r\n");
            cnt = 0;
          }
          cnt++;
          rt_kprintf("num:%3d|hold:%5d#",i,usRegHoldingBuf[i]);
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
        rt_kprintf("num:%3d|hold:%5d#",i,usRegHoldingBuf[i]);
      }
    }
    rt_kprintf("\r\n");
  }
  else if (!rt_strcmp(argv[1], "coil"))
  {
    if(start+len > REG_COILS_SIZE)
    {
      rt_kprintf("Input out of index range\r\n");
      return;
    }
    for(uint16_t i = start;i < start+len;i++)
    {
      if(flag == 1)
      {
        if(MB_GET_BIT(i)!=0)
        {
          if(cnt % 7 == 0)
          {
            rt_kprintf("\r\n");
            cnt = 0;
          }
          cnt++;
          rt_kprintf("num:%3d|coil:1#",i);
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
        if(MB_GET_BIT(i))
        {
          rt_kprintf("num:%3d|coil:1#",i);
        }
        else
        {
          rt_kprintf("num:%3d|coil:0#",i);
        }
      }
    }
    rt_kprintf("\r\n");
  }
  else
  {
      rt_kprintf("Please input'Modbus_list <type|start_addr|lenth>'\n");
      rt_kprintf("       e.g : Modbus_list  hold    100       10\n");
      rt_kprintf("      type : hold | coil\r\n");
      rt_kprintf("      EN   : EN is Only look at not zero data\r\n");
      return;
  }
}
MSH_CMD_EXPORT_ALIAS(Modbus_list,Modbus_list,Modbus_list <type|start_addr|lenth>);
/**
  * @brief  ����MODBUS����
  * @param  ��һ��������int�͵�argc��Ϊ���ͣ�����ͳ�Ƴ�������ʱ���͸�main�����������в����ĸ���
  * @retval �ڶ���������char*�͵�argv[]��Ϊ�ַ������飬�������ָ����ַ���������ָ�����飬ÿһ��Ԫ��ָ��һ��������
  * @note   None.
*/
static void Modbus_Set(int argc, char**argv)
{
  int16_t num,addr;
  if (argc < 3)
  {
      rt_kprintf("Please input'Modbus_Set <type|addr|num>'\n");
      rt_kprintf("       e.g : Modbus_Set  hold 100  10\n");
      rt_kprintf("       type: hold | coil\r\n");
      return;
  }
  if(rt_strcmp(argv[1], "hold") && rt_strcmp(argv[1], "coil"))
  {
    rt_kprintf("Please enter the:hold | coil, before entering the value\n");
    return;
  }
  if(strspn(argv[2], "0123456789") == strlen(argv[2]))//�жϴ����ַ����Ƿ�ȫ����
  {
    addr = atoi(argv[2]);
  }
  else
  {
    rt_kprintf("Please enter only digits\n");
    return;
  }
  
  if(strspn(argv[3], "0123456789") == strlen(argv[3]))//�жϴ����ַ����Ƿ�ȫ����
  {
    num = atoi(argv[3]);
  }
  else
  {
    rt_kprintf("Please enter only digits\n");
    return;
  }
  if(!rt_strcmp(argv[1], "hold"))
  {
    if(addr > REG_INPUT_NREGS || addr < 0)
    {
      rt_kprintf("Input out of index range\r\n");
      return;
    }
    usRegHoldingBuf[addr] = num;
    rt_kprintf("usRegHoldingBuf[%d] set value:%d\r\n",addr,num);
  }
  else if (!rt_strcmp(argv[1], "coil"))
  {
    if(addr > REG_COILS_SIZE || addr < 0 || (num != 0 && num != 1))
    {
      rt_kprintf("Input out of index range\r\n");
      return;
    }
    if(num)
      MB_SET_BIT(addr);
    else
      MB_RESET_BIT(addr);
    rt_kprintf("ucRegCoilsBuf[%d] set value:%d\r\n",addr,num);
  }
  else
  {
      rt_kprintf("Please input'Modbus_Set <type|start_addr|lenth|EN>'\n");
      rt_kprintf("       e.g : Modbus_Set  hold    100       10\n");
      rt_kprintf("       type: hold | coil\r\n");
      return;
  }
}
MSH_CMD_EXPORT_ALIAS(Modbus_Set,Modbus_Set,Modbus_Set <type|addr|num>);
/****************************************************************************
* ��	  ��:eMBRegInputCB 
* ��    ��:��ȡ����Ĵ���,��Ӧ�������� 04 eMBFuncReadInputRegister
* ��ڲ���:pucRegBuffer: ���ݻ�����,������Ӧ����   
*						usAddress: �Ĵ�����ַ
*						usNRegs: Ҫ��ȡ�ļĴ�������
* ���ڲ���:
* ע	  ��:��λ�������� ֡��ʽ��: SlaveAddr(1 Byte)+FuncCode(1 Byte)
*								+StartAddrHiByte(1 Byte)+StartAddrLoByte(1 Byte)
*								+LenAddrHiByte(1 Byte)+LenAddrLoByte(1 Byte)+
*								+CRCAddrHiByte(1 Byte)+CRCAddrLoByte(1 Byte)
*							3 ��
****************************************************************************/
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
    //��ѯ�Ƿ��ڼĴ�����Χ��
    //Ϊ�˱��⾯�棬�޸�Ϊ�з�������
    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {   //��ò���ƫ���������β�����ʼ��ַ-����Ĵ����ĳ�ʼ��ַ
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )//�����ֵ
        {
            *pucRegBuffer++ = ( UCHAR )( usRegInputBuf[iRegIndex] >> 8 );//��ֵ���ֽ�
            *pucRegBuffer++ = ( UCHAR )( usRegInputBuf[iRegIndex] & 0xFF );//��ֵ���ֽ�
            iRegIndex++;//ƫ��������
            usNRegs--;//�������Ĵ��������ݼ�
        }
    }
    else
    {
        eStatus = MB_ENOREG;//���ش���״̬���޼Ĵ���
    }

    return eStatus;
}

/****************************************************************************
* ��	  ��:eMBRegHoldingCB 
* ��    ��:��Ӧ��������:06 д���ּĴ��� eMBFuncWriteHoldingRegister 
*													16 д������ּĴ��� eMBFuncWriteMultipleHoldingRegister
*													03 �����ּĴ��� eMBFuncReadHoldingRegister
*													23 ��д������ּĴ��� eMBFuncReadWriteMultipleHoldingRegister
* ��ڲ���:pucRegBuffer: ���ݻ�����,������Ӧ����   
*						usAddress: �Ĵ�����ַ
*						usNRegs: Ҫ��д�ļĴ�������
*						eMode: ������
* ���ڲ���:
* ע	  ��:4 ��
****************************************************************************/
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
	eMBErrorCode    eStatus = MB_ENOERR;//����״̬
	int             iRegIndex;          //ƫ����

  //�жϼĴ����ǲ����ڷ�Χ��
	if((usAddress >= REG_HOLDING_START)&&\
		((usAddress+usNRegs) <= (REG_HOLDING_START + REG_HOLDING_NREGS)))
	{
		iRegIndex = (int)(usAddress - usRegHoldingStart);//����ƫ����
		switch(eMode)
		{                                       
			case MB_REG_READ://�� MB_REG_READ = 0//��������
        while(usNRegs > 0)
				{
					*pucRegBuffer++ = (uint8_t)(usRegHoldingBuf[iRegIndex] >> 8);            
					*pucRegBuffer++ = (uint8_t)(usRegHoldingBuf[iRegIndex] & 0xFF); 
          iRegIndex++;
          usNRegs--;					
				}                            
        break;
			case MB_REG_WRITE://д MB_REG_WRITE = 0//д������
				while(usNRegs > 0)
				{         
					usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
          usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
          iRegIndex++;
          usNRegs--;
        }		
        break;
			}
	}
	else//����
	{
		eStatus = MB_ENOREG;//���ش���״̬
	}	
	
	return eStatus;
}

/****************************************************************************
* ��	  ��:eMBRegCoilsCB 
* ��    ��:��Ӧ��������:01 ����Ȧ eMBFuncReadCoils
*													05 д��Ȧ eMBFuncWriteCoil
*													15 д�����Ȧ eMBFuncWriteMultipleCoils
* ��ڲ���:pucRegBuffer: ���ݻ�����,������Ӧ����   
*						usAddress: ��Ȧ��ַ
*						usNCoils: Ҫ��д����Ȧ����
*						eMode: ������
* ���ڲ���:
* ע	  ��:��̵��� 
           FF00H������Ȧ����ON״̬��0000H������Ȧ����OFF״̬��
*						0 ��
****************************************************************************/
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
  //����״̬
  eMBErrorCode eStatus = MB_ENOERR;
  //�Ĵ�������
  int16_t iNCoils = ( int16_t )usNCoils;
  //�Ĵ���ƫ����
  int16_t usBitOffset;

  //���Ĵ����Ƿ���ָ����Χ��
  if( ( (int16_t)usAddress >= REG_COILS_START ) && ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
  {
    //����Ĵ���ƫ����
    usBitOffset = ( int16_t )( usAddress - ucRegCoilsStart );
    switch ( eMode )
    {
      //������
      case MB_REG_READ:
      while( iNCoils > 0 )
      {
        *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
        ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
        iNCoils -= 8;
        usBitOffset += 8;
      }
      break;

      //д����
      case MB_REG_WRITE:
      while( iNCoils > 0 )
      {
        xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
        ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),
        *pucRegBuffer++ );
        iNCoils -= 8;
      }
      break;
    }
  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}
/****************************************************************************
* ��	  ��:eMBRegDiscreteCB 
* ��    ��:��ȡ��ɢ�Ĵ���,��Ӧ��������:02 ����ɢ�Ĵ��� eMBFuncReadDiscreteInputs
* ��ڲ���:pucRegBuffer: ���ݻ�����,������Ӧ����   
*						usAddress: �Ĵ�����ַ
*						usNDiscrete: Ҫ��ȡ�ļĴ�������
* ���ڲ���:
* ע	  ��:1 ��
****************************************************************************/
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
  //����״̬
  eMBErrorCode eStatus = MB_ENOERR;
  //�����Ĵ�������
  int16_t iNDiscrete = ( int16_t )usNDiscrete;
  //ƫ����
  uint16_t usBitOffset;

  //�жϼĴ���ʱ�����ƶ���Χ��
  if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
  ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
  {
    //���ƫ����
    usBitOffset = ( uint16_t )( usAddress - ucRegDiscreteStart );

    while( iNDiscrete > 0 )
    {
      *pucRegBuffer++ = xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,
      ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );
      iNDiscrete -= 8;
      usBitOffset += 8;
    }

  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}
