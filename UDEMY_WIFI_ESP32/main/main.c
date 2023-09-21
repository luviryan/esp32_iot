/**
 * Application entry point.
 */

#include "nvs_flash.h"

#include "fft.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include "esp_log.h"
#include "sntp_time_sync.h"
#include "wifi_app.h"
#include "adc.h"
#include "mqtt.h"
#include "http_server.h"





void app_main(void)
{
    // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	clock_init();

	// Start Wifi
	wifi_app_start();

	//Starting the adc task
	ADC_reading_task_start();

}

