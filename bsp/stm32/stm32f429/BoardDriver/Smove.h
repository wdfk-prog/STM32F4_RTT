/**
  ******************************************************************************
  * @file    Smove.c
  * @brief   �������V3.0
  * @date    2022.08.12
  ******************************************************************************
  * @attention  �ع�����������������
  7s�Ӽ��ٲ����������޸�
  �鿴C�ļ����������������ʽ����ѡ��
  
  ʹ�ò��裺1.����SMove_TypeDef����;
            2.ʵ��  SMove_Stop
                    Modify_Freq 
                    SMove_init 
                    SMove_Stop_Priority�����ȼ������ж�Ϊ1����ֹͣ���޷��ٴ�������
                    SMove_Direction
              �������ҹ���
            3.��ʼ��SMove_7s_TypeDef����������SMove_7S_Default_Config();��ʼ��Ĭ�ϲ���
            4.��ʼ��SMove_Initial����
   ���򿪽Ƕȶ�λʹ�� #define SMove_Angle_Control 1
            �����������⣬����ͨ��SMove_SetAngle_Range�������ýǶȷ�Χ��
            �����ýǶȷ�ΧĬ�ϲ��жϽǶȷ�Χ
            ��ͨ��SMove_GetAngle_Over������ȡ����Ƕ��Ƿ񳬳���Χ��
            
            ͨ��SMove_SetAngle_Absoluteʵ�־��ԽǶȼӼ��١�
            ��ɾ��ԽǶȼӼ��ٺ󣬾���ʹ��SMove_SetAngle_Cache����ǶȻ��档
            ����ʹ���ٶȺ�������ʹ�ýǶȺ���ʱ�����λ���һ�µ����޷��������Զ�λ��
            
            
   �����ٶ�ʹ�� #define SMove_Speed_Control 1
            �����������⣬����ͨ��SMove_SetAngle_Range���������ٶȷ�Χ��
            �����ýǶȷ�ΧĬ�ϲ��ж��ٶȷ�Χ
            ��ͨ��SMove_GetAngle_Over������ȡ�����ٶ��Ƿ񳬳���Χ��
            ͨ��SMove_Get_Speed��ȡ��ǰ�ٶ�
            
            ͨ��SMove_SetSpeedʵ���ٶȼӼ��١�
            
  * @author HLY
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SMOVE_H
#define __SMOVE_H
/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "sys.h"
#include "motor.h"

#define SMove_Angle_Control 1 //ʹ�ܽǶȿ���
#define SMove_Speed_Control 1 //ʹ���ٶȿ���
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  MOTOR �Ӽ���״̬ structures definition  
  */  
typedef enum 
{
  ACCEL,//����
  CONST,//����
  DECEL,//����
} Motor_PLSRstate;
/** 
  * @brief  MOTOR ��λ״̬ structures definition  
  */  
typedef enum 
{
  ZERO_INIT      = 0X00U,//��ʼ��
  QUICK_TOUCH    = 0X01U,//���ٽӽ�
  LOW_LEAVE      = 0x02U,//�����뿪
  ZERO_OK        = 0X03U,//���
  ZERO_ABNORMAL  = 0X04U,//�쳣
} ZERO_state;
/** 
  * @brief  MOTOR ��λ״̬ structures definition  
  */  
typedef enum 
{
  MONITOR_INIT      = 0X00U,//��ʼ��
  MONITOR_ZERO      = 0X01U,//��λ�쳣����
  MONITOR_LIMIT     = 0x02U,//��λ�쳣����
  MONITOR_NORMAL    = 0X03U,//����
  MONITOR_ABNORMAL  = 0X04U,//�쳣
  MONITOR_ALL       = 0X05U,//������
} LIMIT_MONITOR_STATE;
/** 
  * @brief  MOTOR ���Ʋ��� structures definition  
  */  
