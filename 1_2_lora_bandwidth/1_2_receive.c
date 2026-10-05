/*
 * ESE5180 Lab 1.2 - Team 15 high-throughput LoRa receiver
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
#define MAX_DATA_LEN 255
#define TEAM_PAYLOAD_PREFIX "ese5180t15-"
#define TEAM_PAYLOAD_PREFIX_LEN (sizeof(TEAM_PAYLOAD_PREFIX) - 1)

BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(DEFAULT_RADIO_NODE),
	     "No default LoRa radio specified in devicetree");

LOG_MODULE_REGISTER(lora_bandwidth_receive, LOG_LEVEL_INF);

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_NODELABEL(blue_led_1), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(green_led_2), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(red_led_3), gpios),
};

static uint8_t active_led_idx;

static void lora_receive_cb(const struct device *dev, uint8_t *data,
			    uint16_t size, int16_t rssi, int8_t snr,
			    void *user_data)
{
	char rx_buf[MAX_DATA_LEN + 1];
	uint16_t print_size;

	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	if (size != TEAM_PAYLOAD_PREFIX_LEN + 1 ||
	    memcmp(data, TEAM_PAYLOAD_PREFIX, TEAM_PAYLOAD_PREFIX_LEN) != 0 ||
	    data[TEAM_PAYLOAD_PREFIX_LEN] < '0' ||
	    data[TEAM_PAYLOAD_PREFIX_LEN] > '9') {
		return;
	}

	if (gpio_is_ready_dt(&leds[active_led_idx])) {
		gpio_pin_set_dt(&leds[active_led_idx], 0);
	}
	active_led_idx = (active_led_idx + 1) % ARRAY_SIZE(leds);
	if (gpio_is_ready_dt(&leds[active_led_idx])) {
		gpio_pin_set_dt(&leds[active_led_idx], 1);
	}

	print_size = MIN(size, MAX_DATA_LEN);
	memcpy(rx_buf, data, print_size);
	rx_buf[print_size] = '\0';

	LOG_INF("RX RSSI: %d dBm | SNR: %d dB | Payload: %s",
		rssi, snr, rx_buf);
}

int main(void)
{
	const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
	struct lora_modem_config config = {
		.frequency = 433920000,
		.bandwidth = BW_500_KHZ,
		.datarate = SF_5,
		.coding_rate = CR_4_5,
		.preamble_len = 12,
		.tx = false,
		.iq_inverted = false,
		.public_network = false,
	};
	int ret;

	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s device not ready", lora_dev->name);
		return 0;
	}

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (gpio_is_ready_dt(&leds[i])) {
			gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		}
	}
	if (gpio_is_ready_dt(&leds[0])) {
		gpio_pin_set_dt(&leds[0], 1);
	}

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed: %d", ret);
		return 0;
	}

	LOG_INF("Team 15 high-throughput RX: 433.92 MHz, BW500, SF5, CR4/5");

	ret = lora_recv_async(lora_dev, lora_receive_cb, NULL);
	if (ret < 0) {
		LOG_ERR("Failed to start async receive: %d", ret);
		return 0;
	}

	k_sleep(K_FOREVER);
	return 0;
}
