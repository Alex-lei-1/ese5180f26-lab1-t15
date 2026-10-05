/*
 * ESE5180 Lab 1.3 - PPK2 transmit-current measurement
 *
 * Build the same application twice with TX_POWER_DBM=-9 and
 * TX_POWER_DBM=22. All other radio and timing parameters remain identical,
 * so the measured TX-current difference is attributable to output power.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
#define TEST_FREQUENCY_HZ 915000000
#define SEND_INTERVAL K_SECONDS(2)

BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(DEFAULT_RADIO_NODE),
	     "No default LoRa radio specified in devicetree");
BUILD_ASSERT(TX_POWER_DBM == -9 || TX_POWER_DBM == 22,
	     "Use TX_POWER_DBM=-9 for low or 22 for high");

LOG_MODULE_REGISTER(lab1_energy, LOG_LEVEL_INF);

static uint8_t payload[] = "ese5180t15-0";

int main(void)
{
	const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
	struct lora_modem_config config = {
		.frequency = TEST_FREQUENCY_HZ,
		.bandwidth = BW_125_KHZ,
		.datarate = SF_10,
		.coding_rate = CR_4_5,
		.preamble_len = 8,
		.tx_power = TX_POWER_DBM,
		.tx = true,
		.iq_inverted = false,
		.public_network = false,
	};
	int ret;

	if (!device_is_ready(lora_dev)) {
		LOG_ERR("LoRa device is not ready");
		return 0;
	}

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa configuration failed: %d", ret);
		return 0;
	}

	LOG_INF("PPK2 profile: %d dBm, airtime %u ms", TX_POWER_DBM,
		lora_airtime(lora_dev, sizeof(payload) - 1));

	while (1) {
		ret = lora_send(lora_dev, payload, sizeof(payload) - 1);
		if (ret < 0) {
			LOG_ERR("LoRa send failed: %d", ret);
			return 0;
		}

		k_sleep(SEND_INTERVAL);
	}

	return 0;
}
