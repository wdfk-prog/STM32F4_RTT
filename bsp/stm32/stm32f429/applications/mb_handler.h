/**
  ******************************************************************************
  * @file    demo.h
  * @brief   FREEMODBUS�Զ���
  ******************************************************************************
  * @attention  
  * @author HLY
  �����������������Ҫ���� �����������
  
  �Ż�ת����˳ʱ���ƽ����ѡ��                         2022.07.20
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MB_HANDLER_H
#define __MB_HANDLER_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "user_math.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
//����Ĵ�����ʼ��ַ
#define REG_INPUT_START 0
//����Ĵ�������
#define REG_INPUT_NREGS 1000
//���ּĴ�����ʼ��ַ
#define REG_HOLDING_START 0
//���ּĴ�������
#define REG_HOLDING_NREGS 1000
//��Ȧ��ʼ��ַ
#define REG_COILS_START 0
//��Ȧ����
#define REG_COILS_SIZE 160 //8�ı���
//���ؼĴ�����ʼ��ַ
#define REG_DISCRETE_START 0
//���ؼĴ�������
#define REG_DISCRETE_SIZE 160//8�ı���

#define Turn_Cycle_Pulse_Num     20000         //һ������Ҫ������
#define Turn_REDUCTION_RATIO     1             //���ٱ�
#define Turn_CLOCKWISE           GPIO_PIN_RESET//ת���������ƽֵ
#define Turn_ANGLE_MAX           180           //���Ƕ� ��λ�Ƕ�
#define Turn_ANGLE_MIN           -180          //��С�Ƕ� ��λ�Ƕ�
#define Turn_SPEED_MAX           1000          //����ٶ� ��λRPM
#define Turn_SPEED_MIN           0             //��С�ٶ� ��λRPM
#define Turn_SPEED_JOG           5             //�㶯�ٶ� ��λRPM
#define Turn_SANGLE_INIT         1169//-66.98  //��λ�Ƕ� ��λ����
#define Turn_BACK_HIGH_SPEED     -16           //��ԭ���� ��λRPM
#define Turn_BACK_LOW_SPEED      1             //��ԭ���� ��λRPM

#define Walk_RESOLUTION          1024
#define Walk_Cycle_Pulse_Num     Walk_RESOLUTION * 4//һ������Ҫ������ = �ֱ��ʱ����� * 4��Ƶ
#define Walk_REDUCTION_RATIO     1             //���ٱ�
#define Walk_CLOCKWISE           GPIO_PIN_RESET//ת���������ƽֵ
#define Walk_SPEED_MAX           1000          //����ٶ� ��λRPM
#define Walk_SPEED_MIN           0             //��С�ٶ� ��λRPM
#define Walk_STEP                100           //����     ��λRPM
#define Walk_TIME                1             //����     ��λMS

#define ROPE_4MA_HEIGHT   0   
#define ROPE_20MA_HEIGHT  2130 
/* Exported macro ------------------------------------------------------------*/
#define ushort uint16_t //ǿ��ת���޷�������
#define _short int16_t  //ǿ��ת���з�������
#define _float(x) ((float)((int16_t)(x)))
#define MB_SET_BIT(x)    USER_SET_BIT  (ucRegCoilsBuf[(uint8_t)((x) / 8)],(x - (uint8_t)((x) / 8) * 8))
#define MB_RESET_BIT(x)  USER_CLEAR_BIT(ucRegCoilsBuf[(uint8_t)((x) / 8)],(x - (uint8_t)((x) / 8) * 8))
#define MB_GET_BIT(x)    USER_GET_BIT  (ucRegCoilsBuf[(uint8_t)((x) / 8)],(x - (uint8_t)((x) / 8) * 8))
/*******************����������********************************/
/*******************0X01~0X23*********************************/
#define SOFT_STOP_SET             MB_SET_BIT  (1)  //��ͣ��ť����
#define SOFT_STOP_RESET           MB_RESET_BIT(1)  //��ͣ��ť�ɿ�

#define TURN_ZERO_SET             MB_SET_BIT  (2)  //ת������λ����
#define TURN_ZERO_RESET           MB_RESET_BIT(2)  //ת������λ����

#define TURN_LOCATE_SET           MB_SET_BIT  (3)  //ת������λ����
#define TURN_LOCATE_RESET         MB_RESET_BIT(3)  //ת������λ����

#define TURN_UP_SET               MB_SET_BIT  (4)  //ת������㶯+����
#define TURN_UP_RESET             MB_RESET_BIT(4)  //ת������㶯+����

#define TURN_DOWN_SET             MB_SET_BIT  (5)  //ת������㶯-����
#define TURN_DOWN_RESET           MB_RESET_BIT(5)  //ת������㶯-����

#define TURN_AXIS_RESET           MB_RESET_BIT(6)  //ת����������

#define TURN_ENABLE_SET           MB_SET_BIT  (7)  //ת����ʹ��
#define TURN_ENABLE_RESET         MB_RESET_BIT(7)  //ת��������

