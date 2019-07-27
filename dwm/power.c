#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "dwm.h"
#include "dbus.h"
#include "power.h"

typedef enum UPowerDeviceType UPowerDeviceType;
enum UPowerDeviceType {
	UPOWER_DEVICE_TYPE_UNKNOWN,
	UPOWER_DEVICE_TYPE_LINE_POWER,
	UPOWER_DEVICE_TYPE_BATTERY,
	UPOWER_DEVICE_TYPE_UPS,
	UPOWER_DEVICE_TYPE_MONITOR,
	UPOWER_DEVICE_TYPE_MOUSE,
	UPOWER_DEVICE_TYPE_KEYBOARD,
	UPOWER_DEVICE_TYPE_PDA,
	UPOWER_DEVICE_TYPE_PHONE,
};

static void send(const char *destination, const char *interface, const char *object, const char *method) {
	DBusMessage *msg = NULL, *reply = NULL;
	DBusMessageIter args;
	DBusError error;
	
	msg=dbus_message_new_method_call(destination, object, interface, method);
	if(!msg)
		return;
	
	dbus_message_iter_init_append(msg, &args);
	bool interactive = true;
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &interactive)) {
		dbus_message_unref(msg);
		return;
	}
	
	//dbus_connection_send(dbus.system.connection, msg, &dbus.system.serial);
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus.system.connection, msg, -1, &error);
	//dbus_connection_flush(dbus.system.connection);
	
	if(!reply)
		return;
	
	
	dbus_error_init(&error);
	dbus_message_get_args(reply, &error, DBUS_TYPE_INVALID);
	
	dbus_message_unref(msg);
	dbus_message_unref(reply);
	//dbus.system.serial++;
}


static PowerBattery *battery_status(const char *path) {
	int current_type;
	DBusMessage *msg, *reply;
	DBusMessageIter args, array, dict, element, var;
	DBusError error;
	
	PowerBattery *battery = NULL;
	
	if(!(msg=dbus_message_new_method_call("org.freedesktop.UPower", path, "org.freedesktop.DBus.Properties", "GetAll")))
		return NULL;

	dbus_message_iter_init_append(msg, &args);
	
	const char *device_interface = "org.freedesktop.UPower.Device";
	if(!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &device_interface)) {
		dbus_message_unref(msg);
		return NULL;
	}
	
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus.system.connection, msg, -1, &error);
	
	if(!reply) {
		return NULL;
		dbus_message_unref(msg);
	}
	
	if(!dbus_message_iter_init(reply, &args)) {
		dbus_message_unref(msg);
		dbus_message_unref(reply);
		return NULL;
	}
	
	dbus_message_unref(msg);
	
	if(dbus_message_iter_get_arg_type(&args)!=DBUS_TYPE_ARRAY) {
		dbus_message_unref(reply);
		return NULL;
	}
	dbus_message_iter_recurse(&args, &array);
	if(dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_DICT_ENTRY) {
		dbus_message_unref(reply);
		return NULL;
	}
	
	battery = calloc(1, sizeof(PowerBattery));
	
	for(dbus_message_iter_recurse(&args, &dict); (current_type=dbus_message_iter_get_arg_type(&dict)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&dict)) {
		char *element_key;
		int element_type;
		dbus_message_iter_recurse(&dict, &element);
		if(dbus_message_iter_get_arg_type(&element) != DBUS_TYPE_STRING)
			continue;
		
		dbus_message_iter_get_basic(&element, &element_key);
		dbus_message_iter_next(&element);
		if(dbus_message_iter_get_arg_type(&element) != DBUS_TYPE_VARIANT)
			continue;
		
		dbus_message_iter_recurse(&element, &var);
		element_type=dbus_message_iter_get_arg_type(&var);
		
		if(!strcmp(element_key, "Type") && element_type == DBUS_TYPE_UINT32) {
			uint32_t val;
			dbus_message_iter_get_basic(&var, &val);
			
			if(val != UPOWER_DEVICE_TYPE_BATTERY) {
				free(battery);
				battery = NULL;
				break;
			}
			
		} else if(!strcmp(element_key, "State") && element_type == DBUS_TYPE_UINT32) {
			uint32_t val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->state = val;
			
		} else if(!strcmp(element_key, "Percentage") && element_type == DBUS_TYPE_DOUBLE) {
			double val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->percentage = val;
			
		} else if(!strcmp(element_key, "Voltage") && element_type == DBUS_TYPE_DOUBLE) {
			double val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->voltage = val;
			
		} else if(!strcmp(element_key, "Percentage") && element_type == DBUS_TYPE_DOUBLE) {
			double val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->capacity = val;
			
		} else if(!strcmp(element_key, "TimeToFull") && element_type == DBUS_TYPE_INT64) {
			int64_t val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->time_to_full = val;
			
		} else if(!strcmp(element_key, "TimeToEmpty") && element_type == DBUS_TYPE_INT64) {
			int64_t val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->time_to_empty = val;
			
		} else if(!strcmp(element_key, "Capacity") && element_type == DBUS_TYPE_DOUBLE) {
			double val;
			dbus_message_iter_get_basic(&var, &val);
			
			battery->capacity = val;
			
		} 
		
	}
	dbus_message_unref(reply);
	
	return battery;
}

