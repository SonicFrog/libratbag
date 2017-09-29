#pragma once

#include <stdint.h>
#include <string.h>

#include "hidpp-generic.h"
#include "hidpp20.h"


#define HIDPP_RECEIVER_REG 0xB5
#define HIDPP_RECEIVER_SUB_ID 0x83

#define CMD_UNIFYING_GET_FIRST_DEV_NAME 0x40
#define CMD_UNIFYING_GET_FIRST_DEV_ID 0x20

#define HIDPP_RECEIVER_NAME_MAX_LEN 14

struct _hidpp_receiver_peer_name_msg {
	uint8_t report_id;
	uint8_t device_idx;
	uint8_t sub_id;
	uint8_t address;
	uint8_t paired_device_idx;
	uint8_t name_length;
	uint8_t name_bytes[HIDPP_RECEIVER_NAME_MAX_LEN];
};

struct _hidpp_receiver_read_register_msg {
	uint8_t report_id;
	uint8_t device_idx;
	uint8_t sub_id;
	uint8_t address;
	uint8_t paired_device_idx;
	uint8_t dest_id;
	uint8_t default_report_rate;
	uint16_t device_wireless_id;
	uint16_t reserved;
	uint8_t unifying_dev_type;
	uint8_t reserved_2[6];
};

union hidpp_receiver_message {
	struct _hidpp_receiver_read_register_msg read;
	struct _hidpp_receiver_peer_name_msg peer;
	uint8_t data[20];
};

struct ratbag_device;

int
hidpp_receiver_request_command(struct hidpp_device* dev,
							   union hidpp_receiver_message *msg);

uint16_t hidpp_receiver_read_real_id(struct hidpp20_device* dev);

char*
hidpp_receiver_read_real_name(struct hidpp20_device *dev);

int
hidpp_receiver_extended_probe(struct ratbag_device *device,
							   struct hidpp20_device* dev);
