/**
  ******************************************************************************
  * @file    mb_key.c
  * @brief   mb��������V1.0
  * @date    2021.01.10
  ******************************************************************************
  * @attention  ��MB��Ȧ����Ϊ����
  MBKEY����ʽ��1.��������һ���̣߳���ѯ��ʽѭ�����С� ���򵥣��ױ�д��
                 2.Ϊÿ����������һ���̣߳�MBKEY�����ź������߳��ڻָ�����
                 ��ά�����ѣ����������̹߳��ࡿ
  * @author HLY
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "mb_key.h"
/* Private includes ----------------------------------------------------------*/
#include "user_math.h"
#include "mb_handler.h"
#include "turn_motor.h"
#include "walk_motor.h"
#include "lifter_motor.h"
#include "adc_dma.h"
/*ulog include*/
#define LOG_TAG              "mb key"
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/
/** 
  * @brief  ����״̬��������״̬
  */ 
typedef enum _MBKEY_STATUS_LIST
{
  MBKEY_ENABLE,   // ʹ�ܣ���ֵΪ1
	MBKEY_DISENABLE,// ʧ�ܣ���ֵΪ0
	MBKEY_PRESS,    // ��������
	MBKEY_RAISE,    // ����̧��
}MBKEY_STATUS_LIST;
/** 
  * @brief  �������α�־
  */  
typedef FunctionalState MBKEY_ENABLE_STATUS;
/** 
  * @brief  ������ƽ״̬
  */  
typedef  GPIO_PinState IO_STATUS_LIST;
/** 
  * @brief  ״̬����ʼ������
  */  
typedef struct
{
	uint32_t GPIO_Pull;		//������������ģʽ
	uint16_t GPIOx;	      //������Ӧ�Ķ˿�
	uint16_t GPIO_Pin_x;	//����������
	uint8_t  key_nox;
}MBKey_Init;
/** 
  * @brief  ״̬����
  */
typedef struct _MBKEY_COMPONENTS
{
    MBKEY_ENABLE_STATUS 	MBKEY_SHIELD; 		//�������Σ�DISABLE(0):���Σ�ENABLE(1):������
    uint8_t             	MBKEY_TIMECOUNT;  //������������
    IO_STATUS_LIST 	    	MBKEY_FLAG;       //��־�������±�־
    IO_STATUS_LIST 	    	MBKEY_DOWN_LEVEL; //����ʱ������IOʵ�ʵĵ�ƽ
    MBKEY_STATUS_LIST     MBKEY_STATUS; 		//����״̬
    MBKEY_STATUS_LIST     MBKEY_EVENT;  		//�����¼�
    IO_STATUS_LIST (*MBREAD_PIN)(MBKey_Init Key);//��IO��ƽ����
}MBKEY_COMPONENTS;
/** 
  * @brief  ������
  */
