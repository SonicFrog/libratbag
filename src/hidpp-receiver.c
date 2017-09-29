#include "config.h"

#include <linux/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hidpp-receiver.h"
#include "hidpp-generic.h"
#include "hidpp20.h"

#include "libratbag-private.h"
#include "libratbag-hidraw.h"
#include "libratbag-data.h"

int hidpp_receiver_request_command(struct hidpp_device* dev, union hidpp_receiver_message *msg)
{
    int rc;
    unsigned msg_len = (msg->peer.report_id == REPORT_ID_LONG) ?
        LONG_MESSAGE_LENGTH : SHORT_MESSAGE_LENGTH;

    rc = hidpp_write_command(dev, msg->data, msg_len);

    if (rc < 0)
        return rc;

    rc = hidpp_read_response(dev, msg->data, REPORT_ID_LONG);

    if (rc < 0)
        return rc;

    return 0;
}


uint16_t hidpp_receiver_read_real_id(struct hidpp20_device* dev) {
    uint16_t wid;
    int rc;
    union hidpp_receiver_message mess = {
        .read = {
            .report_id = REPORT_ID_SHORT,
            .device_idx = HIDPP_RECEIVER_IDX,
            .sub_id = HIDPP_RECEIVER_SUB_ID,
            .address = HIDPP_RECEIVER_REG,
            // This is not an unifying receiver so it only has one device
            .paired_device_idx = CMD_UNIFYING_GET_FIRST_DEV_ID,
        },
    };

    // We can't use the hidpp20 functions directly since the protocol differs
    rc = hidpp_write_command(&dev->base, mess.data, SHORT_MESSAGE_LENGTH);

    if (rc < 0)
    {
        hidpp_log_error(&dev->base, "failed to send command to device: %s\n", strerror(errno));
        return 0;
    }

    rc = hidpp_read_response(&dev->base, mess.data, LONG_MESSAGE_LENGTH);

    if (rc < 0)
    {
        hidpp_log_error(&dev->base, "failed to receive response from device: %s\n",
                        strerror(errno));
    }

    wid = hidpp_be_u16_to_cpu(mess.read.device_wireless_id);

    hidpp_log_debug(&dev->base, "real device id is %04x\n", wid);

    return wid;
}

char*
hidpp_receiver_read_real_name(struct hidpp20_device *dev)
{
    int rc;
    const char* fmt = "Logitech %s wireless";
    const unsigned len = strlen(fmt) + HIDPP_RECEIVER_NAME_MAX_LEN;
    char* out = zalloc(len * sizeof(char));
    union hidpp_receiver_message msg = {
        .peer = {
            .report_id = REPORT_ID_SHORT,
            .device_idx = HIDPP_RECEIVER_IDX,
            .sub_id = HIDPP_RECEIVER_SUB_ID,
            .address = HIDPP_RECEIVER_REG,
            .paired_device_idx = CMD_UNIFYING_GET_FIRST_DEV_NAME,
        }
    };

    rc = hidpp_receiver_request_command(&dev->base, &msg);

    if (rc) {
        return NULL;
    }

    snprintf(out, len, fmt, msg.peer.name_bytes);
	out[len - 1] = '\0';

    return out;
}

int hidpp_receiver_extended_probe(struct ratbag_device *device, struct hidpp20_device* dev)
{
    hidpp_log_debug(&dev->base, "device is wireless, doing proper id\n");


    device->ids.product = hidpp_receiver_read_real_id(dev);

    if (!device->ids.product) {
        hidpp_log_error(&dev->base, "unable to get real device from receiver!\n");
        return -EINVAL;
    }

    char* name = hidpp_receiver_read_real_name(dev);

    if (!name) {
        hidpp_log_error(&dev->base, "unable to read real name from device\n");
        return -EINVAL;
    }

    free(device->name);
    device->name = name;

	return 0;
}
