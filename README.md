# ESE5180 Lab 1: Wireless Communications

**Team 15**

| Team member | Email |
|---|---|
| Lei, Si Wei | laialex@engineering.upenn.edu |
| Yu, Alexander | ayu2126@engineering.upenn.edu |

**Repository:** https://github.com/Alex-lei-1/ese5180f26-lab1-t15

## 1.1 LoRa Range Challenge

Source: `1_1_lora_range/1_1_send.c` and `1_1_receive.c`

Payload: `ese5180t15-N`

| Result | Value |
|---|---:|
| Longest successful distance | **478 m** |
| RSSI at longest distance | **-99 dBm** |
| SNR at longest distance | **-6 dB** |
| Packet airtime | **857 ms** |
| Required quiet period | **25,710 ms** |

![Team 15 range-test serial data](docs/part1-1-range-t15.png)

| Parameter | Value |
|---|---:|
| Frequency | 433.92 MHz |
| TX power | -10 dBm |
| Bandwidth | 125 kHz |
| Spreading factor | SF11 |
| Coding rate | 4/8 |
| Preamble | 16 symbols |

This profile increased range through high processing gain, stronger coding,
and a longer preamble. Clear line of sight, correct antenna orientation, and
distance from metal also improved the link. The disadvantages are low data
rate, long airtime, greater energy per packet, and more channel occupancy. A
real product should use the lowest airtime and power that still provide enough
link margin.

Raw LoRa accepts any packet using matching physical settings. The receiver
therefore validates the `ese5180t15-` prefix and counter so other teams' packets
are ignored.

## 1.2 Trading Range for Bandwidth

Source: `1_2_lora_bandwidth/1_2_send.c` and `1_2_receive.c`

| Parameter | 1.1 range | 1.2 bandwidth |
|---|---:|---:|
| Bandwidth | 125 kHz | **500 kHz** |
| Spreading factor | SF11 | **SF5** |
| Coding rate | 4/8 | **4/5** |
| Preamble | 16 | **12** |
| 12-byte airtime | 857 ms | **4 ms** |
| Nominal coded bit rate | 0.336 kbps | **62.5 kbps** |

Frequency remained 433.92 MHz and TX power remained -10 dBm. BW500, SF5,
CR4/5, and the minimum valid SF5 preamble produced the fastest measured packet
airtime: **4 ms**. The coded-rate calculation includes the 4/5 coding factor;
78.125 kbps would be the uncoded modulation rate. End-to-end verification
received consecutive Team 15 packets:

```text
RX RSSI: -54 dBm | SNR: 9 dB | Payload: ese5180t15-1
RX RSSI: -52 dBm | SNR: 9 dB | Payload: ese5180t15-2
```

**Spreading-factor tradeoff:** Higher SF increases symbol time, sensitivity,
range, airtime, and energy while reducing data rate. Lower SF does the reverse.

**Low-bandwidth applications:** Environmental/agricultural sensors, utility
meters, and remote asset trackers send small, infrequent reports; long range
and battery life matter more than throughput.

## 1.3 Energy Check

Source: `1_3_energy/`. Both builds used 915 MHz, BW125, SF10, CR4/5, an
8-symbol preamble, a 12-byte payload, and a 2-second interval. Only TX power
changed.

| TX power | Peak TX current |
|---:|---:|
| -9 dBm | **39.20 mA** |
| +22 dBm | **157.89 mA** |

![PPK2 low-to-high TX-current transition](docs/part1-3-ppk2-transition.png)

Battery life also depends on airtime, packet frequency, retries, receive
windows, MCU sleep current, peripherals, regulator efficiency, and battery
self-discharge. A coin cell is generally unsuitable for the measured 157.89 mA
pulse because of internal resistance and voltage sag. AA cells or a suitable
LiPo can provide higher pulse current, subject to their datasheet limits.

## 2. LoRaWAN

Source: `2_lorawan/`. Credentials are kept in gitignored `src/secrets.h`;
`secrets.example.h` contains only placeholders. The TTN formatter is
`uplink_decoder.js`.

```json
{"name":"Alex","team":"15","board":"WL55JC"}
```

| Setting | Value |
|---|---|
| Activation | ABP |
| Region / channels | US915 FSB2 (8-15, 65) |
| Data rate | DR3 / SF7BW125 |
| Class / FPort | Class A / 2 |
| Uplink | Unconfirmed, every 10 seconds |
| Payload | 44 bytes, no trailing null |
| TTN RSSI / SNR | -88 dBm / 8.75 dB |

![TTN decoded payload and serial transmission](docs/part2-verification.png)

Build, flash, serial transmission, gateway reception, and decoded JSON all
passed. Removing the sample's `LinkCheckReq` eliminated misleading RX2 timeout
messages while preserving successful unconfirmed uplinks.

### LoRaWAN questions

**Confirmed vs. unconfirmed:** Confirmed uplinks request an acknowledgement and
fit alarms or critical state changes, but acknowledgements/retries cost airtime,
latency, and energy. Unconfirmed uplinks fit replaceable periodic telemetry.

**Reset session and MAC state:** An ABP device and TTN must agree on frame
counters, keys, and MAC state. Reflashing can reset device counters while TTN
retains old values, causing replay rejection; reset resynchronizes the lab
prototype.

**Identifiers and keys:** Raw LoRa provides only radio transport. LoRaWAN uses
DevEUI/DevAddr for identity and routing, NwkSKey for network integrity, and
AppSKey for application confidentiality, allowing many authenticated devices
to share gateway infrastructure.

**Why OTAA:** OTAA performs a join and derives fresh session keys. It improves
provisioning, key rotation, replacement, and network migration, so it is safer
than fixed ABP credentials in real deployments.

## Verification summary

All 1.1, 1.2, 1.3, and LoRaWAN applications built successfully for
`nucleo_wl55jc/stm32wl55xx` with Zephyr 4.4.99. Hardware tests and required
screenshots are included above.
