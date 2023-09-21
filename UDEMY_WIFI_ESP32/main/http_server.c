/*
 * http_server.c
 *
 *  Created on: Oct 20, 2021
 *      Author: sinem
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "sys/param.h"

#include "fft.h"
#include "adc.h"
#include "sntp_time_sync.h"
#include "http_server.h"
#include "tasks_common.h"
#include "wifi_app.h"
#include "cJSON.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

//Local time status
static bool g_is_local_time_set = false;

// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

// Wifi connect status
static int g_wifi_connect_status = 0;

// Firmware update status
static int g_fw_update_status = OTA_UPDATE_PENDING;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

#define NFFT 512
float show[NFFT];

double start, end;

timer_config_t timer_config = {
  .alarm_en = false,
  .counter_en = true,
  .counter_dir = TIMER_COUNT_UP,
  .divider = 80   /* 1 us per tick */
};



/**
 * ESP32 timer configuration passed to esp_timer_create.
 */
const esp_timer_create_args_t fw_update_reset_args = {
		.callback = &http_server_fw_update_reset_callback,
		.arg = NULL,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "fw_update_reset"
};
esp_timer_handle_t fw_update_reset;


float x1[NFFT]={0.043578,0.339805,-1.895482,-3.487084,-4.019023,
		-4.993769,-4.365126,-2.323118,-1.448207,-0.923849,-0.276936,
		-0.098775,1.677544,4.367093,4.590293,3.951197,4.062138,
		2.541151,0.900768,1.652468,1.493893,-0.888388,-2.605080,
		-3.822482,-4.818392,-3.956422,-2.827169,-2.705209,-2.012785,
		-1.600079,-1.564403,0.896043,3.806987,3.849950,3.609848,
		4.075735,2.931691,2.151743,3.071438,2.484443,0.303868,
		-1.484753,-3.335110,-4.143545,-3.166951,-3.076741,-3.605760,
		-2.900634,-2.862355,-2.849050,0.043578,2.811941,2.806800,
		2.985052,3.589429,3.006231,3.243326,4.149018,3.254075,
		1.548287,-0.276936,-2.570911,-3.024738,-2.105042,-3.018159,
		-4.048803,-3.546314,-3.930985,-3.801514,-0.819668,1.493893,
		1.583748,2.097202,2.649654,2.790060,4.043578,4.781283,
		3.766927,2.689497,0.872057,-1.564403,-1.576093,-0.895295,
		-2.622186,-3.998604,-3.924265,-4.676761,-4.320393,-1.630844,
		0.012307,0.303868,0.987383,1.367172,2.328591,4.441501,
		4.923259,4.002692,3.571501,1.839927,-0.376914,0.043578,
		0.339805,-1.895482,-3.487084,-4.019023,-4.993769,-4.365126,
		-2.323118,-1.448207,-0.923849,-0.276936,-0.098775,1.677544,
		4.367093,4.590293,3.951197,4.062138,2.541151,0.900768,
		1.652468,1.493893,-0.888388,-2.605080,-3.822482,-4.818392,
		-3.956422,-2.827169,-2.705209,-2.012785,-1.600079,-1.564403,
		0.896043,3.806987,3.849950,3.609848,4.075735,2.931691,
		2.151743,3.071438,2.484443,0.303868,-1.484753,-3.335110,
		-4.143545,-3.166951,-3.076741,-3.605760,-2.900634,-2.862355,
		-2.849050,0.043578,2.811941,2.806800,2.985052,3.589429,
		3.006231,3.243326,4.149018,3.254075,1.548287,-0.276936,
		-2.570911,-3.024738,-2.105042,-3.018159,-4.048803,-3.546314,
		-3.930985,-3.801514,-0.819668,1.493893,1.583748,2.097202,
		2.649654,2.790060,4.043578,4.781283,3.766927,2.689497,
		0.872057,-1.564403,-1.576093,-0.895295,-2.622186,-3.998604,
		-3.924265,-4.676761,-4.320393,-1.630844,0.012307,0.303868,
		0.987383,1.367172,2.328591,4.441501,4.923259,4.002692,
		3.571501,1.839927,-0.376914,0.043578,0.339805,-1.895482,
		-3.487084,-4.019023,-4.993769,-4.365126,-2.323118,-1.448207,
		-0.923849,-0.276936,-0.098775,1.677544,4.367093,4.590293,
		3.951197,4.062138,2.541151,0.900768,1.652468,1.493893,
		-0.888388,-2.605080,-3.822482,-4.818392,-3.956422,-2.827169,
		-2.705209,-2.012785,-1.600079,-1.564403,0.896043,3.806987,
		3.849950,3.609848,4.075735,2.931691,2.151743,3.071438,
		2.484443,0.303868,-1.484753,-3.335110,-4.143545,-3.166951,
		-3.076741,-3.605760,-2.900634,-2.862355,-2.849050,0.043578,
		2.811941,2.806800,2.985052,3.589429,3.006231,3.243326,
		4.149018,3.254075,1.548287,-0.276936,-2.570911,-3.024738,
		-2.105042,-3.018159,-4.048803,-3.546314,-3.930985,-3.801514,
		-0.819668,1.493893,1.583748,2.097202,2.649654,2.790060,
		4.043578,4.781283,3.766927,2.689497,0.872057,-1.564403,
		-1.576093,-0.895295,-2.622186,-3.998604,-3.924265,-4.676761,
		-4.320393,-1.630844,0.012307,0.303868,0.987383,1.367172,
		2.328591,4.441501,4.923259,4.002692,3.571501,1.839927,
		-0.376914,0.043578,0.339805,-1.895482,-3.487084,-4.019023,
		-4.993769,-4.365126,-2.323118,-1.448207,-0.923849,-0.276936,
		-0.098775,1.677544,4.367093,4.590293,3.951197,4.062138,
		2.541151,0.900768,1.652468,1.493893,-0.888388,-2.605080,
		-3.822482,-4.818392,-3.956422,-2.827169,-2.705209,-2.012785,
		-1.600079,-1.564403,0.896043,3.806987,3.849950,3.609848,
		4.075735,2.931691,2.151743,3.071438,2.484443,0.303868,
		-1.484753,-3.335110,-4.143545,-3.166951,-3.076741,-3.605760,
		-2.900634,-2.862355,-2.849050,0.043578,2.811941,2.806800,
		2.985052,3.589429,3.006231,3.243326,4.149018,3.254075,
		1.548287,-0.276936,-2.570911,-3.024738,-2.105042,-3.018159,
		-4.048803,-3.546314,-3.930985,-3.801514,-0.819668,1.493893,
		1.583748,2.097202,2.649654,2.790060,4.043578,4.781283,
		3.766927,2.689497,0.872057,-1.564403,-1.576093,-0.895295,
		-2.622186,-3.998604,-3.924265,-4.676761,-4.320393,-1.630844,
		0.012307,0.303868,0.987383,1.367172,2.328591,4.441501,
		4.923259,4.002692,3.571501,1.839927,-0.376914,0.043578,
		0.339805,-1.895482,-3.487084,-4.019023,-4.993769,-4.365126,
		-2.323118,-1.448207,-0.923849,-0.276936,-0.098775,1.677544,
		4.367093,4.590293,3.951197,4.062138,2.541151,0.900768,
		1.652468,1.493893,-0.888388,-2.605080,-3.822482,-4.818392,
		-3.956422,-2.827169,-2.705209,-2.012785,-1.600079,-1.564403,
		0.896043,3.806987,3.849950,3.609848,4.075735,2.931691,
		2.151743,3.071438,2.484443,0.303868,-1.484753,-3.335110,
		-4.143545,-3.166951,-3.076741,-3.605760,-2.900634,-2.862355,
		-2.849050,0.043578,2.811941,2.806800,2.985052,3.589429,
		3.006231,3.243326,4.149018,3.254075,1.548287,-0.276936,
		-2.570911,-3.024738,-2.105042,-3.018159,-4.048803,-3.546314,
		-3.930985,-3.801514,-0.819668,1.493893,1.583748,2.097202,
		2.649654,2.790060,4.043578,4.781283,3.766927,2.689497,
		0.872057,-1.564403,-1.576093,-0.895295,-2.622186,-3.998604,
		-3.924265,-4.676761,-4.320393,-1.630844,0.012307,0.303868,
		0.987383,1.367172,2.328591,4.441501,4.923259,4.002692,
		3.571501,1.839927,-0.376914,0.043578,0.339805,-1.895482,
		-3.487084,-4.019023,-4.993769,-4.365126,-2.323118,-1.448207,
		-0.923849,-0.276936,-0.098775
    };


