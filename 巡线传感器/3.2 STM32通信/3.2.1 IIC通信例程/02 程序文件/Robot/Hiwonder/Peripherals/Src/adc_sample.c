#include "adc_sample.h"
#include "adc.h"

#define VREF			3.3f
#define ADC_MAXBIT		4095.0f
#define R1				100000.0f
#define R2				10000.0f
#define ADC_BUFFER_SIZE	12
#define SAMPLE_CHANNEL	3

uint16_t adc_value[ADC_BUFFER_SIZE] = {0};

void adc_sample_init()
{
	HAL_ADCEx_Calibration_Start(&hadc1);
}

void adc_sample_handler()
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_value, ADC_BUFFER_SIZE);
}

float get_battery_volocity()
{
	uint32_t sum = 0;
	float sample_average;
	
	for (uint8_t i = 0; i < ADC_BUFFER_SIZE / SAMPLE_CHANNEL; i++)
	{
		sum += adc_value[i * 3];
	}
	sample_average = (float)sum / (ADC_BUFFER_SIZE / SAMPLE_CHANNEL);


	return sample_average * VREF * ((R1 + R2) / R2) / 4095;
}


uint8_t get_button_state()
{
	uint16_t sample_average;
	uint32_t sum = 0;
	
	for (uint8_t i = 0; i < ADC_BUFFER_SIZE / SAMPLE_CHANNEL; i++)
	{
		sum += adc_value[i * 3 + 1];
	}
	sample_average = sum / (ADC_BUFFER_SIZE / SAMPLE_CHANNEL);
	if(sample_average > 1013 - 30 && sample_average < 1013 + 30)
	{
		return 1;
	}
	else if(sample_average > 2043 - 30 && sample_average < 2043 + 30)
	{
		return 2;
	}
	else if(sample_average > 811 - 30 && sample_average < 811 + 30)
	{
		return 3;
	}
	return 0;
}


