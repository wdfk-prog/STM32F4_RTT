/**
  ******************************************************************************
  * @file    user_math.c
  * @brief   �û��Զ��庯��
  ******************************************************************************
  * @attention  
  * https://blog.csdn.net/qq_39016531/article/details/107411030#comments_19409528
  * https://blog.csdn.net/qlexcel/article/details/103651072
  * @author ��������ţ_QJ����ľľ
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "user_math.h"
/* Private includes ----------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
bool compare_and_swap (int* reg, int oldval,int newval)
{
  int old_reg_val = *reg;
  if(old_reg_val == oldval)
  {
    *reg = newval;
    return true;
  }
  return false;
}
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**********************************************************************************************************
*	�� �� ����PID_Cal
*	����˵����λ��ʽPID����
*   ��    �룺
    NowValue:��ǰֵ
    AimValue:Ŀ��ֵ
*   ��    ����PID����ֵ��ֱ�Ӹ�ֵ��ִ�к���
https://blog.csdn.net/qlexcel/article/details/103651072
**********************************************************************************************************/

float PID_Cal(position_PID *pid, float NowValue, float AimValue)
{
    float  iError,     //��ǰ���
            Output;    //�������	
    iError = AimValue - NowValue;                   //���㵱ǰ���
	
#ifdef 	PID_INTEGRAL_ON	
    pid->Integral += pid->I * iError;	            //λ��ʽPID�������ۼ�
    pid->Integral = pid->Integral > pid->IntegralMax?pid->IntegralMax:pid->Integral;  //���������޷�
    pid->Integral = pid->Integral <-pid->IntegralMax?-pid->IntegralMax:pid->Integral; //���������޷�
#endif		
	
    Output = pid->P * iError                        //����P            
           + pid->D * (iError - pid->Last_Error);   //΢��D
	
#ifdef 	PID_INTEGRAL_ON		
    Output += pid->Integral;                        //����I
#endif	
 
    Output = Output > pid->OutputMax?pid->OutputMax:Output;  //����������޷�
    Output = Output <-pid->OutputMax?-pid->OutputMax:Output; //����������޷�
	
	pid->Last_Error = iError;		  	                     //�����ϴ��������´μ��� 
	return Output;	//���ؿ������ֵ
}
/**********************************************************************************************************
*	�� �� ����IncPIDCal
*	����˵��������ʽPID����
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/ 
float IncPIDCal(increment_PID *pid, float NowValue, float AimValue)     
{
	float iError;                          //��ǰ���ֵ
	float Output;                           //�����������ֵ
	
	iError = AimValue - NowValue;            //Ŀ��ֵ�뵱ǰֵ֮��
 
	Output = (pid->P * iError)               //E[k]��
			-(pid->I * pid->LastError)       //E[k-1]��
			+(pid->D * pid->PrevError);      //E[k-2]��
 
	pid->PrevError = pid->LastError;         //�洢�������´μ���
	pid->LastError = iError;
	
    Output = Output > pid->OutputMax?pid->OutputMax:Output;  //����������޷�
    Output = Output <-pid->OutputMax?-pid->OutputMax:Output; //����������޷�
	
	return(Output);                         //��������ֵ
}
/**
  * @brief  һ���ͺ��˲�����
  * @param  FOL_TypeDef �ͺ��˲�����
  * @param  intput ����ֵ
  * @retval None.
  * @note   Kϵ����Ҫ���������������Ҫ����
*/
float First_Order_Lag(FOL_TypeDef *p,float intput)
{
  p->Ai = intput;
  p->result = p->Ai * p->K + p->Bi * (1 - p->K);
  p->Bi = p->result;
  return p->result;
}
/**
  * @brief  ������������ķ����Ƿ��෴
  * @param  None.
  * @retval ���������෴ʱ����1,���򷵻�0 ��������һ����Ϊ0�򷵻�1
  * @note   Detect if two integers have opposite signs.
Manfred Weis suggested I add this entry on November 26, 2009.
https://graphics.stanford.edu/~seander/bithacks.html#SwappingValuesSubAdd
*/
bool Detect_Opposite_INT(int x, int y)// input values to compare signs
{
  return ((x ^ y) < 0); // true iff x and y have opposite signs
}
/**
  * @brief  ���������ķ���
  * @param  ��Ҫ������ŵ�����������λ��
  * @retval ����� -1��0 �� +1����ʹ��
  * @note   None.
*/
int Compute_Sign_INT(int v,uint8_t CHAR_BIT)// we want to find the sign of v
{
  int sign;   // the result goes here 
  sign = (v != 0) | -(int)((unsigned int)((int)v) >> (sizeof(int) * CHAR_BIT - 1));
  // Or, for more speed but less portability:
  sign = (v != 0) | (v >> (sizeof(int) * CHAR_BIT - 1));  // -1, 0, or +1
  // Or, for portability, brevity, and (perhaps) speed:
  sign = (v > 0) - (v < 0); // -1, 0, or +1
  return sign;
}
//�ǵݹ������
long long qpow(int16_t a, int16_t n)
{
    int ans = 1;
    while(n)
    {
      if(n&1)        //���n�ĵ�ǰĩλΪ1
        ans *= a;    //ans���ϵ�ǰ��a
      
      a *= a;        //a�Գ�
      n >>= 1;       //n������һλ
    }
    return ans;
}
/**
  * @brief  �ǶȻ���
  * @param  None.
  * @retval None.
  * @note   �ǶȻ��� ��Χ -180~180
*/
float Angle_Conversion(float absolute_angle)
{
  absolute_angle = fmod(absolute_angle,360);
  if(absolute_angle == 180)
    absolute_angle = -180;
  else if(absolute_angle > 180)
    absolute_angle =  absolute_angle- 360;
  else if(absolute_angle < -180)
    absolute_angle = absolute_angle + 360;
  return absolute_angle;
}
/**
  * @brief  ���ַ����
  * @param  input:��Ҫ������ֵ
  * @param  Array:��Ҫ��������
  * @param  size:�����С
  * @retval None.
  * @note   http://www.wawooo.com/191.html
*/
uint16_t Binary_Search(uint32_t input,uint32_t *Array,uint16_t size)//������Ҫ��С��������
{
	
  int start=0; //����ͷ����ţ�
  int end = size; //����β����ţ�
  int mid = 0;  //�����м䣨��ţ�
  while(start<=end)
  {
    mid=(start+end)/2;//�۰� ȡ�м䣨��ţ�
    if(input==Array[mid]) break;  //��ǰADֵ�������м�ֵ��� ˵���鵽
    if((input<Array[mid])&&(input>Array[mid+1]))  break; //������֮��
    if(input>Array[mid])  //��ǰADֵ���������м���ֵ ˵��Ҫ���ҵ����ں�һ��
      start = mid+1;   //ǰһ�������β���� ����ͷ���·���
    else if(input<Array[mid])  //��ǰADֵС�������м���ֵ ˵��Ҫ���ҵ�����ǰһ��
      end = mid-1;  //ǰһ�������ͷ���� ����β���·���
  }
  return mid;//���ز鵽��������
}
/**
  * @brief  �ж�������λ��
  * @param  long n:����
  * @retval λ��.
  * @note   https://blog.csdn.net/qq_40757240/article/details/85727578
*/
int Bit_Int_2(long n)
{
	if(n==0)
		return 1;
	else
		return log10(abs(n))+1;//��������λ��
	//����  1234  lgx+1;                   
}
//https://blog.csdn.net/qq_43537721/article/details/107757766
/**
  * @brief  ʮ�����Ƶ�������
  * @param  None.
  * @retval None.
  * @note   None.
*/
float Hex_To_Decimal(unsigned char *Byte)//,int num
{
//  	char cByte[4];//����һ
//     for (int i=0;i<num;i++)
//     {
// 	cByte[i] = Byte[i];
//     }
//      
// 	float pfValue=*(float*)&cByte;
//return  pfValue;
		return *((float*)Byte);//������
}
/**
  * @brief  //��������ʮ������ת��
  * @param  None.
  * @retval None.
  * @note   None.
*/
long FloatTohex(float HEX)
{
	return *( long *)&HEX;
}
/**
  * @brief  ��������ʮ������ת��2����
  * @param  None.
  * @retval None.
  * @note   None.
*/
void FloatToByte(float floatNum,unsigned char* byteArry)
{
    int i = 0;
    char* pchar=(char*)&floatNum;
    for( i=0;i < sizeof(float);i++)
    {
      *byteArry=*pchar;
      pchar++;
      byteArry++;
    }
}
/**
  * @brief  �ߵ�λ����.
  * @param  None.
  * @retval None.
  * @note   ��ʽ������.
https://blog.csdn.net/qq_43630810/article/details/108819378
*/
unsigned char Reverse_byte(unsigned char wdata)
{
	wdata=(wdata<<4)|(wdata>>4);
	wdata=((wdata<<2)&0xcc)|((wdata>>2)&0x33);
	wdata=((wdata<<1)&0xaa)|((wdata>>1)&0x55);
	return wdata;
}
/**
  * @brief  ����ת�ַ�����֧���������.
  * @param  None.
  * @retval None.
  * @note   ԭʵ�ֽ�֧��ʮ���ƣ�����������ĩ��Ϊ0����ض������
����10 - > 1
https://www.edaboard.com/threads/itoa-for-8051-based-micros.133344/.
*/
void itoa(int n, char *s,int radix) 
{ 
  char i;
  int n1;
  if (n<0) 
  {
    n=-n;
    *s++='-';
  }
  do
  {
    n1=n;
    i=0;
    while (1) 
    {
      if (n1<=(radix-1))   
      {
        *s++=n1+'0';
        break;
      }
      n1=n1/radix;
      i++;
    }
    while (i) 
    {
      i--;
      n1=n1*radix;
    }
    n-=n1;
  }while (n);
  if(!(n1 % radix))
     *s++='0';
  *s++=0;
}
/**
  * @brief  ����
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Reverse(int *arr,int start,int end)
{      //����
  for(; start < end;start++,end--)
  {
     int s = arr[end];
     arr[end] = arr[start];
     arr[start] = s;
   }
}
/**
  * @brief  ����ѭ�������ƶ�kλ���㷨.
  * @param  None.
  * @retval None.
  * @note  
 1 2 3 4 5 6 7 ��Ҫ����4�Σ���ô������Ҫ�Ľ���ǣ� 5 6 7 1 2 3 4��
1.��1234���� ��� 4321
2.��567���� ��� 765
3.��������������ƴ�ӣ� 4321765
4.�������ƴ�ӵ��������ã� 5671234 �ͳ���������Ҫ�Ľ���ˡ�
*/
void LeftShift(int* arr,int N, int K)
{
      K = K%N;                      //��Ӧ���Ĳ���
      Reverse(arr,0,K-1);           //1 
      Reverse(arr,K,N-1);           //2
      Reverse(arr,0,N-1);           //4
}
/**
  * @brief  ����ѭ�������ƶ�kλ���㷨.
  * @param  arr:����.
  * @param  N:���鳤��
  * @param  K:��λ����
* @retval None.
  * @note   None.
 1 2 3 4 5 6 7 ��Ҫ����4�Σ���ô������Ҫ�Ľ���ǣ�4 5 6 7 1 2 3��
1.��1234���� ��� 4321
2.��567���� ��� 765
3.��������������ƴ�ӣ� 4321765
4.�������ƴ�ӵ��������ã� 5671234 �ͳ���������Ҫ�Ľ���ˡ�
*/
void RightShift(int* arr,int N, int K)
{
    K = K%N;                      //��Ӧ���Ĳ���
    Reverse(arr,  0 , N - K - 1 );
    Reverse(arr, N - K, N - 1 );
    Reverse(arr,  0 , N - 1 );
}