// Embedded files: JQuery, index.html, app.css, app.js and favicon.ico files
extern const uint8_t jquery_3_3_1_min_js_start[]	asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[]		asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[]				asm("_binary_index_html_start");
extern const uint8_t index_html_end[]				asm("_binary_index_html_end");
extern const uint8_t app_css_start[]				asm("_binary_app_css_start");
extern const uint8_t app_css_end[]					asm("_binary_app_css_end");
extern const uint8_t app_js_start[]					asm("_binary_app_js_start");
extern const uint8_t app_js_end[]					asm("_binary_app_js_end");
extern const uint8_t chart_js_start[]					asm("_binary_chart_js_start");
extern const uint8_t chart_js_end[]					asm("_binary_chart_js_end");
extern const uint8_t favicon_ico_start[]			asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[]				asm("_binary_favicon_ico_end");

void clock_init()
{
  timer_init(TIMER_GROUP_0, TIMER_0, &timer_config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  timer_start(TIMER_GROUP_0, TIMER_0);
}

/**
 * Checks the g_fw_update_status and creates the fw_update_reset timer if g_fw_update_status is true.
 */
static void http_server_fw_update_reset_timer(void)
{
	if (g_fw_update_status == OTA_UPDATE_SUCCESSFUL)
	{
		ESP_LOGI(TAG, "http_server_fw_update_reset_timer: FW updated successful starting FW update reset timer");

		// Give the web page a chance to receive an acknowledge back and initialize the timer
		ESP_ERROR_CHECK(esp_timer_create(&fw_update_reset_args, &fw_update_reset));
		ESP_ERROR_CHECK(esp_timer_start_once(fw_update_reset, 8000000));
	}
	else
	{
		ESP_LOGI(TAG, "http_server_fw_update_reset_timer: FW update unsuccessful");
	}
}

/**
 * HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	for (;;)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case HTTP_MSG_WIFI_CONNECT_INIT:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");
					g_wifi_connect_status= HTTP_WIFI_STATUS_CONNECTING;

					break;

				case HTTP_MSG_WIFI_CONNECT_SUCCESS:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_SUCCESS;

					break;

				case HTTP_MSG_WIFI_CONNECT_FAIL:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_FAILED;

					break;

				case HTTP_MSG_WIFI_USER_DISCONNECT:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_USER_DISCONNECT");
					g_wifi_connect_status = HTTP_WIFI_STATUS_DISCONNECTED;

					break;

				case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_SUCCESSFUL");
					g_fw_update_status = OTA_UPDATE_SUCCESSFUL;
					http_server_fw_update_reset_timer();

					break;

				case HTTP_MSG_OTA_UPDATE_FAILED:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_FAILED");
					g_fw_update_status = OTA_UPDATE_FAILED;

					break;
				case HTTP_MSG_TIME_SERVICE_INITIALIZED:
					ESP_LOGI(TAG, "HTTP_MSG_TIME_SERVICE_INITIALIZED");
					g_is_local_time_set = true;

					break;

				default:
					break;
			}
		}
	}
}

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Jquery requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}

/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "index.html requested");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}

/**
 * app.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.css requested");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

	return ESP_OK;
}

/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

	return ESP_OK;
}

/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_chart_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "chart.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)chart_js_start, chart_js_end - chart_js_start);

	return ESP_OK;
}

/**
 * Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "favicon.ico requested");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

/**
 * Receives the .bin file fia the web page and handles the firmware update
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_OTA_update_handler(httpd_req_t *req)
{
	esp_ota_handle_t ota_handle;

	char ota_buff[1024];
	int content_length = req->content_len;
	int content_received = 0;
	int recv_len;
	bool is_req_body_started = false;
	bool flash_successful = false;

	const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

	do
	{
		// Read the data for the request
		if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0)
		{
			// Check if timeout occurred
			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
			{
				ESP_LOGI(TAG, "http_server_OTA_update_handler: Socket Timeout");
				continue; ///> Retry receiving if timeout occurred
			}
			ESP_LOGI(TAG, "http_server_OTA_update_handler: OTA other Error %d", recv_len);
			return ESP_FAIL;
		}
		printf("http_server_OTA_update_handler: OTA RX: %d of %d\r", content_received, content_length);

		// Is this the first data we are receiving
		// If so, it will have the information in the header that we need.
		if (!is_req_body_started)
		{
			is_req_body_started = true;

			// Get the location of the .bin file content (remove the web form data)
			char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;
			int body_part_len = recv_len - (body_start_p - ota_buff);

			printf("http_server_OTA_update_handler: OTA file size: %d\r\n", content_length);

			esp_err_t err = esp_ota_begin(update_partition, 0xffffffff, &ota_handle);
			if (err != ESP_OK)
			{
				printf("http_server_OTA_update_handler: Error with OTA begin, cancelling OTA\r\n");
				return ESP_FAIL;
			}
			else
			{
				printf("http_server_OTA_update_handler: Writing to partition subtype %d at offset 0x%x\r\n", update_partition->subtype, update_partition->address);
			}

			// Write this first part of the data
			esp_ota_write(ota_handle, body_start_p, body_part_len);
			content_received += body_part_len;
		}
		else
		{
			// Write OTA data
			esp_ota_write(ota_handle, ota_buff, recv_len);
			content_received += recv_len;
		}

	} while (recv_len > 0 && content_received < content_length);

	if (esp_ota_end(ota_handle) == ESP_OK)
	{
		// Lets update the partition
		if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
		{

			flash_successful = true;
		}
		else
		{
			ESP_LOGI(TAG, "http_server_OTA_update_handler: FLASHED ERROR!!!");
		}
	}
	else
	{
		ESP_LOGI(TAG, "http_server_OTA_update_handler: esp_ota_end ERROR!!!");
	}

	// We won't update the global variables throughout the file, so send the message about the status
	if (flash_successful) { http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_SUCCESSFUL); } else { http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_FAILED); }

	return ESP_OK;
}

/**
 * OTA status handler responds with the firmware update status after the OTA update is started
 * and responds with the compile time/date when the page is first requested
 * @param req HTTP request for which the uri needs to be handled
 * @return ESP_OK
 */
esp_err_t http_server_OTA_status_handler(httpd_req_t *req)
{
	char otaJSON[100];

	ESP_LOGI(TAG, "OTAstatus requested");

	sprintf(otaJSON, "{\"ota_update_status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\"}", g_fw_update_status, __TIME__, __DATE__);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, otaJSON, strlen(otaJSON));

	return ESP_OK;
}

