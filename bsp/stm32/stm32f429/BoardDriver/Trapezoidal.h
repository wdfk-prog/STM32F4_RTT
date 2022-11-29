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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRAPEZOIDAL_H
#define __TRAPEZOIDAL_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "motor.h"
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  Motor �Ӽ��ٲ��� structures definition  
  */  
typedef struct
{
  uint16_t step;       //�Ӽ��ٲ���
  uint16_t time;       //��������
  
  int32_t target;      //Ŀ��ֵ
  int32_t last_target; //��һ��Ŀ��ֵ
  int32_t set_value;   //�趨ֵ
}Motor_ANDTypeDef;
/** 
  * @brief  Motor ���� structures definition  
  */  
typedef struct
{
  /*��Ҫ��ʼ���Ĳ���*/
  u32                 cycle_pulse_num;             //����������������
  u16                 reduction_ratio;             //������ٱ�
  int16_t             max_speed;                   //�������
  int16_t             min_speed;                   //��С����
  int16_t             set_speed;                   //�����ٶ�[������][RPM]
  float               get_speed;                   //������ٶ�
  u32                 set_freq;                    //����Ƶ��
  int32_t             get_freq;                    //����Ƶ��
  TIM_HandleTypeDef*  TIMx;                        //��ʱ��
  u32                 Channel;                     //��ʱ��ͨ��
  GPIO_PinState       clockwise;                   //˳ʱ�뷽��Ĭ�ϵ�ƽ
  Directionstate dir;                             //����
  Motor_ANDTypeDef    AND;                         //�Ӽ��ٲ���
  
  void (*Motor_Enable)(void);       //���ʹ�ܺ���
  void (*Motor_Disenable)(void);    //������ú���
  void (*Motor_Start)(void);        //�����������
  void (*Motor_Stop) (void);        //���ֹͣ����
  void (*Motor_Direction)(Directionstate dir);//���������
  void (*Motor_init) (TIM_HandleTypeDef* TIMx);//�����ʼ������
  /*����true,ֹͣ������˳����ƺ���
    ����FALSE,�������п��ƺ���
    ��֤״̬�쳣ʱ���޷��������õ��
  */
  uint8_t (*Motor_Stop_Priority)(void);//���ֹͣ���ȼ�����
  PWM_StatusTypeDef (*Modify_Freq)(TIM_HandleTypeDef* TIMx,u32 Channel, uint32_t _ulFreq);//�޸�Ƶ�ʺ���
}Trapezoidal_TypeDef;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
extern void Trapezoidal_Init(Trapezoidal_TypeDef *p,u16 reduction_ratio,
int16_t max_speed,int16_t min_speed,TIM_HandleTypeDef* TIMx, u32 Channel,
uint16_t time,uint16_t step,GPIO_PinState clockwise,u32 cycle_pulse_num);   //���μӼ��ٳ�ʼ��
extern MOVE_state Trapezoidal_Out_Speed(Trapezoidal_TypeDef* p,int16_t v); //�����ٶ�ֵ
extern MOVE_state Trapezoidal_AND(Trapezoidal_TypeDef* p);                 //�Ӽ��ٹ������� �����붨ʱ������
#endif /* __TRAPEZOIDAL_H */