typedef struct 
{
  //д����Ϣ
  u16 reduction_ratio;              //������ٱ�
  GPIO_PinState clockwise;          //˳ʱ�뷽��Ĭ�ϵ�ƽ
  TIM_HandleTypeDef* Master_TIMx;   //����ʱ��
  u32 Channel;                      //��ʱ��ͨ��
  u32 cycle_pulse_num;              //����������������
  TIM_HandleTypeDef* Slave_TIMx;    //�Ӷ�ʱ��
  //��ѯ��Ϣ
  u32     set_freq;                 //����Ķ�ʱ��Ƶ��
  FunctionalState en;	              //ʹ��
  FunctionalState speedenbale;		  //�Ƿ�ʹ���ٶȿ���
  FunctionalState running;		  	  //ת����ɱ�־ 
  Directionstate dir;        //����
  //������Ϣ
  u32 pulsecount;                   //�Ը�Ƶ������������������
  u32 CurrentIndex;    	            //��ǰ���λ��
  u32 TargetIndex;    	            //Ŀ���ٶ��ڱ���λ��
  Motor_PLSRstate  state;          //�Ӽ���״̬
  u32 RevetDot;			  	 	          //����˶��ļ��ٵ�
  
  u32 StartTableLength;             //�������ݱ�
  u32 StopTableLength;              //ֹͣ���ݱ�
  
  u32 StartSteps;					          //�����������
  u32 StopSteps;					          //���ֹͣ����
  
  u32 PulsesGiven;			            //����˶����ܲ���
  u32 PulsesHaven;				          //����Ѿ����еĲ���
//  int32_t PulsesErr;                //���ʣ���˶�����
  
  u32 CurrentPosition;		          //��ǰλ��
  u32 MaxPosition;				          //���λ�ã�������λ����0
  
  u32 CurrentPosition_Pulse;        //��ǰλ���������
  u32 MaxPosition_Pulse;		        //���λ���������
  
  u32 Slave_Pulse_IT;               //�Ӷ�ʱ������жϼ���
  u32 Slave_Pulse_Now;		          //�Ӷ�ʱ����ǰ��¼���������
  u32 Slave_Pulse_Last;		          //�Ӷ�ʱ����һ�μ�¼���������
  
  u32 *Counter_Table;  		          //ָ������ʱ��ʱ�����������
  u32 *Step_Table;  			          //ָ������ʱ��ÿ��Ƶ�����������
  
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
} SMove_Control_TypeDef;
#if (SMove_Angle_Control == 1)
/** 
  * @brief  MOTOR �ǶȲ��� structures definition  
  */  
typedef struct
{
  int16_t max_angle;                    //���Ƕ�
  int16_t min_angle;                    //��С�Ƕ�
  
  float   set_angle;                    //����Ƕ�
  float   get_angle;                    //�����Ƕ�
  
  u32     dest_position;                //Ŀ��λ��
  
  FunctionalState over_range;           //�Ƿ񳬳���Χ��־    ENABLE:������DISABLE���޳���
  
  float last;                           //��һ�νǶ�
  float err;                            //�Ƿ��и���     ���ǶȲΪ0ʱ��֤���нǶȱ仯�����¼���Ӽ����б�
}SMove_AngleTypeDef;
#endif
#if (SMove_Speed_Control == 1)
/** 
  * @brief  MOTOR �ٶȲ��� structures definition  
  */  
typedef struct
{
  u16     max_speed;                    //�������
  u16     min_speed;                    //��С����
  int16_t set_speed;                    //�����ٶ�[������]
  int32_t get_speed;                    //������ٶ�[������]
}SMove_SpeedTypeDef;
#endif
/** 
  * @brief  MOTOR 7s�Ӽ��ٲ��� structures definition  
  */  
typedef struct
{
  uint8_t  STEP_PARA;				              //����ʱ��ת�������������ӡ�Ӱ��ʱ����ϸ�ֶȣ�ԽСʱ��Խ����Խϸ�֡�
  uint16_t STEP_AA;						            //�Ӽ��ٽ׶Σ���ɢ��������Խ��Խϸ�֣���Ӱ��ʱ�䡿
  uint16_t STEP_UA;				                //�ȼ��ٽ׶Σ���ɢ������
  uint16_t STEP_RA;			                  //�����ٽ׶Σ���ɢ������

  uint16_t fstart;                        //���������Ƶ��
  uint32_t faa;                           //���Ƶ�ʵļӼ��ٶȡ���������ٶ����ޡ�
  float taa;                              //���Ƶ�ʵļӼ���ʱ��
  float tua;                              //���Ƶ�ʵ��ȼ���ʱ��
  float tra;                              //���Ƶ�ʵļ�����ʱ��  
  
  uint16_t STEP_LENGTH;  //(STEP_AA + STEP_UA + STEP_RA)//�ܲ���
  
  u32 *MotorTimeTable;  		              //ָ������ʱ��ʱ�����������
  u32 *MotorStepTable;  			            //ָ������ʱ��ÿ��Ƶ�����������
}SMove_7s_TypeDef;
/** 
  * @brief  MOTOR ��ԭ���� structures definition  
  */  