static esp_err_t http_server_adc_sensor_reading_handler(httpd_req_t *req){
	//ESP_LOGI(TAG, "/adcSensor.json requested.");
	char adcSensorJSON[100];
	//printf("Voltage: %1.f \n", getADCvalue());
	sprintf(adcSensorJSON, "{\"adc_voltage\": \"%1.f\"}", getADCvalue());
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, adcSensorJSON, strlen(adcSensorJSON));
	return ESP_OK;
}
static esp_err_t http_server_fft_handler(httpd_req_t *req){
	//ESP_LOGI(TAG, "/adcSensor.json requested.");

	int k;

	    float re_part;
	    float im_part;
	    // Create fft plan and let it allocate arrays
	    fft_config_t *fft_analysis = fft_init(NFFT, FFT_REAL, FFT_FORWARD, NULL, NULL);

	    // Fill array with input signal
	    for (k = 0 ; k < fft_analysis->size ; k++){
	      fft_analysis->input[k] = x1[k];
	      //printf("%f\n",fft_analysis->input[k]);
	    }
	      // Create a cJSON array
	      cJSON *jsonArray = cJSON_CreateArray();
	      timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &start);
	      fft_execute(fft_analysis);
	      timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &end);

	      printf(" Real FFT size=%d runtime=%f ms\n", NFFT, 1000 * (end - start));

	      timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &start);
	      for (k = 0 ; k < fft_analysis->size/2 ; k++){
	          	re_part = fft_analysis->output[2*k];
	          	im_part = fft_analysis->output[2*k+1];

	          	//printf("%f", re_part);
	          	//printf("+ i%f\n", im_part);
	          	show[k] = sqrt(re_part*re_part+im_part*im_part);


	          	cJSON *jsonValue = cJSON_CreateNumber(show[k]);
	          	cJSON_AddItemToArray(jsonArray, jsonValue);
	      }
	      timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &end);
	      printf(" For %f ms\n", 1000 * (end - start));
	      // Add elements from myArray to the cJSON array


	      // Convert the cJSON array to a JSON string
	      char *jsonString = cJSON_Print(jsonArray);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, jsonString, strlen(jsonString));
	free(jsonString);
    fft_destroy(fft_analysis);
	return ESP_OK;
}

