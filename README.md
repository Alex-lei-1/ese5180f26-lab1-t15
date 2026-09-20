# ESE5180: Lab 1 Wireless Comms

**Group Number:** 15

| Team Member Name | Email Address |
|---|---|
| Lei, Si Wei | laialex@engineering.upenn.edu |
| Yu, Alexander | ayu2126@engineering.upenn.edu |

**GitHub Repository URL:** https://github.com/Alex-lei-1/ese5180f26-lab1-t15

## 1.1 LoRa Range Challenge

The source files for this section are in `1_1_lora_range/`:

- `1_1_send.c`: FCC-compliant 433.92 MHz LoRa transmitter
- `1_1_receive.c`: matching LoRa receiver with RSSI/SNR logging

### Team and packet format

- Team number: **15**
- Packet format: `ese5180t15-N`
- `N` is a single-digit packet counter that cycles from 0 through 9.

### Range-test result

- Longest successful distance: **TODO: record during the in-person test**
- RSSI at longest successful distance: **TODO dBm**
- SNR at longest successful distance: **TODO dB**
- Test location/conditions: **TODO**

Receiver output has the following form:

```text
RX RSSI: -000 dBm | SNR: 0 dB | Payload: ese5180t15-0 (Active LED: 1)
```

### Configuration used

| Parameter | Value |
|---|---:|
| Frequency | 433.92 MHz |
| Transmit power | -10 dBm |
| Bandwidth | 125 kHz |
| Spreading factor | SF10 |
| Coding rate | 4/5 |
| Preamble | 8 symbols |
| IQ inversion | Disabled |
| Public network | Disabled |

The transmitter calculates airtime at startup and refuses to transmit if a
packet would last one second or longer. After each packet it remains silent for
at least 30 times the packet airtime and never less than 10 seconds.

### Range observations

**What helped increase range?** A clear line of sight, correct antenna
orientation, physical separation from nearby metal/obstructions, and matched
frequency/bandwidth/spreading-factor/coding-rate settings on both boards.

**What are the disadvantages?** Range-oriented LoRa settings reduce data rate,
increase airtime and energy per message, and make the channel more susceptible
to congestion because each packet occupies it longer.

**What is realistic in a product?** A product should choose the lowest airtime
and transmit power that still provide an adequate link margin in the intended
environment. It should also authenticate or identify packets at the application
layer; raw LoRa receivers otherwise accept any packet with matching physical
settings.

### Range-test checklist

1. Build and flash one board with `1_1_send.c` and the other with
   `1_1_receive.c`.
2. Confirm the receiver serial log shows the correct team payload, RSSI, and
   SNR before leaving for the outdoor test.
3. Bring a charged laptop and an offline serial-terminal program.
4. During the test, save the longest successful distance and its RSSI/SNR above.
5. Power off the transmitter whenever it is not actively being tested.

### Build, flash, and operate

The two applications must be built separately. First copy the transmitter from
the cloned repository into a Zephyr sample application:

```sh
cd <path-to-this-cloned-repository>
cp -R ~/zephyrproject/zephyr/samples/drivers/lora/send ~/zephyrproject/lab1_send
cp 1_1_lora_range/1_1_send.c ~/zephyrproject/lab1_send/src/main.c

cd ~/zephyrproject
source .venv/bin/activate
west build --pristine -b nucleo_wl55jc lab1_send
west flash
```

That flashes the transmitter onto the currently connected NUCLEO-WL55JC.
Disconnect it, connect the board that will be the receiver, and then run:

```sh
cd <path-to-this-cloned-repository>
cp -R ~/zephyrproject/zephyr/samples/drivers/lora/receive ~/zephyrproject/lab1_receive
cp 1_1_lora_range/1_1_receive.c ~/zephyrproject/lab1_receive/src/main.c

cd ~/zephyrproject
source .venv/bin/activate
west build --pristine -b nucleo_wl55jc lab1_receive
west flash
```

After flashing, open the receiver's serial port at 115200 baud. On macOS,
identify the port after connecting the board:

```sh
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodemXXXX 115200
```

Replace `XXXX` with the actual port suffix. To close `screen`, press `Ctrl-A`,
then `\`, then confirm. The receiver's blue LED means it is listening; each
validly received LoRa packet advances the active LED from blue to green to red.
The transmitter's red LED is on only during RF transmission, followed by a
blue/green double flash and the required quiet period.