typedef struct
{
	MBKey_Init 				MBKey_Board; // �̳г�ʼ������
	MBKEY_COMPONENTS 	MBKeyStatus; // �̳�״̬������
}MBKey_Config;
/* Private define ------------------------------------------------------------*/
#define MKKEY_HANDLE_NUM 4
/*����IO��ȡ��־*/
#define  LOW_LEVEL 	   GPIO_PIN_RESET
#define  HIGH_LEVER    GPIO_PIN_SET
/* Private macro -------------------------------------------------------------*/
/* �߳����� */
#define THREAD_PRIORITY      12//�߳����ȼ�
#define THREAD_TIMESLICE     10//�߳�ʱ��Ƭ
#define THREAD_STACK_SIZE    1024//ջ��С
/* Private variables ---------------------------------------------------------*/
MBKey_Config MBKey_Buf[MBKEY_NUM];	// ������������
SENSOR_state Stop_Button_state   = SENSOR_INIT;//��ͣ��ť״̬
uint16_t WDT_key = 0;//���Ź���������
/* Private function prototypes -----------------------------------------------*/
extern void MB_Param_Read(void);
extern void MB_Param_Save(void);
/**
  * @brief  ���´����¼�
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void MBKEY_ENABLE_Handle(uint8_t i)
{
  switch(i)
  {
    case KEY_SOFT_STOP:         //��ͣ
      TURN_ZERO_RESET;
      TURN_LOCATE_RESET;
      TURN_UP_RESET;
      TURN_DOWN_RESET;
      SMove_Stop(&turn.motor.Ctrl);
      USER_SET_BIT(turn.Stop_state,SOFT_STOP);
    
      WALK_BREAK_SET;
      USER_SET_BIT(walk.Stop_state,SOFT_STOP);
      break;
    case KEY_TURN_ZERO:         //ת������λ
      break;
    case KEY_TURN_LOCATE:       //ת������λ
      TURN_ZERO_RESET;
      TURN_UP_RESET;
      TURN_DOWN_RESET;
      turn.motor.Abs.set_angle = turn.set_radian * 180 / PI;
      SMove_SetAngle_Absolute(&turn.motor,turn.motor.Abs.set_angle);
      break;
    case KEY_TURN_UP:           //ת������㶯+
      TURN_ZERO_RESET;
      TURN_LOCATE_RESET;
      TURN_DOWN_RESET;
      SMove_SetRPM(&turn.motor,turn.motor.V.set_speed,DISABLE);
      break;
    case KEY_TURN_DOWN:         //ת������㶯-
      TURN_ZERO_RESET;
      TURN_LOCATE_RESET;
      TURN_UP_RESET;
      SMove_SetRPM(&turn.motor,-turn.motor.V.set_speed,DISABLE);
      break;
    case KEY_TURN_AXIS:         //ת����������
      break;
    case KEY_TURN_ENABLE:       //ת����ʹ��
      turn.motor.Ctrl.Motor_Enable();
      break;
    case KEY_WALK_BREAK:        //���ߵ��ɲ��
      Walk_Motor_Stop();
      break;
    case KEY_WALK_ENABLE:       //���ߵ��ʹ��
      walk.motor.Motor_Enable();
      break;
    case KEY_LIFT_UP:           //��۶���
      lift.limit_mode = 4;
      LIFT_DOWN_RESET;
      LIFT_LOCATE_RESET;
      LIFT_ZERO_RESET;
      Lifter_Motor_Jack(&lift);
      break;
    case KEY_LIFT_DOWN:         //����½�
      LIFT_UP_RESET;
      LIFT_LOCATE_RESET;
      LIFT_ZERO_RESET;
      Lifter_Motor_Fall(&lift);
      break;
    case KEY_LIFT_LOCATE:       //��۶�λ
      LIFT_UP_RESET;
      LIFT_DOWN_RESET;
      Lifter_Motor_Set_Target(&lift,lift.incoming);
      break;
    case KEY_LIFT_ZERO:         //��ۻ���
      break;
    case KEY_LIFT_SQP_SHIELD:   //��۽ӽ���������
      lift.SQP.state = SENSOR_LEAVE;   //���ñ�־
      LIFT_SQP_RESET;
      break;
    case KEY_LIFT_4MA_SET://����������4ma����
      break;
    case KEY_LIFT_20MA_SET://����������20ma����
      break;
    case KEY_FACTORY:           //�ָ���������
      break;
    case KEY_READ:              //��ȡEEPROM����
      break;
    case KEY_SAVE:              //����EEPROM����
      break;
    case KEY_TSET_WDT:          //���Ź�����
      break;
  }
}
/**
  * @brief  �ɿ������¼�
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void MBKEY_DISENABLE_Handle(u8 i)
{
  switch(i)
  {
    case KEY_SOFT_STOP:         //��ͣ
      break;
    case KEY_TURN_ZERO:         //ת������λ
      break;
    case KEY_TURN_LOCATE:       //ת������λ
      break;
    case KEY_TURN_UP:           //ת������㶯+
      if(turn.jog_flag == 1)
        SMove_SetRPM(&turn.motor,0,ENABLE);
      break;
    case KEY_TURN_DOWN:         //ת������㶯-
      if(turn.jog_flag == 2)
        SMove_SetRPM(&turn.motor,0,ENABLE);
      break;
    case KEY_TURN_AXIS:         //ת����������
      break;
    case KEY_TURN_ENABLE:       //ת������ʹ��
      turn.motor.Ctrl.Motor_Disenable();
      TURN_ZERO_RESET;
      TURN_LOCATE_RESET;
      TURN_UP_RESET;
      TURN_DOWN_RESET;
      SMove_Stop(&turn.motor.Ctrl);
      break;
    case KEY_WALK_BREAK:        //���ߵ��ɲ��
      Trapezoidal_Out_Speed(&walk.motor,walk.motor.set_speed);
      break;
    case KEY_WALK_ENABLE:       //���ߵ����ʹ��
      walk.motor.Motor_Disenable();
      WALK_BREAK_SET;
      break;
    case KEY_LIFT_UP:           //��۶���
      break;
    case KEY_LIFT_DOWN:         //����½�
      break;
    case KEY_LIFT_LOCATE:       //��۶�λ
      break;
    case KEY_LIFT_ZERO:         //��ۻ���
      break;
    case KEY_LIFT_SQP_SHIELD:   //��۽ӽ���������
      break;
    case KEY_LIFT_4MA_SET://����������4ma����
      break;
    case KEY_LIFT_20MA_SET://����������20ma����
      break;
    case KEY_FACTORY:           //�ָ���������
      break;
    case KEY_READ:              //��ȡEEPROM����
      break;
    case KEY_SAVE:              //����EEPROM����
      break;
    case KEY_TSET_WDT:          //���Ź�����
      break;
  }
}
/**
  * @brief  �ɿ��������¼�
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void MBKEY_PRESS_Handle(u8 i)
{
  switch(i)
  {
    case KEY_SOFT_STOP:         //��ͣ
      turn.limit_mode = 4;
      turn.jog_flag   = 4;
      MBKey_Shield_Operate(KEY_TURN_ZERO,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_DOWN,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_UP,DISABLE);     //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_LOCATE,DISABLE); //������ɨ�谴��
    
      WALK_BREAK_RESET; //�ɿ�ɲ��
      MBKey_Shield_Operate(KEY_WALK_BREAK,DISABLE); //������ɨ�谴��
      break;
    case KEY_TURN_ZERO:         //ת������λ
      if(Stop_Button_state != SENSOR_TOUCH)
      {
        //�����ʼ��״̬����ɻ��߳�ʼ�������ԭ����
        if(turn.motor.Do.init_state == 3 ||  turn.motor.Do.init_state == 0XFF)
        {
          turn.motor.Do.init_state = 1;
          /* �����߳�*/
          rt_thread_t thread = rt_thread_create( "turn  do",            /* �߳����� */
                                                 Turn_SMove_DoReset,    /* �߳���ں��� */
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
              LOG_E("turn  do thread created failed.");
          }
        }
      }
      break;
    case KEY_TURN_LOCATE:       //ת������λ
      turn.limit_mode = 4;
      turn.jog_flag   = 4;
      //���ð���
      MBKey_Shield_Operate(KEY_TURN_ZERO,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_DOWN,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_UP,DISABLE);     //������ɨ�谴��
      break;
    case KEY_TURN_UP:           //ת������㶯+
      turn.limit_mode = 1;
      turn.jog_flag   = 1;
      //���ð���
      MBKey_Shield_Operate(KEY_TURN_ZERO,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_DOWN,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_LOCATE,DISABLE); //������ɨ�谴��
      break;
    case KEY_TURN_DOWN:         //ת������㶯-
      turn.limit_mode = 2;
      turn.jog_flag   = 2;
      //���ð���
      MBKey_Shield_Operate(KEY_TURN_ZERO,DISABLE);   //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_UP,DISABLE);     //������ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_LOCATE,DISABLE); //������ɨ�谴��
      break;
    case KEY_TURN_AXIS:        //ת����������
      SMove_SetAxis_Reset(&turn.motor.Ctrl);
      TURN_AXIS_RESET;
      break;
    case KEY_TURN_ENABLE:      //ת����ʹ��
      break;
    case KEY_WALK_BREAK:       //���ߵ��ɲ��
      break;
    case KEY_WALK_ENABLE:      //���ߵ��ʹ��
      break;
    case KEY_LIFT_UP:           //��۶���
      lift.limit_mode = 1;
      //���ð���
      MBKey_Shield_Operate(KEY_LIFT_DOWN,DISABLE); //������ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_LOCATE,DISABLE); //������ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_ZERO,DISABLE); //������ɨ�谴��
      break;
    case KEY_LIFT_DOWN:         //����½�
      lift.limit_mode = 2;
      //���ð���
      MBKey_Shield_Operate(KEY_LIFT_UP,DISABLE); //������ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_LOCATE,DISABLE); //������ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_ZERO,DISABLE); //������ɨ�谴��
      break;
    case KEY_LIFT_LOCATE:       //��۶�λ
      lift.limit_mode = 3;
      //���ð���
      MBKey_Shield_Operate(KEY_LIFT_UP,DISABLE); //������ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_DOWN,DISABLE); //������ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_ZERO,DISABLE); //������ɨ�谴��
      break;
    case KEY_LIFT_ZERO:         //��ۻ���
      break;
    case KEY_LIFT_SQP_SHIELD:   //��۽ӽ���������
      break;
    case KEY_LIFT_4MA_SET://����������4ma����
      Rope_Encoder_Set_4mA();
      break;
    case KEY_LIFT_20MA_SET://����������20ma����
      Rope_Encoder_Set_20mA();
      break;
    case KEY_FACTORY:           //�ָ���������
      Modbus_Data_Init();
      DEFAULT_DATA_RESET;
      break;
    case KEY_READ:              //��ȡEEPROM����
      MB_Param_Read();
      READ_RESET;
      break;
    case KEY_SAVE:              //����EEPROM����
      MB_Param_Save();
      SAVE_RESET;
      break;
    case KEY_TSET_WDT:          //���Ź�����
      if(WDT_key == 2234)
      {
        rt_hw_us_delay(5000*1000);//5S��ʱ���������Ź�
      }
      break;
  }
}
/**
  * @brief  ���µ��ɿ��¼�
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void MBKEY_RAISE_Handle(u8 i)
{
  switch(i)
  {
    case KEY_SOFT_STOP:         //��ͣ
      if(turn.motor.Do.init_state == 3)//�޸�������ť���¸�λ���ڶ���ʱ���¼�ͣ�������ٴΰ���������ť��λ�������ڶ������� 2022.07.25 
        TURN_LOCATE_SET;//����ת������λ
      USER_CLEAR_BIT(turn.Stop_state,SOFT_STOP);
      MBKey_Shield_Operate(KEY_TURN_ZERO,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_UP,ENABLE);     //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_DOWN,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_LOCATE,ENABLE); //����ɨ�谴��
      
      WALK_BREAK_RESET;      //�ɿ�ɲ��
      USER_CLEAR_BIT(walk.Stop_state,SOFT_STOP);
      MBKey_Shield_Operate(KEY_WALK_BREAK,ENABLE);  //����ɨ�谴��
      break;
    case KEY_TURN_ZERO:         //ת������λ
      break;
    case KEY_TURN_LOCATE:       //ת������λ
      //���ð���
      MBKey_Shield_Operate(KEY_TURN_ZERO,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_UP,ENABLE);     //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_DOWN,ENABLE);   //����ɨ�谴��
      break;
    case KEY_TURN_UP:           //ת������㶯+
    case KEY_TURN_DOWN:         //ת������㶯-
      //���ð���
      MBKey_Shield_Operate(KEY_TURN_ZERO,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_UP,ENABLE);     //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_DOWN,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_TURN_LOCATE,ENABLE); //����ɨ�谴��
      break;
    case KEY_TURN_AXIS:         //ת����������
      break;
    case KEY_TURN_ENABLE:       //ת������ʹ��
      break;
    case KEY_WALK_BREAK:        //���ߵ��ɲ��
      break;
    case KEY_WALK_ENABLE:       //���ߵ��ʹ��
      break;
    case KEY_LIFT_UP:           //��۶���
      MBKey_Shield_Operate(KEY_LIFT_DOWN,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_LOCATE,ENABLE);     //����ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_ZERO,ENABLE);   //����ɨ�谴��
      break;
    case KEY_LIFT_DOWN:         //����½�
      MBKey_Shield_Operate(KEY_LIFT_UP,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_LOCATE,ENABLE);     //����ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_ZERO,ENABLE);   //����ɨ�谴��
      break;
    case KEY_LIFT_LOCATE:       //��۶�λ
      MBKey_Shield_Operate(KEY_LIFT_UP,ENABLE);   //����ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_DOWN,ENABLE); //����ɨ�谴��
      MBKey_Shield_Operate(KEY_LIFT_ZERO,ENABLE);   //����ɨ�谴��
      break;
    case KEY_LIFT_ZERO:         //��ۻ���
      break;
    case KEY_LIFT_SQP_SHIELD:   //��۽ӽ���������
      walk.limit_mode = 3;//�ָ�������������״̬
      break;
    case KEY_LIFT_4MA_SET://����������4ma����
      break;
    case KEY_LIFT_20MA_SET://����������20ma����
      break;
    case KEY_FACTORY:           //�ָ���������
      break;
    case KEY_READ:              //��ȡEEPROM����
      break;
    case KEY_SAVE:              //����EEPROM����
      break;
    case KEY_TSET_WDT:          //���Ź�����
      break;
  }
}
/** 
  * @brief  ��ȡIO��ƽ�ĺ���
  ������ȡ����
  */  
