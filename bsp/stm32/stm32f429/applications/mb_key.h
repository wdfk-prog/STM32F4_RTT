/**
  ******************************************************************************
  * @file    mb_key.h
  * @brief   mb��������V1.0
  ******************************************************************************
  * @attention  None.
  * @author HLY
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MB_KEY_H
#define __MB_KEY_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  ����ע���
  */
typedef enum
{
	KEY_SOFT_STOP,                //��ͣ
  KEY_TURN_ZERO,                //ת������λ
  KEY_TURN_LOCATE,              //ת������λ
  KEY_TURN_UP,                  //ת������㶯+  
  KEY_TURN_DOWN,                //ת������㶯-  
  KEY_TURN_AXIS,                //ת����������  
  KEY_TURN_ENABLE,              //ת����ʹ��
  KEY_WALK_BREAK,               //���ߵ��ɲ��
  KEY_WALK_ENABLE,              //���ߵ��ʹ��
  KEY_LIFT_UP,                  //��۶���
  KEY_LIFT_DOWN,                //����½�
  KEY_LIFT_LOCATE,              //��۶�λ
  KEY_LIFT_ZERO,                //��ۻ���
  KEY_LIFT_SQP_SHIELD,          //��۽ӽ���������
  KEY_LIFT_4MA_SET,             //����������4ma����
  KEY_LIFT_20MA_SET,            //����������20ma����
  KEY_FACTORY,                  //�ָ���������
  KEY_READ,                     //��ȡEEPROM����
  KEY_SAVE,                     //����EEPROM����
  KEY_TSET_WDT,                 //���Ź�����
  MBKEY_NUM,// ����Ҫ�еļ�¼��ť���������������
}MBKEY_LIST;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern uint16_t WDT_key;
/* Exported functions prototypes ---------------------------------------------*/
extern void MBKEY_Init(void);//IO��ʼ��
extern void MBKey_Handler(void *p);//����������
extern void MBKey_Shield_Operate(uint8_t num,FunctionalState option);
#endif /* __MB_KEY_H */