/**
 * wifiConnect.json handler is invoked after the connect button is pressed
 * and handles receiving the SSID and password entered by the user
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_wifi_connect_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnect.json requested");

	size_t len_ssid = 0, len_pass = 0;// no size ssid and password length
	char *ssid_str = NULL, *pass_str = NULL; //These pointers will be used to store the actual SSID and password strings extracted from the HTTP headers.

	// Get SSID header
	len_ssid = httpd_req_get_hdr_value_len(req, "my-connect-ssid") + 1; //takes the header length of in the webpage my-connect-ssid
	if (len_ssid > 1)
	{
		ssid_str = malloc(len_ssid);
		if (httpd_req_get_hdr_value_str(req, "my-connect-ssid", ssid_str, len_ssid) == ESP_OK) //ssid_str içerisine http den alýnan deðerin yerleþtirilmesi
		{
			ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => my-connect-ssid: %s", ssid_str);
		}
	}
	// Same procedure to get Password header
		len_pass = httpd_req_get_hdr_value_len(req, "my-connect-pwd") + 1;
		if (len_pass > 1)
		{
			pass_str = malloc(len_pass);
			if (httpd_req_get_hdr_value_str(req, "my-connect-pwd", pass_str, len_pass) == ESP_OK)
			{
				ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => my-connect-pwd: %s", pass_str);
			}
		}

		// Update the Wifi networks configuration and let the wifi application know
		wifi_config_t* wifi_config = wifi_app_get_wifi_config();
		memset(wifi_config, 0x00, sizeof(wifi_config_t)); //wifi configuration filled with zeros
		memcpy(wifi_config->sta.ssid, ssid_str, len_ssid);	//sets the sta.ssid as ssid_str and length as len_ssid
		memcpy(wifi_config->sta.password, pass_str, len_pass);
		wifi_app_send_message(WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER);

		//clears the pointers
		free(ssid_str);
		free(pass_str);

		return ESP_OK;
	}

/**
 * wifiConnectStatus handler updates the connection status for the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_wifi_connect_status_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnectStatus requested");

	char statusJSON[100];

	sprintf(statusJSON, "{\"wifi_connect_status\": \"%d\"}", g_wifi_connect_status);
	//sprintf(adcSensorJSON, "{\"adc_voltage\": \"%1.f\"}", getADCvalue());
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, statusJSON, strlen(statusJSON));

	return ESP_OK;
}

/**
 * wifiConnectInfo handler updates the the web page with connection information.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_wifi_connect_info_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnectInfo.json requested");

		char ipInfoJSON[200];
		memset(ipInfoJSON, 0, sizeof(ipInfoJSON));

		char ip[IP4ADDR_STRLEN_MAX];
		char netmask[IP4ADDR_STRLEN_MAX];
		char gw[IP4ADDR_STRLEN_MAX];

		if (g_wifi_connect_status == HTTP_WIFI_STATUS_CONNECT_SUCCESS)
		{
			wifi_ap_record_t wifi_data;
			ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&wifi_data));
			char *ssid = (char*)wifi_data.ssid;

			esp_netif_ip_info_t ip_info;
			ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_sta, &ip_info));
			esp_ip4addr_ntoa(&ip_info.ip, ip, IP4ADDR_STRLEN_MAX);
			esp_ip4addr_ntoa(&ip_info.netmask, netmask, IP4ADDR_STRLEN_MAX);
			esp_ip4addr_ntoa(&ip_info.gw, gw, IP4ADDR_STRLEN_MAX);

			sprintf(ipInfoJSON, "{\"ip\":\"%s\",\"netmask\":\"%s\",\"gw\":\"%s\",\"ap\":\"%s\"}", ip, netmask, gw, ssid);
		}

		httpd_resp_set_type(req, "application/json");
		httpd_resp_send(req, ipInfoJSON, strlen(ipInfoJSON));

		return ESP_OK;
}
/**
 * localTime.json handler responds by sending the local time.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_local_time_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/localTime.json requested");

	char localTimeJSON[100] = {0};

	if (g_is_local_time_set)
	{
		sprintf(localTimeJSON, "{\"time\":\"%s\"}", sntp_time_sync_get_time());
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, localTimeJSON, strlen(localTimeJSON));

	return ESP_OK;
}
/**
 * apSSID.json handler responds by sending the local time.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_ap_ssid_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/apSSID.json requested");
	char ssidJSON[50];

	wifi_config_t *wifi_config = wifi_app_get_wifi_config();
	esp_wifi_get_config(ESP_IF_WIFI_AP, wifi_config);
	char *ssid=(char*)wifi_config->ap.ssid;

	sprintf(ssidJSON, "{\"ssid\":\"%s\"}", ssid);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, ssidJSON, strlen(ssidJSON));

	return ESP_OK;
}

static esp_err_t http_server_get_wifi_ssid_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiSSID.json requested");
	char wifiSSIDJSON[300];
	memset(wifiSSIDJSON, 0, sizeof(wifiSSIDJSON));
	wifi_scan_config_t scan_config = {
	    .ssid = 0,
	    .bssid = 0,
	    .channel = 0,
	    .show_hidden = true
	  };

	ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
	wifi_ap_record_t wifi_records[MAXIMUM_AP];

	uint16_t max_records = MAXIMUM_AP;
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_records, wifi_records));
	ESP_LOGI(TAG,"Number of Access Points Found: %d\n", max_records);

	char *wifi1 = (char *)wifi_records[0].ssid;
	char *wifi2 = (char *)wifi_records[1].ssid;
	char *wifi3 = (char *)wifi_records[2].ssid;
	char *wifi4 = (char *)wifi_records[3].ssid;
	char *wifi5 = (char *)wifi_records[4].ssid;
	sprintf(wifiSSIDJSON, "{\"wifi_1\":\"%s\",\"wifi_2\":\"%s\",\"wifi_3\":\"%s\",\"wifi_4\":\"%s\",\"wifi_5\":\"%s\"}", wifi1, wifi2, wifi3, wifi4, wifi5);


	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, wifiSSIDJSON, strlen(wifiSSIDJSON));

	return ESP_OK;

}

static esp_err_t http_server_wifi_disconnect_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "wifiDisconnect.json requested");

	wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECTED);
	return ESP_OK;
}

/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
	// Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Create HTTP server monitor task
	xTaskCreatePinnedToCore(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor, HTTP_SERVER_MONITOR_CORE_ID);

	// Create the message queue
	http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

	// The core that the HTTP server will run on
	config.core_id = HTTP_SERVER_TASK_CORE_ID;

	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	ESP_LOGI(TAG,
			"http_server_configure: Starting server on port: '%d' with task priority: '%d'",
			config.server_port,
			config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");

		// register query handler
		httpd_uri_t jquery_js = {
				.uri = "/jquery-3.3.1.min.js",
				.method = 1,
				.handler = http_server_jquery_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &jquery_js);

		// register index.html handler
		httpd_uri_t index_html = {
				.uri = "/",
				.method = 1,
				.handler = http_server_index_html_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &index_html);

		// register app.css handler
		httpd_uri_t app_css = {
				.uri = "/app.css",
				.method = 1,
				.handler = http_server_app_css_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_css);

		// register app.js handler
		httpd_uri_t app_js = {
				.uri = "/app.js",
				.method = 1,
				.handler = http_server_app_js_handler,
				.user_ctx = NULL
		};


		httpd_register_uri_handler(http_server_handle, &app_js);

		// register chart.js handler
		httpd_uri_t chart_js = {
			.uri = "/chart.js",
			.method = 1,
			.handler = http_server_chart_js_handler,
			.user_ctx = NULL
						};
		httpd_register_uri_handler(http_server_handle, &chart_js);


		// register favicon.ico handler
		httpd_uri_t favicon_ico = {
				.uri = "/favicon.ico",
				.method = 1,
				.handler = http_server_favicon_ico_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &favicon_ico);

		// register OTAupdate handler
		httpd_uri_t OTA_update = {
				.uri = "/OTAupdate",
				.method = 3,
				.handler = http_server_OTA_update_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &OTA_update);

		// register OTAstatus handler
		httpd_uri_t OTA_status = {
				.uri = "/OTAstatus",
				.method = 3,
				.handler = http_server_OTA_status_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &OTA_status);

		//register adcSensor.json handler
		httpd_uri_t adc_sensor_json = {
				.uri = "/adcSensor.json",
				.method = 1,
				.handler = http_server_adc_sensor_reading_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &adc_sensor_json);


		//register fft.json handler
		httpd_uri_t fft_json = {
				.uri = "/fft.json",
				.method = 1,
				.handler = http_server_fft_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &fft_json);


		//register wifiConnect.json handler
		httpd_uri_t wifi_connect_json = {
			.uri = "/wifiConnect.json",
			.method = 3,
			.handler = http_server_wifi_connect_json_handler,
			.user_ctx = NULL
			};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_json);


		//register wifiConnectStatus.json handler
		httpd_uri_t wifi_connect_status_json = {
			.uri = "/wifiConnectStatus",
			.method = 3,
			.handler = http_server_wifi_connect_status_json_handler,
			.user_ctx = NULL
			};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_status_json);


		//register wifiConnectInfo.json handler
		httpd_uri_t wifi_connect_info_json = {
			.uri = "/wifiConnectInfo.json",
			.method = 1,
			.handler = http_server_get_wifi_connect_info_json_handler,
			.user_ctx = NULL
			};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_info_json);

		//register wifiDisconnect.json handler
		httpd_uri_t wifi_disconnect_json = {
			.uri = "/wifiDisconnect.json",
			.method = 0,
			.handler = http_server_wifi_disconnect_json_handler,
			.user_ctx = NULL
			};
		httpd_register_uri_handler(http_server_handle, &wifi_disconnect_json);

		// register localTime.json handler
		httpd_uri_t local_time_json = {
				.uri = "/localTime.json",
				.method = 1,
				.handler = http_server_get_local_time_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &local_time_json);


		// register apSSID.json handler
		httpd_uri_t ap_ssid_json = {
				.uri = "/apSSID.json",
				.method = 1,
				.handler = http_server_get_ap_ssid_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &ap_ssid_json);


		// register wifiSSID.json handler
		httpd_uri_t wifi_ssid_json = {
				.uri = "/wifiSSID.json",
				.method = 1,
				.handler = http_server_get_wifi_ssid_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &wifi_ssid_json);


		return http_server_handle;
	}

	return NULL;
}

void http_server_start(void)
{
	if (http_server_handle == NULL)
	{
		http_server_handle = http_server_configure();
	}
}

void http_server_stop(void)
{
	if (http_server_handle)
	{
		httpd_stop(http_server_handle);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
		http_server_handle = NULL;
	}
	if (task_http_server_monitor)
	{
		vTaskDelete(task_http_server_monitor);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
		task_http_server_monitor = NULL;
	}
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
	http_server_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}

void http_server_fw_update_reset_callback(void *arg)
{
	ESP_LOGI(TAG, "http_server_fw_update_reset_callback: Timer timed-out, restarting the device");
	esp_restart();
}



















