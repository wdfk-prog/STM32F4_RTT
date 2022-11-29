/**
  ******************************************************************************
  * @file    user_math.h
  * @brief   �û��Զ��庯��
  ******************************************************************************
  * @attention  
  * https://blog.csdn.net/qq_39016531/article/details/107411030#comments_19409528
  * https://blog.csdn.net/qlexcel/article/details/103651072
  �����������궨��                           2022.07.20
  * @author ��������ţ_QJ����ľľ
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER__MATH_H
#define __USER__MATH_H
/* Includes ------------------------------------------------------------------*/
#include "sys.h"
/* Private includes ----------------------------------------------------------*/
#include <math.h>
#include <stdbool.h>
/* Exported constants --------------------------------------------------------*/
/***********************�Ƕ�***************************************************/
#define ushort uint16_t //ǿ��ת���޷�������
#define _short int16_t  //ǿ��ת���з�������
#define PI acos(-1)

/* Exported types ------------------------------------------------------------*/

/***********λ��ʽPID***************************************/
//#define PID_INTEGRAL_ON    //λ��ʽPID�Ƿ����������������PD���ƣ�ע�ͱ���
typedef struct
{ 
    float P;               
    float I;
    float D;	
#ifdef 	PID_INTEGRAL_ON
    float Integral;        //λ��ʽPID������
    float IntegralMax;     //λ��ʽPID���������ֵ�������޷�
#endif	
    float Last_Error;      //��һ�����	
    float OutputMax;       //λ��ʽPID������ֵ�������޷�
}position_PID;
/***********����ʽPID***************************************/
typedef struct
{
    float P;              //Kpϵ��
    float I;              //Kiϵ��
    float D;              //Kdϵ��
    float OutputMax;      //������ֵ�������޷�	
	
    int32_t LastError;     //ǰһ�����    
    int32_t PrevError;     //ǰ�������
}increment_PID;
/***********һ���ͺ��˲�**************************************/
typedef struct
{
  float K;//ϵ����ԽСԽƽ������Ӧʱ��Խ����
  float Ai;//���ֵ
  float Bi;//��һ��ֵ
  float result;//���
}FOL_TypeDef;
/* Exported macro ------------------------------------------------------------*/
//https://graphics.stanford.edu/~seander/bithacks.html#SwappingValuesSubAdd
#define SWAP(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b)))//��������ֵ
/*https://blog.csdn.net/pl0020/article/details/104813884*/
#define	USER_CLEAR_BIT(x, bit)	(x  &= ~(1 << bit))	        /* �����bitλ */
#define	USER_GET_BIT(x, bit)	  ((x &   (1 << bit)) >> bit)	/* ��ȡ��bitλ */
#define	USER_SET_BIT(x, bit)	  (x  |=  (1 << bit))	        /* ��λ��bitλ */
/*https://www.cnblogs.com/lvdongjie/p/4488011.html*/

/*https://www.cnblogs.com/life2refuel/p/8283435.html*/
extern bool compare_and_swap (int* reg, int oldval,int newval);
#define ONCE_RUN(code) {                                    \
    static int _done;                                       \
    if (!_done) {                                           \
        if (compare_and_swap(&_done, 0, 1)) {   \
            code                                            \
        }                                                   \
    }                                                       \
}

//
// EXTERN_RUN - �򵥵�����, ������ʹ�õĺ�
// ftest    : ��Ҫִ�еĺ�������
// ...      : �ɱ����, ����
//
#define EXTERN_RUN(ftest, ...)  \
do {                            \
    extern void ftest();        \
    ftest (__VA_ARGS__);        \
} while(0)

/*��������궨��
�ú궨������linux kernel 3.10��

https://blog.csdn.net/u012028275/article/details/118864192

x��ʾ���������������������������߸�����

divisor��ʾ����������ֻ����������

ע�⣺����������Ǹ�������ô����divisor�ı������Ͳ������޷��ŵģ����߽����δ����ġ�
*/
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

#define GPIO_TURN(x) (((x) == (GPIO_PIN_RESET))?(GPIO_PIN_SET):(GPIO_PIN_RESET))//��ƽ��ת
/* Exported variables ---------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
extern float PID_Cal(position_PID *pid, float NowValue, float AimValue);
extern float IncPIDCal(increment_PID *pid, float NowValue, float AimValue);

/*�˲�����*/
extern float First_Order_Lag(FOL_TypeDef *p,float intput);

/*��������*/
//https://graphics.stanford.edu/~seander/bithacks.html#SwappingValuesSubAdd
extern bool Detect_Opposite_INT(int x, int y);
extern int Compute_Sign_INT(int v,uint8_t CHAR_BIT);

extern long long qpow(int16_t a, int16_t n);
//https://blog.csdn.net/qq_43537721/article/details/107757766
extern float Hex_To_Decimal(unsigned char *Byte);
extern long FloatTohex(float HEX);
extern void FloatToByte(float floatNum,unsigned char* byteArry);
//https://blog.csdn.net/qq_43630810/article/details/108819378
extern unsigned char Reverse_byte(unsigned char wdata);
//https://www.edaboard.com/threads/itoa-for-8051-based-micros.133344/
extern void itoa(int n, char *s,int radix);
//https://blog.csdn.net/mayh554024289/article/details/47914237
extern void LeftShift(int* arr,int N, int K);
extern void RightShift(int* arr,int N, int K);

extern float Angle_Conversion(float absolute_angle);

extern uint16_t Binary_Search(uint32_t input,uint32_t *Array,uint16_t size);
//https://blog.csdn.net/qq_40757240/article/details/85727578
extern int Bit_Int_2(long n);
#endif /* __USER__MATH_H */
