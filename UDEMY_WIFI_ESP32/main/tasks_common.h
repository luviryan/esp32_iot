/*
 * tasks_common.h
 *
 *  Created on: Oct 17, 2023
 *      Author: sinem
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				5
#define WIFI_APP_TASK_CORE_ID				0

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			4
#define HTTP_SERVER_TASK_CORE_ID			0

// HTTP Server Monitor task
#define HTTP_SERVER_MONITOR_STACK_SIZE		4096
#define HTTP_SERVER_MONITOR_PRIORITY		3
#define HTTP_SERVER_MONITOR_CORE_ID			0

//ADC reading task
#define ADC_READING_TASK_STACK_SIZE			4096
#define	ADC_READING_TASK_PRIORITY			5
#define ADC_READING_TASK_CORE_ID			1

//SNTP Time Sync task
#define SNTP_TIME_SYNC_TASK_STACK_SIZE		4096
#define SNTP_TIME_SYNC_TASK_PRIORITY		4
#define SNTP_TIME_SYNC_TASK_CORE_ID			1

//AWS IoT Task
#define AWS_IOT_TASK_STACK_SIZE				9216
#define AWS_IOT_TASK_PRIORITY				6
#define AWS_IOT_TASK_CORE_ID				1

//MQTT Task
#define MQTT_TASK_STACK_SIZE				4096
#define	MQTT_TASK_PRIORITY					5
#define MQTT_TASK_CORE_ID					1




#endif /* MAIN_TASKS_COMMON_H_ */
