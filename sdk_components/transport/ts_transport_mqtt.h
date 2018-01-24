// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_TRANSPORT_MQTT_H
#define TS_TRANSPORT_MQTT_H

#include "ts_connection.h"

#define TS_TRANSPORT_MQTT_MAX_HANDLERS 4

typedef struct Timer {
	uint64_t end_time;
} Timer;

// TODO - use TsTransport_t instead?
typedef struct Network Network;
struct Network {

	TsConnectionRef_t _connection;
	TsStatus_t _last_status;

	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

#endif // TS_TRANSPORT_MQTT_H
