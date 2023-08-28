/*
 * adc.c
 *
 *  Created on: 23 Aðu 2023
 *      Author: sinem
 */
#include <stdio.h>
#include <stdlib.h>

#include "tasks_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

uint32_t adc_voltage;
static esp_adc_cal_characteristics_t adc1_chars;

float getADCvalue() { return adc_voltage; }
static void ADC_task(void *pvParameter){

	printf("Starting the ADC task.");

	esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, (ADC_WIDTH_MAX-1), 0, &adc1_chars);
	adc1_config_width((ADC_WIDTH_MAX-1));
	adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);


	for(;;){
		int adc_value = adc1_get_raw(ADC1_CHANNEL_7);
		adc_voltage = esp_adc_cal_raw_to_voltage(adc_value, &adc1_chars);

		vTaskDelay(500 / portTICK_RATE_MS);
	}

}

void ADC_reading_task_start(void){
	xTaskCreatePinnedToCore(&ADC_task, "ADC_reading_task", ADC_READING_TASK_STACK_SIZE, NULL, ADC_READING_TASK_PRIORITY, NULL, ADC_READING_TASK_CORE_ID);
}




