
SPI driver is used to communicate with 2 microcontrollers

![[00_notes/_images/021_SPI_Introduction.png]]

![[00_notes/_images/022_SPI_components.png]]

![[00_notes/_images/023_SPI_Overview.png]]

![[00_notes/_images/024_SPI_Chip_Select_Pin.png]]

![[00_notes/_images/025_SPI_MOSI.png]]

![[00_notes/_images/026_SPI_MISO.png]]

![[00_notes/_images/027_SPI_Clk_Polarity.png]]

![[00_notes/_images/028_Clk_Phase.png]]

![[00_notes/_images/029_SPI_Clk_Modes.png]]

![[00_notes/_images/030_SPI_Configurations.png]]

---

## Process to enable and connect to SPI interface

- First run `sudo raspi-config` and enable spi interfacing and reboot the system.
```
interface-options -> SPI -> enable (Yes)
sudo reboot
```

- Now check if the SPI interface is enabled
```
ls /dev/ | grep spi
```

---

## Code logic explained

## **1. Module metadata**

```
MODULE_LICENSE("GPL"); MODULE_AUTHOR("Johannes 4 GNU/Linux"); MODULE_DESCRIPTION("STM32 SPI driver with read/write helpers");
```

- These are **kernel macros** that describe the module.
- `MODULE_LICENSE("GPL")` ensures the module is GPL-compatible. Without this, some symbols in the kernel cannot be used.
- `MODULE_AUTHOR` and `MODULE_DESCRIPTION` are informational.

---

## **2. Global SPI device pointer**

```
static struct spi_device *stm32_dev;
```

- This **global pointer** will store the reference to the STM32 SPI device.
- Once `probe()` is called, the kernel passes a `struct spi_device *` that represents the hardware device; we save it here for later read/write operations.    

---

## **3. SPI write helper**

```
static int stm32_write(
	struct spi_device *spi,
	u8 *data,
	size_t len)
{
	struct spi_transfer t = {
		.tx_buf = data,
		.len = len,
	};
	struct spi_message m;
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(spi, &m);
}
```

- **Purpose:** Send data to STM32 via SPI.
- **`struct spi_transfer`**: Describes one SPI transaction.
    - `.tx_buf` → pointer to data to send
    - `.len` → number of bytes to send
- **`struct spi_message`**: A message can include multiple transfers (here we only have one).
- **`spi_message_init()`** → initializes the message
- **`spi_message_add_tail()`** → adds our transfer to the message
- **`spi_sync()`** → sends the message **synchronously** to the device and waits for completion

---

## **4. SPI read helper**

```
static int stm32_read(
	struct spi_device *spi, 
	u8 *tx_data,
	u8 *rx_data,
	size_t len)
{
	struct spi_transfer t = {
		.tx_buf = tx_data,
		.rx_buf = rx_data,
		.len = len,
	};
	struct spi_message m;
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(spi, &m);
}
```

- **Purpose:** Read data from STM32.
- SPI is **full-duplex**, so even when reading, you must transmit something (often a dummy byte or a command).
- `.tx_buf` → the command or dummy data sent to the device 
- `.rx_buf` → where the received data is stored
- `.len` → number of bytes to read/write
- Otherwise, it works like the write helper.

---

## **5. Probe function**

```
static int stm32_probe(struct spi_device *spi) {
	u8 cmd = 0x01;     // example command
	u8 rx_buf[2] = {0};

	int ret;
	printk(KERN_INFO "STM32 SPI device probed\n");
	stm32_dev = spi;

	/* Configure device */
	stm32_dev->mode = SPI_MODE_3;
	stm32_dev->bits_per_word = 8;
	stm32_dev->max_speed_hz = 1000000;
	spi_setup(stm32_dev);
```

- **`probe()`** is called by the kernel **when the SPI device exists** (either via device tree or board info).
- `stm32_dev = spi` → save device pointer for future use
- **Device configuration:**
    - `mode = SPI_MODE_3` → clock polarity/phase (CPOL=1, CPHA=1)
    - `bits_per_word = 8` → standard byte transfer
    - `max_speed_hz = 1 MHz` → clock speed for communication
- `spi_setup()` applies these settings to the SPI controller.

---

### **Example communication inside probe**

```
    /* Example write */
	ret = stm32_write(stm32_dev, &cmd, 1);
	if (ret)
		printk(KERN_ERR "SPI write failed\n");

	/* Example read */
	ret = stm32_read(stm32_dev, &cmd, rx_buf, 2);
	if (ret)
		printk(KERN_ERR "SPI read failed\n");
	else
		printk(KERN_INFO "Received from STM32: 0x%x 0x%x\n", rx_buf[0], rx_buf[1]);
	
	return 0;
}
```

- Sends `0x01` as a command
- Reads 2 bytes from STM32 in response
- Prints result via `printk` (kernel log, view with `dmesg`)

---

## **6. Remove function**

```
static void stm32_remove(struct spi_device *spi) {
	printk(KERN_INFO "STM32 SPI device removed\n");
}
```

- Called by the kernel **when the SPI device is removed** or driver is unloaded
- Modern kernels expect **`void` return type**, not `int`
- Here it just logs removal

---

## **7. SPI driver structure**

```
static struct spi_driver stm32_spi_driver = {
	.driver = {
		.name = "stm32",
		.owner = THIS_MODULE,
	},
	.probe = stm32_probe,
	.remove = stm32_remove,
};
```

- Defines the **SPI driver** for the kernel
- `.name` → driver name, used for binding
- `.probe` / `.remove` → kernel calls these when device appears/disappears

---

## **8. Module init and exit**

```
static int __init ModuleInit(void) {
	printk(KERN_INFO "STM32 SPI module registering driver...\n");
	return spi_register_driver(&stm32_spi_driver);
}  


static void __exit ModuleExit(void) {
	spi_unregister_driver(&stm32_spi_driver);
	printk(KERN_INFO "STM32 SPI module unregistered\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);
```

- **`ModuleInit`** → called when module is loaded
    - Registers SPI driver with kernel (`spi_register_driver`)
- **`ModuleExit`** → called when module is unloaded
    - Unregisters driver (`spi_unregister_driver`)

---