#define WALK_BREAK_SET            MB_SET_BIT  (8)  //���ߵ��ɲ��
#define WALK_BREAK_RESET          MB_RESET_BIT(8)  //���ߵ���ɿ�ɲ��

#define WALK_ENABLE_SET           MB_SET_BIT  (9)  //���ߵ��ʹ��
#define WALK_ENABLE_RESET         MB_RESET_BIT(9)  //���ߵ������

#define LIFT_UP_SET               MB_SET_BIT  (10) //����ֶ�����
#define LIFT_UP_RESET             MB_RESET_BIT(10) //���ȡ���ֶ�����
#define LIFT_UP_GET               MB_GET_BIT  (10) //����ֶ�������ѯ

#define LIFT_DOWN_SET             MB_SET_BIT  (11) //����ֶ��½�
#define LIFT_DOWN_RESET           MB_RESET_BIT(11) //���ȡ���ֶ��½�
#define LIFT_DOWN_GET             MB_GET_BIT  (11) //����ֶ��½���ѯ

#define LIFT_LOCATE_SET           MB_SET_BIT  (12) //��۶�λʹ��
#define LIFT_LOCATE_RESET         MB_RESET_BIT(12) //��۶�λ����
#define LIFT_LOCATE_GET           MB_GET_BIT  (12) //��۶�λ��ѯ

#define LIFT_ZERO_SET             MB_SET_BIT  (13) //��ۻ���ʹ��
#define LIFT_ZERO_RESET           MB_RESET_BIT(13) //��ۻ������

#define LIFT_BREAK_SET            MB_SET_BIT  (14) //���ɲ��
#define LIFT_BREAK_RESET          MB_RESET_BIT(14) //
#define LIFT_BREAK_GET            MB_GET_BIT  (14) //���ɲ����ѯ

#define LIFT_SQP_SHIELD_SET       MB_SET_BIT  (15) //��۽ӽ���������
#define LIFT_SQP_SHIELD_RESET     MB_RESET_BIT(15) //
#define LIFT_SQP_SHIELD_GET       MB_GET_BIT  (15) //

#define LIFT_ENCODER_4MA_SET      MB_SET_BIT  (16) //����������4ma����
#define LIFT_ENCODER_4MA_RESET    MB_RESET_BIT(16) //

#define LIFT_ENCODER_20MA_SET     MB_SET_BIT  (17) //����������20ma����
#define LIFT_ENCODER_20MA_RESET   MB_RESET_BIT(17) //

#define REMOVE_ALARM_SET          MB_SET_BIT  (18) //���������Ϣ
#define REMOVE_ALARM_RESET        MB_RESET_BIT(18) //���������Ϣ
/*******************ϵͳ��������********************************/
/*******************0X24~0X47***********************************/
#define DEFAULT_DATA_SET          MB_SET_BIT  (24) //�ָ���������
#define DEFAULT_DATA_RESET        MB_RESET_BIT(24) //�ָ���������

#define READ_SET                  MB_SET_BIT  (25) //��ȡEEPROM����
#define READ_RESET                MB_RESET_BIT(25) //��ȡEEPROM����

#define SAVE_SET                  MB_SET_BIT  (26) //��������
#define SAVE_RESET                MB_RESET_BIT(26) //��������

#define WDT_TEST_SET              MB_SET_BIT  (27) //���Ź�����
#define WDT_TEST_RERSET           MB_RESET_BIT(27) //���Ź�����

#define ROPE_RESET_SET            MB_SET_BIT  (28) //������������
#define ROPE_RESET_RESET          MB_RESET_BIT(28) //������������
#define ROPE_RESET_GET            MB_GET_BIT  (28) //������������
/*******************IO�࿴����**********************************/
/*******************0X80~0X103**********************************/
#define SENSOR_LIMIT_SET          MB_SET_BIT  (80) //�޵���λ�Ӵ�
#define SENSOR_LIMIT_RESET        MB_RESET_BIT(80) //�޵���λ�뿪

#define SENSOR_ZERO_SET           MB_SET_BIT  (81) //�����λ�Ӵ�
#define SENSOR_ZERO_RESET         MB_RESET_BIT(81) //�����λ�뿪

#define ReadEMG_SET               MB_SET_BIT  (82) //��ͣ����
#define ReadEMG_RESET             MB_RESET_BIT(82) //��ͣ�ɿ�

#define BEFORE_RADAR_1_SET        MB_SET_BIT  (83) //ǰ�״�1
#define BEFORE_RADAR_1_RESET      MB_RESET_BIT(83) //
#define BEFORE_RADAR_2_SET        MB_SET_BIT  (84) //ǰ�״�2
#define BEFORE_RADAR_2_RESET      MB_RESET_BIT(84) //
#define BEFORE_RADAR_3_SET        MB_SET_BIT  (85) //ǰ�״�3
#define BEFORE_RADAR_3_RESET      MB_RESET_BIT(85) //
#define BEFORE_RADAR_3_GET        MB_GET_BIT  (85) //

