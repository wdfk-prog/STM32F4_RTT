/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : adc_dma.c
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
    ����CACHE���� ��Ҫ����IRAM1��0x20000000��ʼ
                           ʹ�ô�0x20200000 �ᵼ��DMA����   2022.03.21
  * ADCת������Խ�� ������Խ�����׶�ʧ����
  * �Ǳ�Ҫ������ʹ��������ڲ���
  * F7 �������� EOC flag at the end of all conversions�����������׼ȷ2022.04.16
    ʵ��LED������˸������ʵ��������ָ�ԭ����˸״̬                     2022.04.18
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "adc_dma.h"
/* Private includes ----------------------------------------------------------*/
#include "adc.h"
#include "mb_handler.h"
#include "turn_motor.h"
#include "walk_motor.h"
/* Private typedef -----------------------------------------------------------*/
/** 
  * @brief ADC1ͨ��ֵ����
  */  
typedef enum
{
  ADC_Channel = 0x00,
  Vrefint_Channel,
  Encoder_Channel,
}ADC1_CHANNEL;
/* Private define ------------------------------------------------------------*/
#define ADC1_CHANNEL_NUM 3  //ADC1����2ͨ��
#define ADC1_BUFFER_SIZE 8 //ÿ��ͨ����32��, ������ƽ��
#define ADC1_BUFFER_LEN  ADC1_BUFFER_SIZE*ADC1_CHANNEL_NUM
#define VABBT_LONG_TIME  30//�͵�ƽ����ʱ���ж�
/* Private macro -------------------------------------------------------------*/
/*
@Note If the  application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible
      by the Cortex M7 and the  MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs.
      If the application needs to use DMA(or other masters) based access or requires more RAM, then  the user has to:
              - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000)
              - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters(DMAs,DMA2D,LTDC,MDMA).
              - The addresses and the size of cacheable buffers (shared between CPU and other masters)
                must be	properly defined to be aligned to L1-CACHE line size (32 bytes). 
*/
//32�ֽڶ���(��ַ+��С)
//adc1_dataָ���� AXI SRAM ��0x20020000 //���ò���ȷ�������debug
//adc3_dataָ���� SRAM4 ��0x38000000    //
//ALIGN_32BYTES (__IO uint16_t adc1_data[ADC1_BUFFER_SIZE]) __attribute__((section(".ARM.__at_0x20020000")));
//ALIGN_32BYTES (uint16_t adc3_data[ADC3_BUFFER_SIZE]) __attribute__((section(".ARM.__at_0x38000000")));
/* Private variables ---------------------------------------------------------*/
uint16_t adc1_data[ADC1_BUFFER_LEN];
uint16_t Vrefint_vaule = 0;   //�ڲ��ο���ѹֵ
ADC_Monitor_TypeDef   voltage;//��ص�ѹ
Rope_Encoder_TypeDef  lifter_encoder; //4-20ma����������
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ��ȡоƬ�ڲ��ο���ѹADC����ֵ
  * @param  None
  * @retval None
  * @note   �ڲ���ѹ���ȶ�����ƽ��ֵ�ȿɣ������˲�������ʱ
*/

static float Get_Vrefint_Value(void)
{
  float    Vrefint_K     = 0;//ת��ϵ�� Լ���� 3.3 / 4095
  uint32_t sum           = 0;
  for(int i = Vrefint_Channel; i < ADC1_BUFFER_LEN; i += ADC1_CHANNEL_NUM)
  {
    sum += adc1_data[i];
  }
  Vrefint_vaule = sum / ADC1_BUFFER_SIZE;
  
  Vrefint_K = 1.2 / Vrefint_vaule;//�ڲ���ѹ�ο�ֵΪ1.2V
  
  return Vrefint_K;
}
/**
  * @brief  ����adcֵ
  * @param  None
  * @retval None
  * @note   None
*/

