# Lab 1.3 Energy Firmware

This app produces repeated LoRa TX pulses for PPK2 measurement. Both builds use
915 MHz, BW125, SF10, CR4/5, an 8-symbol preamble, a 12-byte payload, and a
2-second interval. Only TX power changes.

```sh
# From the repository root, copy the standalone app into the Zephyr workspace.
mkdir -p ~/zephyrproject/lab1_energy
cp -R 1_3_energy/. ~/zephyrproject/lab1_energy/

cd ~/zephyrproject
source .venv/bin/activate

west build --pristine -d build-energy-low \
  -b nucleo_wl55jc/stm32wl55xx lab1_energy -- -DTX_POWER_DBM=-9

west build --pristine -d build-energy-high \
  -b nucleo_wl55jc/stm32wl55xx lab1_energy -- -DTX_POWER_DBM=22

west flash -d build-energy-low --runner openocd
# Start one continuous PPK2 recording, then switch profiles:
west flash -d build-energy-high --runner openocd
```

PPK2 mode: **Ampere meter**, power output enabled, 100 kS/s. The measured peaks
were **39.20 mA at -9 dBm** and **157.89 mA at +22 dBm**.