#define AFTER_RADAR_1_SET         MB_SET_BIT  (86) //���״�1
#define AFTER_RADAR_1_RESET       MB_RESET_BIT(86) //
#define AFTER_RADAR_2_SET         MB_SET_BIT  (87) //���״�2
#define AFTER_RADAR_2_RESET       MB_RESET_BIT(87) //
#define AFTER_RADAR_3_SET         MB_SET_BIT  (88) //���״�3
#define AFTER_RADAR_3_RESET       MB_RESET_BIT(88) //
#define AFTER_RADAR_3_GET         MB_GET_BIT  (88) //

#define CRASH_SET                 MB_SET_BIT  (89) //��ײ������
#define CRASH_RESET               MB_RESET_BIT(89) //

#define START_SET                 MB_SET_BIT  (90) //������ť����
#define START_RESET               MB_RESET_BIT(90) //������ť�ɿ�

#define LIFT_SQP_SET              MB_SET_BIT  (91) //��۽ӽ�����
#define LIFT_SQP_RESET            MB_RESET_BIT(91) 
#define LIFT_SQP_GET              MB_GET_BIT(91) 

#define LIFT_REFLECTION_SET       MB_SET_BIT  (92) //��������䴫����
#define LIFT_REFLECTION_RESET     MB_RESET_BIT(92) 
#define LIFT_REFLECTION_GET       MB_GET_BIT  (92)

#define LIFT_LOWER_SET            MB_SET_BIT  (93) //�������λ
#define LIFT_LOWER_RESET          MB_RESET_BIT(93) //

#define LIFT_ENCODER_SET          MB_SET_BIT  (94) //��۾���ʽ��������������
#define LIFT_ENCODER_RESET        MB_RESET_BIT(94) //��۾���ʽ��������������
/*******************������Ϣ����********************************/
/*******************0X104~0X127*********************************/
#define ADC_ALARM_SET             MB_SET_BIT  (104)//ADC����λ
#define ADC_ALARM_RESET           MB_RESET_BIT(104)//ADC����λ
#define ADC_ALARM_GET             MB_GET_BIT  (104)//ADC����λ

#define ADC_SHUTDOWN_SET          MB_SET_BIT  (105)//ADCֹͣ�˶�λ
#define ADC_SHUTDOWN_RESET        MB_RESET_BIT(105)//ADCֹͣ�˶�λ
#define ADC_SHUTDOWN_GET          MB_GET_BIT  (105)

#define TURN_ALARM_SET            MB_SET_BIT  (106) //ת��������
#define TURN_ALARM_RESET          MB_RESET_BIT(106) //ת����������
#define TURN_ALARM_GET            MB_GET_BIT  (106)

#define Walk_ALARM_SET            MB_SET_BIT  (107)//���ߵ籨��λ
#define Walk_ALARM_RESET          MB_RESET_BIT(107)
#define Walk_ALARM_GET            MB_GET_BIT  (107)

#define WDT_ALARM_SET             MB_SET_BIT  (108)//���Ź��������
#define WDT_ALARM_RESET           MB_RESET_BIT(108)//���Ź�����������쳣

#define IPC_BEAT_BIT_SET          MB_SET_BIT  (109)//���ػ��������쳣
#define IPC_BEAT_BIT_RESET        MB_RESET_BIT(109)//���ػ����������쳣

#define SERIAL1_SET               MB_SET_BIT  (110)//����1�쳣
#define SERIAL1_RESET             MB_RESET_BIT(110)//����1����

#define SERIAL2_SET               MB_SET_BIT  (111)//����2�쳣
#define SERIAL2_RESET             MB_RESET_BIT(111)//����2����

#define WEINVIEW_BEAT_BIT_SET     MB_SET_BIT  (112)//����ͨ�������쳣
#define WEINVIEW_BEAT_BIT_RESET   MB_RESET_BIT(112)//����ͨ���������쳣

#define TURN_LIMIT_ALARM_SET      MB_SET_BIT  (113)//ת���������ޱ���
#define TURN_LIMIT_ALARM_RESET    MB_RESET_BIT(113)//

#define TURN_ZERO_ALARM_SET       MB_SET_BIT  (114)//ת���������ޱ���
#define TURN_ZERO_ALARM_RESET     MB_RESET_BIT(114)//

#define CRASH_ALARM_SET           MB_SET_BIT  (115)//��ײ������
#define CRASH_ALARM_RESET         MB_RESET_BIT(115)
#define CRASH_ALARM_GET           MB_GET_BIT  (115)

#define LIFT_ENCODER_ALARM_SET    MB_SET_BIT  (116) //��۾���ʽ�������������߱���
#define LIFT_ENCODER_ALARM_RESET  MB_RESET_BIT(116) //
#define LIFT_ENCODER_ALARM_GET    MB_GET_BIT  (116)
/* Exported variables ---------------------------------------------------------*/
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
extern uint8_t  ucRegCoilsBuf  [REG_COILS_SIZE/ 8];
/* Exported functions prototypes ---------------------------------------------*/
extern void Modbus_Handler(void);
extern void Modbus_Data_Init(void);
extern void VarData_To_Save(void);
extern void VarData_To_Read(void);
/* Exported functions prototypes ---------------------------------------------*/
#endif /* __MB_HANDLER_H */
