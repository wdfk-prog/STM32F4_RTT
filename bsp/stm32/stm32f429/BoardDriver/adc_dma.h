/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : adc_dma.h
  * @brief          : adc_dma program body
  ******************************************************************************
  * @attention
  * F7 ����L1-cache�ᵼ��DMA���ݲ����¡����紮�ڣ�adc
  * https://bbs.21ic.com/icview-2380632-1-1.html
  * �������Ĭ���ڴ��ַ����Ϊ0x20020000��ʼ����������SRAM1���Ż������������
  * ���Ĭ���ڴ��ַΪ0x20000000��ʼ����������DTCM������������ģ�����D-cache������
  * DMA���ʹ����SRAM1��Ҫô�Ͳ�ʹ��D-cache��Ҫʹ��D-cache��Ҫ�ֶ�clean����������Ϊwrite-through
  * ��һ�ֽ���취�籾������ʾ��
  * https://blog.csdn.net/weifengdq/article/details/121802176
  ʵ��LED������˸������ʵ��������ָ�ԭ����˸״̬                     2022.04.18
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_DMA_H
#define __ADC_DMA_H
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "led.h"
#include "user_math.h"
#include "motor.h"
#include "stm32f4xx_it.h"
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief ADC�����������
  */  
typedef struct
{
  float Compensation;//����ֵ
  float vaule;       //adc����ֵ
  FOL_TypeDef FOL;
}ADC_Cal_TypeDef;
/** 
  * @brief ADC���Ӷ���
  */  
typedef struct
{
  ADC_Cal_TypeDef AD;
  volatile  float       VPT;        //������ֵ
}ADC_Monitor_TypeDef;
/** 
  * @brief ģ��ɼ�����������
  */  
typedef struct
{
  ADC_Cal_TypeDef AD; //��ѹ�ṹ��
  float value;        //����ֵ(mA)
}ADC_Encoder_TypeDef;
/** 
  * @brief ��������������
  */  
typedef struct
{
  ADC_Encoder_TypeDef Current;
  uint16_t position;
  SENSOR_state state; //SENSOR_TOUCHΪ����״̬
  uint16_t height_4ma; //4maλ�ø߶�, ��λmm [100mm]
  volatile uint16_t height_20ma;//20maλ�ø߶�,��λmm [2900mm]
}Rope_Encoder_TypeDef;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
//���ۼ����ѹϵ��Ϊ7.0606 ʵ�ʲ�����ҪΪ7.2׼ȷ
#define ADC_RATIO 11.0f //((10+1)/ 1))
/* Exported variables ---------------------------------------------------------*/
extern ADC_Monitor_TypeDef voltage;
extern volatile uint8_t voltage_count;
extern uint16_t Vrefint_vaule;
extern Rope_Encoder_TypeDef lifter_encoder; //4-20ma����������
/* Exported functions prototypes ---------------------------------------------*/
extern void Rope_Encoder_Get_Postion(void);
extern void Rope_Encoder_Set_4mA(void);
extern void Rope_Encoder_Set_20mA(void);
#endif /* __ADC_DMA_H */
