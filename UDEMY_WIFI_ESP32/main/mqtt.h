/*
 * mqtt.h
 *
 *  Created on: 5 Eyl 2023
 *      Author: sinem
 */

#ifndef MAIN_MQTT_H_
#define MAIN_MQTT_H_


void mqtt_task_start(void);

typedef enum QoS {
	QOS0 = 0,
	QOS1 = 1
} QoS_num;

typedef struct {
	QoS_num qos;		///< Message Quality of Service
	uint8_t isRetained;	///< Retained messages are \b NOT supported by the AWS IoT Service at the time of this SDK release.
	uint8_t isDup;		///< Is this message a duplicate QoS > 0 message?  Handled automatically by the MQTT client.
	uint16_t id;		///< Message sequence identifier.  Handled automatically by the MQTT client.
	void *payload;		///< Pointer to MQTT message payload (bytes).
	size_t payloadLen;	///< Length of MQTT payload.
} Message_Params;



#endif /* MAIN_MQTT_H_ */