typedef struct
{
  int32_t speed_high;     //��ԭ����
  int32_t speed_low;      //��ԭ����
  ZERO_state   flag;     //����״̬
  SENSORTypeDef zero;     //��λ������
  SENSORTypeDef limit;    //��λ������
  float   init_freq;      //��λ�Ƕ�
  uint8_t init_state;     //0XFFΪ����״̬;
}SMove_DoSetTypeDef;
/** 
  * @brief  MOTOR���� structures definition  
  */  
typedef struct
{
  SMove_Control_TypeDef Ctrl;              //���Ʋ���
  SMove_7s_TypeDef      CFG;               //�Ӽ��ٲ���
#if (SMove_Angle_Control == 1)
  SMove_AngleTypeDef    Rel;               //��ԽǶȲ���
  SMove_AngleTypeDef    Abs;               //���ԽǶȲ���
#endif
#if (SMove_Speed_Control == 1)
  SMove_SpeedTypeDef    V;                 //�ٶȲ���
  SMove_DoSetTypeDef    Do;                //��ԭ����
#endif
}SMove_TypeDef;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
extern void SMove_Stop(SMove_Control_TypeDef* p);                                 //���ֹͣ����
extern void SMove_Handler(SMove_Control_TypeDef* p);                        //����us��ʱ����
extern void SMove_Reinitial(SMove_Control_TypeDef *pmotor);                        //���³�ʼ�����Ʋ���
extern void SMove_Initial(SMove_Control_TypeDef *pmotor,SMove_7s_TypeDef *p,
uint32_t Cycle_pulse_num,uint16_t Reduction_ratio,TIM_HandleTypeDef* Master_timx,
uint32_t HChannel,GPIO_PinState Clockwise,uint32_t *SMove_TimeTable,uint32_t *SMove_StepTable,
TIM_HandleTypeDef* Slave_timx);                                                     //��ʼ���Ӽ��ٵ��
#if (SMove_Speed_Control == 1)  //���Ƶ������ָ���ٶȺ���
extern MOVE_state SMove_SetRPM(SMove_TypeDef * p,int16_t rpm,FunctionalState Stop); //��RPM��ʽ�����ٶ�
extern MOVE_state SMove_SetSpeed(SMove_TypeDef *p,int8_t SpeedIndex,FunctionalState Stop);//�����ٶ�
extern ZERO_state SMove_DoReset(SMove_TypeDef *p,int32_t zero_max,int32_t zero_min,SENSOR_state zero_state, SENSOR_state limit_state);//��ԭ����
extern LIMIT_MONITOR_STATE SMove_Limit_Detection(SMove_TypeDef* p,uint8_t mode,SENSOR_state zero_state, SENSOR_state limit_state);    //�����غ���
extern void SMove_SetSpeed_Range(SMove_TypeDef *p,uint32_t max,uint32_t min);        //�����ٶȷ�Χ
extern int32_t SMove_Get_Speed(SMove_Control_TypeDef* p);                            //��ȡ�ٶ�
#endif
#if (SMove_Angle_Control == 1)  //���Ƶ������ָ���ǶȺ���
extern FunctionalState SMove_SetAngle_Relative(SMove_TypeDef *p,float relative_angle);         //������ԽǶ�
extern FunctionalState SMove_SetAngle_Absolute(SMove_TypeDef *p,volatile float absolute_angle);//���ƾ��ԽǶ�
extern void SMove_SetAngle_Range(SMove_AngleTypeDef *p,float max,float min);        //���ýǶȷ�Χ
extern void SMove_SetAngle_Cache(SMove_AngleTypeDef *p);                            //����ǶȻ���
extern void SMove_SetAxis_Reset(SMove_Control_TypeDef *p);                          //��������еλ��
extern FunctionalState SMove_GetAngle_Over(SMove_AngleTypeDef *p);                  //��ѯ�Ƕ��Ƿ񳬳���Χ
extern float SMove_Get_Angle(SMove_Control_TypeDef* p);                             //��ȡ��ǰ�Ƕ� �л���
#endif
#endif /* __SMOVE_H */