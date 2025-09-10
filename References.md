
---
## Block device file
---

- Reading or writing blocks for memory (like for sd card : mmcblk0)


---
## Character device file
---

- Reading or writing stream of bytes into memory (like serial port : tty50)


---
## SD card driver
---
### 🔹 `mmcblk0`

- `mmc` = **MultiMediaCard** (covers SD, microSD, and eMMC devices).
- `blk` = **block device** (Linux treats storage devices as block devices).
- `0` = the first MMC device detected (if you had multiple cards/eMMC chips, you’d also see `mmcblk1`, `mmcblk2`, …).

So:  
👉 `mmcblk0` = the **whole SD/eMMC device** (like `/dev/sda` for hard drives).

---
### 🔹 `mmcblk0p1`, `mmcblk0p2`, …

- `p1`, `p2`, … = partitions on that device.
    
- Example:
    - `mmcblk0p1` → the **first partition** (on Raspberry Pi OS this is the FAT32 `/boot` partition).
    - `mmcblk0p2` → the **second partition** (typically the root filesystem `/`).
        

So:  
👉 `mmcblk0p1` = **partition 1 of the first MMC device**.  
👉 `mmcblk0p2` = **partition 2 of the first MMC device**, and so on.

---
### 🔹 Typical Raspberry Pi setup

On a Pi booting from microSD:
- `/dev/mmcblk0` → the entire SD card. 
- `/dev/mmcblk0p1` → the small **boot** partition (FAT32, ~256 MB).
- `/dev/mmcblk0p2` → the larger **root** partition (ext4, takes the rest of the card).


---
## Screen command
---
### 🔹 What `screen` is

- `screen` is a **terminal multiplexer** (lets you run multiple terminal sessions inside one).
- But it also doubles as a very simple **serial terminal** program:
	- `screen /dev/serial0 9600`
	- means: *open the serial device /dev/serial0 at 9600 baud and connect my keyboard/screen to it.*

So when you type characters, `screen` sends them **out the serial port**.  
Anything received on that port is displayed back in your terminal.

---

### 🔹 Why you need it for UART testing

On the Raspberry Pi:
- **TX pin (Pin 8)** → sends data.
- **RX pin (Pin 10)** → receives data.
    
If you short TX and RX, then:
- Whatever you type in `screen` goes TX → RX → back into the Pi.
- `screen` shows you what came back.  
	- This is called a **loopback test**.
- That’s what `screen` saves you from — it just lets you interactively talk to the serial port.

---

✅ **So in short:**  
`screen` is just a program that opens your UART device (like `/dev/serial0`) and lets you type and see data over serial — like a chat window between your keyboard and the Pi’s serial pins.



---
## 
---


