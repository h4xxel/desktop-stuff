#ifndef __POWER_H_
#define __POWER_H

#include <stdint.h>

typedef struct PowerBattery PowerBattery;
typedef enum PowerBatteryState PowerBatteryState;

enum PowerBatteryState {
	POWER_BATTERY_STATE_UNKNOWN,
	POWER_BATTERY_STATE_CHARGING,
	POWER_BATTERY_STATE_DISCHARGING,
	POWER_BATTERY_STATE_EMPTY,
	POWER_BATTERY_STATE_FULL,
	POWER_BATTERY_STATE_PENDING_CHARGE,
	POWER_BATTERY_STATE_PENDING_DISCHARGE,
};

struct PowerBattery {
	double percentage;
	double voltage;
	double capacity;
	int64_t time_to_full;
	int64_t time_to_empty;
	PowerBatteryState state;
};

void power_logout();
void power_shutdown();
void power_restart();
void power_suspend();
void power_hibernate();
#endif
