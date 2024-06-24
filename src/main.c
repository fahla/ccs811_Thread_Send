#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/ccs811.h>
#include <stdio.h>

#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <openthread/udp.h>

int main(void)
{
	otError error = OT_ERROR_NONE;
	const struct device* ccs811 = DEVICE_DT_GET_ANY(ams_ccs811);
	struct sensor_value co2, tvoc;
	int rc = 0;
	char buf[64];
	printk("TRY INIT");

	if (!device_is_ready(ccs811))
	{

		printk("NOT GOOD TO START");

		return 1;

	}

	printk("GOOD TO START");

	while (true)

	{

		rc = sensor_sample_fetch(ccs811);

		sensor_channel_get(ccs811, SENSOR_CHAN_CO2, &co2);
		sensor_channel_get(ccs811, SENSOR_CHAN_VOC, &tvoc);
		printk("\nCCS811: %u ppm eCO2; %u ppb eTVOC\n", co2.val1, tvoc.val1);

		// Format the sensor data as a string
		snprintf(buf, sizeof(buf), "eCO2: %u ppm, eTVOC: %u ppb", co2.val1, tvoc.val1);

		otInstance* myInstance;
		myInstance = openthread_get_default_instance();
		otUdpSocket mySocket;
		otMessageInfo messageInfo;
		memset(&messageInfo, 0, sizeof(messageInfo));
		otIp6AddressFromString("ff03::1", &messageInfo.mPeerAddr);
		messageInfo.mPeerPort = 1234;


		do {
			error = otUdpOpen(myInstance, &mySocket, NULL, NULL);
			if (error != OT_ERROR_NONE) { break; }
			otMessage* test_Message = otUdpNewMessage(myInstance, NULL);
			error = otMessageAppend(test_Message, buf, (uint16_t)strlen(buf));
			if (error != OT_ERROR_NONE) { break; }
			error = otUdpSend(myInstance, &mySocket, test_Message, &messageInfo);
			if (error != OT_ERROR_NONE) { break; }
			error = otUdpClose(myInstance, &mySocket);
		} while (false);
		if (error == OT_ERROR_NONE) {
			printk("Send.\n");
		}
		else {
			printk("udpSend error: %d\n", error);
		}

		k_sleep(K_SECONDS(2));
	}
	return 0;
}