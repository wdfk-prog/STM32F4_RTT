#include <rtthread.h>
#include <rtdevice.h>
#include <main.h>

#define ADC_DEV_NAME        "adc1"      /* ADC �豸���� */
#define ADC_VREF_CHANNEL     ADC_CHANNEL_VREFINT - ADC_CHANNEL_0    /* ADC Vref ͨ�� */
#define ADC_TEMP_CHANNEL     ADC_CHANNEL_TEMPSENSOR - ADC_CHANNEL_0 /* ADC Temp ͨ�� */

static int adc_vol_sample(void)
{
    rt_uint32_t vref_value,temp_value;
    rt_uint16_t vref_mv,temp_mv; 

    rt_adc_device_t adc_dev;
    rt_err_t ret = RT_EOK;
    
    /* �����豸 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return RT_ERROR;
    }

    /* ʹ���豸 */
    ret = rt_adc_enable(adc_dev, ADC_VREF_CHANNEL);
    /* ��ȡ����ֵ */
    vref_value = rt_adc_read(adc_dev, ADC_VREF_CHANNEL);
    /* �ر�ͨ�� */
    ret = rt_adc_disable(adc_dev, ADC_VREF_CHANNEL);

    /* ʹ���豸 */
    ret = rt_adc_enable(adc_dev, ADC_TEMP_CHANNEL);
    /* ��ȡ����ֵ */
    temp_value = rt_adc_read(adc_dev, ADC_TEMP_CHANNEL);
    /* �ر�ͨ�� */
    ret = rt_adc_disable(adc_dev, ADC_TEMP_CHANNEL);
  
    rt_kprintf("Vref  is %u.\n", vref_value);
    rt_kprintf("Temp is %u.\n" , temp_value);
    
    // Calculating Vref voltage
    vref_mv = __LL_ADC_CALC_VREFANALOG_VOLTAGE(vref_value, ADC_RESOLUTION_12B);
    rt_kprintf("Vref voltage is %u mV.\n", vref_mv);
    // Calculate Temperature
    rt_kprintf("%d are Temperature in Degree C.\n",
    __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(2500,760,10,vref_mv, temp_value, ADC_RESOLUTION_12B));
    return RT_EOK;
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(adc_vol_sample, adc voltage convert sample);