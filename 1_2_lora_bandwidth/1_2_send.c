/*
 * ESE5180 Lab 1.2 - Team 15 high-throughput LoRa transmitter
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
#define MAX_DATA_LEN 12
#define FCC_MIN_QUIET_MS 10000U
#define NOMINAL_CODED_BITRATE_BPS 62500U

BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(DEFAULT_RADIO_NODE),
	     "No default LoRa radio specified in devicetree");

LOG_MODULE_REGISTER(lora_bandwidth_send, LOG_LEVEL_INF);

static const struct gpio_dt_spec led_red =
	GPIO_DT_SPEC_GET(DT_NODELABEL(red_led_3), gpios);

static uint8_t data[MAX_DATA_LEN] = {
	'e', 's', 'e', '5', '1', '8', '0', 't', '1', '5', '-', '0'
};

int main(void)
{
	const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
	struct lora_modem_config config = {
		.frequency = 433920000,
		.bandwidth = BW_500_KHZ,
		.datarate = SF_5,
		.coding_rate = CR_4_5,
		.preamble_len = 12,
		.tx_power = -10,
		.tx = true,
		.iq_inverted = false,
		.public_network = false,
	};
	uint32_t airtime_ms;
	uint32_t sleep_ms;
	int ret;

	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s device not ready", lora_dev->name);
		return 0;
	}

	if (gpio_is_ready_dt(&led_red)) {
		gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
	}

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed: %d", ret);
		return 0;
	}

	airtime_ms = lora_airtime(lora_dev, MAX_DATA_LEN);
	sleep_ms = MAX(airtime_ms * 30U, FCC_MIN_QUIET_MS);

	LOG_INF("Team 15 high-throughput TX: 433.92 MHz, BW500, SF5, CR4/5");
	LOG_INF("Nominal coded LoRa bitrate: %u bps",
		NOMINAL_CODED_BITRATE_BPS);
	LOG_INF("12-byte packet airtime: %u ms; quiet period: %u ms",
		airtime_ms, sleep_ms);

	while (1) {
		if (gpio_is_ready_dt(&led_red)) {
			gpio_pin_set_dt(&led_red, 1);
		}

		ret = lora_send(lora_dev, data, MAX_DATA_LEN);

		if (gpio_is_ready_dt(&led_red)) {
			gpio_pin_set_dt(&led_red, 0);
		}

		if (ret < 0) {
			LOG_ERR("LoRa send failed: %d", ret);
			return 0;
		}

		LOG_INF("Sent %c; configured packet airtime: %u ms",
			data[MAX_DATA_LEN - 1], airtime_ms);

		if (data[MAX_DATA_LEN - 1] == '9') {
			data[MAX_DATA_LEN - 1] = '0';
		} else {
			data[MAX_DATA_LEN - 1]++;
		}

		k_sleep(K_MSEC(sleep_ms));
	}

	return 0;
}