PowerBattery *power_battery_status() {
	int current_type;
	DBusMessage *msg, *reply;
	DBusMessageIter args, array, element;
	DBusError error;
	
	PowerBattery *battery = NULL;
	
	if(!(msg=dbus_message_new_method_call("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", "EnumerateDevices")))
		return NULL;
	
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus.system.connection, msg, -1, &error);
	
	if(!reply) {
		return NULL;
		dbus_message_unref(msg);
	}
	
	if(!dbus_message_iter_init(reply, &args)) {
		dbus_message_unref(msg);
		dbus_message_unref(reply);
		return NULL;
	}
	
	dbus_message_unref(msg);
	
	/*if(dbus_message_iter_get_arg_type(&args)!=DBUS_TYPE_VARIANT) {
		dbus_message_unref(msg);
		return NULL;
	}
	dbus_message_iter_recurse(&args, &array);*/
	if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) {
		char *test;
		dbus_message_iter_get_basic(&args, &test);
		
		dbus_message_unref(reply);
		return NULL;
	}
	
	for(dbus_message_iter_recurse(&args, &element); (current_type = dbus_message_iter_get_arg_type(&element)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&element)) {
		char *path;
		
		if(current_type != DBUS_TYPE_OBJECT_PATH)
			continue;
		
		dbus_message_iter_get_basic(&element, &path);
		
		if((battery = battery_status(path))) {
			dbus_message_unref(reply);
			return battery;
		}
	}
	dbus_message_unref(reply);
	
	return NULL;
}

void power_logout() {
	quit(NULL);
}

void power_shutdown() {
	#if USE_CONSOLEKIT
	send("org.freedesktop.ConsoleKit", "org.freedesktop.ConsoleKit.Manager", "/org/freedesktop/ConsoleKit/Manager", "Stop");
	#else
	send("org.freedesktop.login1", "org.freedesktop.login1.Manager", "/org/freedesktop/login1", "PowerOff");
	#endif
}

void power_restart() {
	#if USE_CONSOLEKIT
	send("org.freedesktop.ConsoleKit", "org.freedesktop.ConsoleKit.Manager", "/org/freedesktop/ConsoleKit/Manager", "Restart");
	#else
	send("org.freedesktop.login1", "org.freedesktop.login1.Manager", "/org/freedesktop/login1", "Reboot");
	#endif
}

void power_suspend() {
	#if USE_CONSOLEKIT
	send("org.freedesktop.UPower", "org.freedesktop.UPower", "/org/freedesktop/UPower", "Suspend");
	#else
	send("org.freedesktop.login1", "org.freedesktop.login1.Manager", "/org/freedesktop/login1", "Suspend");
	#endif
}

void power_hibernate() {
	#if USE_CONSOLEKIT
	send("org.freedesktop.UPower", "org.freedesktop.UPower", "/org/freedesktop/UPower", "Hibernate");
	#else
	send("org.freedesktop.login1", "org.freedesktop.login1.Manager", "/org/freedesktop/login1", "Hibernate");
	#endif
}
