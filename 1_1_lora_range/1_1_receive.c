/*
 * Copyright (c) 2019 Manivannan Sadhasivam
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/gpio.h>
#include <errno.h>
#include <string.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(DEFAULT_RADIO_NODE),
	     "No default LoRa radio specified in DT");

#define MAX_DATA_LEN 255
#define TEAM_PAYLOAD_PREFIX "ese5180t15-"
#define TEAM_PAYLOAD_PREFIX_LEN (sizeof(TEAM_PAYLOAD_PREFIX) - 1)

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lora_receive);

/* Explicit node label bindings for Nucleo-WL55JC discrete LEDs */
static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_NODELABEL(blue_led_1), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(green_led_2), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(red_led_3), gpios)
};

static uint8_t active_led_idx;

void lora_receive_cb(const struct device *dev, uint8_t *data, uint16_t size,
		     int16_t rssi, int8_t snr, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	/* Ignore packets from other teams that use the same LoRa physical-layer
	 * settings. The final counter byte may be any digit from 0 through 9. */
	if (size != TEAM_PAYLOAD_PREFIX_LEN + 1 ||
	    memcmp(data, TEAM_PAYLOAD_PREFIX, TEAM_PAYLOAD_PREFIX_LEN) != 0 ||
	    data[TEAM_PAYLOAD_PREFIX_LEN] < '0' ||
	    data[TEAM_PAYLOAD_PREFIX_LEN] > '9') {
		return;
	}

	if (gpio_is_ready_dt(&leds[active_led_idx])) {
		gpio_pin_set_dt(&leds[active_led_idx], 0);
	}

	/* Step to the next LED: blue -> green -> red. */
	active_led_idx = (active_led_idx + 1) % ARRAY_SIZE(leds);

	if (gpio_is_ready_dt(&leds[active_led_idx])) {
		gpio_pin_set_dt(&leds[active_led_idx], 1);
	}

	/* Safely terminate the payload before printing it as a string. */
	char rx_buf[MAX_DATA_LEN + 1];
	uint16_t print_size = MIN(size, MAX_DATA_LEN);

	memcpy(rx_buf, data, print_size);
	rx_buf[print_size] = '\0';

	LOG_INF("RX RSSI: %d dBm | SNR: %d dB | Payload: %s (Active LED: %d)",
		rssi, snr, rx_buf, active_led_idx);
	LOG_HEXDUMP_INF(data, size, "Raw Bytes");
}

int main(void)
{
	const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
	struct lora_modem_config config = {0};
	int ret;

	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s Device not ready", lora_dev->name);
		return 0;
	}

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (gpio_is_ready_dt(&leds[i])) {
			gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		}
	}

	/* Blue LED indicates that the receiver is ready. */
	if (gpio_is_ready_dt(&leds[0])) {
		gpio_pin_set_dt(&leds[0], 1);
	}

	/* These physical-layer settings must match the transmitter. */
	config.frequency = 433920000; /* 433.92 MHz */
	/* Must exactly match the transmitter's range profile. */
	config.bandwidth = BW_125_KHZ;
	config.datarate = SF_11;
	config.preamble_len = 16;
	config.coding_rate = CR_4_8;
	config.iq_inverted = false;
	config.public_network = false;
	config.tx = false;

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
		return 0;
	}

	LOG_INF("Listening continuously on 433.92 MHz (SF11, CR 4/8)...");

	ret = lora_recv_async(lora_dev, lora_receive_cb, NULL);
	if (ret < 0) {
		LOG_ERR("Failed to start async receive: %d", ret);
		return 0;
	}

	k_sleep(K_FOREVER);
	return 0;
}