static IO_STATUS_LIST MBKEY_ReadPin(MBKey_Init Key)
{
  return (IO_STATUS_LIST)MB_GET_BIT(Key.GPIO_Pin_x);
}
/**
  * @brief  ��ȡ����ֵ
  * @param  None.
  * @retval None.
  * @note   ����ʵ�ʰ��°�ť�ĵ�ƽȥ�������������Ľ��
*/
static void Get_MBKEY_Level(void)
{
    uint8_t i;
    for(i = 0;i < MBKEY_NUM;i++)
    {
        if(MBKey_Buf[i].MBKeyStatus.MBKEY_SHIELD == DISABLE)	//��������򲻽��а���ɨ��
            continue;
        if(MBKey_Buf[i].MBKeyStatus.MBREAD_PIN(MBKey_Buf[i].MBKey_Board) == MBKey_Buf[i].MBKeyStatus.MBKEY_DOWN_LEVEL)
            MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG = LOW_LEVEL;
        else
            MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG = HIGH_LEVER;
    }
}
/**
  * @brief  ������������
  * @param  MBKey_Init
  * @retval None.
  * @note   ������������
*/
static void MBCreat_Key(MBKey_Init* Init)
{
	uint8_t i; 
  for(i = 0;i < MBKEY_NUM;i++)
	{
		MBKey_Buf[i].MBKey_Board = Init[i]; // MBKey_Buf��ť����ĳ�ʼ�����Ը�ֵ

		MBKey_Buf[i].MBKey_Board.key_nox = i;
		// ��ʼ����ť�����״̬������
		MBKey_Buf[i].MBKeyStatus.MBKEY_SHIELD = ENABLE;
		MBKey_Buf[i].MBKeyStatus.MBKEY_TIMECOUNT = 0;	
		MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG = LOW_LEVEL;
    
		if(MBKey_Buf[i].MBKey_Board.GPIO_Pull == GPIO_PULLUP) // ����ģʽ���и�ֵ
			MBKey_Buf[i].MBKeyStatus.MBKEY_DOWN_LEVEL = LOW_LEVEL;
		else
			MBKey_Buf[i].MBKeyStatus.MBKEY_DOWN_LEVEL = HIGH_LEVER;
    
		MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = 	MBKEY_DISENABLE;
		MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT	= 	MBKEY_DISENABLE;
		MBKey_Buf[i].MBKeyStatus.MBREAD_PIN 	= 	MBKEY_ReadPin;	//��ֵ������ȡ����
	}
}
/**
  * @brief  ��ȡ����
  * @param  None.
  * @retval None.
  * @note   ״̬����״̬ת��
*/
static void ReadMBKeyStatus(void)
{
  uint8_t i;
  Get_MBKEY_Level();
  for(i = 0;i < MBKEY_NUM;i++)
  {
    switch(MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS)
    {
      //״̬0�������Ϳ�
      case MBKEY_DISENABLE:
        if(MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG == LOW_LEVEL)
        {
            MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_DISENABLE;  //ת��״̬3
            MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT  = MBKEY_DISENABLE;  //���¼�
        }
        else
        {
          MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_PRESS;        //ת��״̬1
          MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT 	= MBKEY_PRESS;        //�����¼�
        }
        break;
			//״̬1����������
      case MBKEY_ENABLE:
        if(MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG == HIGH_LEVER)
        {
            MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_ENABLE;     //ת��״̬3
            MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT  = MBKEY_ENABLE;     //���¼�
        }
        else
        {
          MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_RAISE;        //ת��״̬0
          MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT  = MBKEY_RAISE;        //�����¼�
        }
        break;
      //״̬2����������[�Ϳ�������]
      case MBKEY_PRESS:
        if(MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG == HIGH_LEVER)
        {
            MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_ENABLE;     //ת��״̬3
            MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT  = MBKEY_ENABLE;     //���¼�
        }
        else
        {
            MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_DISENABLE;  //ת��״̬3
            MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT  = MBKEY_DISENABLE;  //���¼�
        }
        break;
			//״̬1����������[���µ��Ϳ�]
			case MBKEY_RAISE:
				if(MBKey_Buf[i].MBKeyStatus.MBKEY_FLAG == LOW_LEVEL)          //�����ͷţ��˿ڸߵ�ƽ
        {
            MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_DISENABLE;  //ת��״̬3
            MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT  = MBKEY_DISENABLE;  //���¼�
        }
				else
        {
            MBKey_Buf[i].MBKeyStatus.MBKEY_STATUS = MBKEY_ENABLE;     //ת��״̬3
            MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT = MBKEY_ENABLE;      //���¼�
        }
        break;
    }
  }
}
/**
  * @brief  ����ָ������.
  * @param  None.
  * @retval None.
  * @note   
ע�⺯����˳�������Խ������
https://blog.csdn.net/feimeng116/article/details/107515317
*/
static void (*MBKEY_Operation[MKKEY_HANDLE_NUM])(u8 i) = 
{ 
  MBKEY_ENABLE_Handle,
  MBKEY_DISENABLE_Handle,
  MBKEY_PRESS_Handle,
  MBKEY_RAISE_Handle,
};
/**
  * @brief  ������
  * @param  None.
  * @retval None.
  * @note   ���ڶ�ʱ��1msһ��
            ����ʼ��ֵ����MBģʽ������
            ɨ�谴���󣬶�ȡ����״̬���¼������ж�����
*/
void MBKey_Handler(void *p)
{
	uint8_t i;
  while(1)
  {
    rt_thread_mdelay(1);
    ReadMBKeyStatus();
    for(i = 0;i < MBKEY_NUM;i++)
    {
      //�ж�����Խ�� ָ���޷��ж�ָ�����ݴ�С
      //https://bbs.csdn.net/topics/80323809
      if(MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT <= MKKEY_HANDLE_NUM)
        MBKEY_Operation[MBKey_Buf[i].MBKeyStatus.MBKEY_EVENT](i);
    }
  }
}
/**
  * @brief  MBKEY�������β���
  * @param  num:MBKEY_LISTע�����ѡ��
  * @param  option: ENABLE  �����á�
                    DISABLE �����á�
  * @retval None.
  * @note   ���û�������
*/
void MBKey_Shield_Operate(uint8_t num,FunctionalState option)
{
  MBKey_Buf[num].MBKeyStatus.MBKEY_SHIELD   = option;
  MBKey_Buf[num].MBKeyStatus.MBKEY_EVENT    = MBKEY_DISENABLE;//�˳�ɲ���¼�
}
/**
  * @brief  IO��ʼ����ʼ��
  * @param  None.
  * @retval None.
  * @note   ״̬����ʼ��
GPIO_PULLUP����ʼ���ߣ��˿ڣ�λ��
*/
void MBKEY_Init(void)
{ 
	MBKey_Init MBKeyInit[MBKEY_NUM]=
	{ 
		{GPIO_PULLUP, 0, 1}, //�����ͣ
		{GPIO_PULLUP, 0, 2}, //ת������λ
		{GPIO_PULLUP, 0, 3}, //ת������λ
		{GPIO_PULLUP, 0, 4}, //ת������㶯+
		{GPIO_PULLUP, 0, 5}, //ת������㶯-
    {GPIO_PULLUP, 0, 6}, //ת����������
    {GPIO_PULLUP, 0, 7}, //ת����ʹ��
    {GPIO_PULLUP, 0, 8}, //���ߵ��ɲ��
    {GPIO_PULLUP, 0, 9}, //���ߵ��ʹ��
    {GPIO_PULLUP, 0, 10},//��۶���
    {GPIO_PULLUP, 0, 11},//����½�
    {GPIO_PULLUP, 0, 12},//��۶�λʹ��
    {GPIO_PULLUP, 0, 13},//��ۻ���
    {GPIO_PULLUP, 0, 14},//����������4ma����
    {GPIO_PULLUP, 0, 15},//����������20ma����
    {GPIO_PULLUP, 0, 16},//��۽ӽ���������
    {GPIO_PULLUP, 0, 24},//�ָ���������
    {GPIO_PULLUP, 0, 25},//��ȡEEPROM����
    {GPIO_PULLUP, 0, 26},//����EEPROM����
    {GPIO_PULLUP, 0, 27},//���Ź�����
	};
	MBCreat_Key(MBKeyInit);// ���ð�����ʼ������
}