static float Adc_Get_Value(void)
{
  float adc_value = 0;
  for(int i = ADC_Channel; i < ADC1_BUFFER_LEN; i += ADC1_CHANNEL_NUM)
  {
    adc_value = First_Order_Lag(&voltage.AD.FOL,adc1_data[i] * Get_Vrefint_Value());
  }
  return adc_value;
}
/**
  * @brief  ��ر������
  * @param  None
  * @retval None
  * @note   ���붨ʱ�ж�1000MSһ��
*/
static void Battery_Alarm_Detection(void)
{
  static uint16_t fifter_count = 0;
  voltage.AD.vaule = Adc_Get_Value() + voltage.AD.Compensation;
  if(voltage.AD.vaule <= (voltage.VPT) / ADC_RATIO)
  {
    if(++fifter_count > VABBT_LONG_TIME)//�͵�������30�뱨��
    {
      //����
      ADC_ALARM_SET;//ADC����λ
      fifter_count = 0;
      USER_SET_BIT(turn.Stop_state,VBATT_STOP);
      USER_SET_BIT(walk.Stop_state,VBATT_STOP);
    }
  }
  else if(voltage.AD.vaule >= (voltage.VPT + 0.5f) / ADC_RATIO)
  {
    fifter_count = 0;
    ADC_ALARM_RESET;//ADC����λ
    USER_CLEAR_BIT(turn.Stop_state,VBATT_STOP);
    USER_CLEAR_BIT(walk.Stop_state,VBATT_STOP);
  }
}
/***********************************4-20ma����������******************************************/
/**
  * @brief  ��ʼ������������
  * @param  None
  * @retval None
  * @note   None
*/
static void Rope_Encoder_Init(void)
{
  lifter_encoder.Current.AD.FOL.K = 0.9;
  lifter_encoder.state = SENSOR_INIT;
  lifter_encoder.height_4ma  = ROPE_4MA_HEIGHT; //��λmm
  lifter_encoder.height_20ma = ROPE_20MA_HEIGHT;//��λmm
  lifter_encoder.Current.AD.Compensation = 0.01;
}
/**
  * @brief  ��������������4maλ��.
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Rope_Encoder_Set_4mA(void)
{
  HAL_GPIO_WritePin(SET4MA_GPIO_Port,SET4MA_Pin,GPIO_PIN_SET);
  lifter_encoder.state = SENSOR_INIT;
}
/**
  * @brief  ��������������4maλ��.
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Rope_Encoder_Set_20mA(void)
{
  HAL_GPIO_WritePin(SET20MA_GPIO_Port,SET20MA_Pin,GPIO_PIN_SET);
  lifter_encoder.state = SENSOR_INIT;
}
/**
  * @brief  ��������������λ�����
  * @param  None.
  * @retval None.
  * @note   ����1mһ��ѭ��.
*/
static void Rope_Encoder_Set_Low(void)
{
  HAL_GPIO_WritePin(SET4MA_GPIO_Port,SET4MA_Pin,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SET20MA_GPIO_Port,SET20MA_Pin,GPIO_PIN_RESET);
  LIFT_ENCODER_4MA_RESET;
  LIFT_ENCODER_20MA_RESET;
}
/**
  * @brief  ��������������adcֵ
  * @param  None
  * @retval None
  * @note   None
*/
static float Rope_Encoder_ADC_Get_Value(void)
{
  float adc_value = 0;
  for(int i = Encoder_Channel; i < ADC1_BUFFER_LEN; i += ADC1_CHANNEL_NUM)
  {
    adc_value = First_Order_Lag(&lifter_encoder.Current.AD.FOL,adc1_data[i] * Get_Vrefint_Value());
  }
  return adc_value + lifter_encoder.Current.AD.Compensation;
}
/**
  * @brief  ������������������ֵ
  * @param  None
  * @retval None
  * @note   4~20maת��Ϊ��ѹ640mV~3200mV
VOUT / ILOOP = K3 * (R5 * R3) / R1 + R3
             = ��200k * 30��/ 1.54k + 30
             �� 160    ���������޸ģ�����Ӳ���ɵ�����
K3 = K2 / K1 = Constant = 1
��ѹ��Χ��0.64~3.26V
*/
static float Rope_Encoder_Get_Current(void)
{
  lifter_encoder.Current.AD.vaule = (Rope_Encoder_ADC_Get_Value());//��ѹ
  return lifter_encoder.Current.AD.vaule * 1000000 / 160;//����(uA)
}
/**
  * @brief  ��������������λ��
  * @param  None
  * @retval None
  * @note   4~20ma������ 16ma����
*/
void Rope_Encoder_Get_Postion(void)
{
  uint16_t range = lifter_encoder.height_20ma - lifter_encoder.height_4ma;
  
  if(range <= 0)
  {
    //���� �������������
    lifter_encoder.state = SENSOR_LEAVE;
    return;
  }
  
  float dpi = range / 16000.0f;//�ֱ��� λ�÷�Χ / 16000
  
  lifter_encoder.Current.value = Rope_Encoder_Get_Current();
 
  if(0 <= lifter_encoder.Current.value  && lifter_encoder.Current.value < 2000)//0~4ma֮����Ϊ���� �� 1ma��Χ
  {
    //���� ����������
    lifter_encoder.state = SENSOR_TIMEOUT;
    LIFT_ENCODER_RESET;
    LIFT_ENCODER_ALARM_SET;
  }
  else if(2000 <= lifter_encoder.Current.value  && lifter_encoder.Current.value <= 22000)//һ����Ϊ2ma~22ma֮��Ϊ����
  {
    //����������
    lifter_encoder.state = SENSOR_TOUCH;
    LIFT_ENCODER_SET;
    if(lifter_encoder.Current.value < 4000)//�߶����Ϊ0���޸�����С��4ma��Ϊ0�߶�
      lifter_encoder.Current.value = 4000;
    lifter_encoder.position = (dpi * (lifter_encoder.Current.value - 4000) + lifter_encoder.height_4ma);
  }
  else
  {
    //���� ��������·����
    lifter_encoder.state = SENSOR_LEAVE;
    LIFT_ENCODER_RESET;
    LIFT_ENCODER_ALARM_SET;
  }
}
/**
  * @brief  ADC 1S.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void adc_1s(void* p)
{
  Rope_Encoder_Set_Low();
  Battery_Alarm_Detection();
}

/**
  * @brief  adc_dma��ʼ��
  * @param  None
  * @retval None
  * @note   �������ڳ�ʼ���׶�
*/
int ADC_DMA_Init(void)
{
  MX_ADC1_Init();
  if(HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_data, ADC1_BUFFER_LEN) != HAL_OK)
  {
    Error_Handler();
  }
  voltage.AD.FOL.K = 0.5;
  Rope_Encoder_Init();
  rt_err_t ret = RT_EOK;
  /* ��ʱ���Ŀ��ƿ� */
  static rt_timer_t timer;
  /* ������ʱ�� 1  ���ڶ�ʱ�� */
  timer = rt_timer_create("ADC 1s", adc_1s,
                           RT_NULL, rt_tick_from_millisecond(1000),
                           RT_TIMER_FLAG_PERIODIC);

  /* ������ʱ�� */
  if (timer != RT_NULL) 
  {
    rt_timer_start(timer);
    ret = RT_EOK;
  }
  else
  {
    ret = RT_ERROR;
  }
  return ret;
}
INIT_APP_EXPORT(ADC_DMA_Init);
/**
  * @brief  ADC_DMA �����ص�����
  * @param  None
  * @retval None
  * @note   ADCת�������ж��а����ݴ浽�����ǰ�벿��
            ping-pong�洢
            FIFO
*/
//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
//{
//  /* Invalidate Data Cache to get the updated content of the SRAM on the first half of the ADC converted data buffer */
//  if(hadc->Instance == ADC1) {
//      SCB_InvalidateDCache_by_Addr((uint32_t *) &adc1_data[0], ADC1_BUFFER_LEN);
//  } 
//}
/**
  * @brief  ADC_DMA ת����ɻص�����
  * @param  None
  * @retval None
  * @note   ADCת������ж��а����ݴ浽����ĺ�벿��
*/
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//   /* Invalidate Data Cache to get the updated content of the SRAM on the second half of the ADC converted data buffer */
//   if(hadc->Instance == ADC1) 
//  {
//       SCB_InvalidateDCache_by_Addr((uint32_t *) &adc1_data[ADC1_BUFFER_LEN/2], ADC1_BUFFER_LEN);
//   } 
//}