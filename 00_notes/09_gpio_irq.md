## Verification of GPIO interface

- First you need to verify whether the gpio is accessible using
```
// Checking if the gpio is used by any driver
// Get the Linux GPIO descriptor no. shown which is used in code implementation
sudo cat /sys/kernel/debug/gpio | grep 17

// Verify if the drivers for gpio are updated with the kernel
uname -r
modinfo gpio_irq.ko | grep vermagic

// Checking if the command line gpio control works
gpioset gpiochip0 17=0
gpioget gpiochip0 17
```


## Code Logic Explained

### **1. Module metadata**

```
MODULE_LICENSE("GPL"); MODULE_AUTHOR("Johannes 4 GNU/Linux"); MODULE_DESCRIPTION("A simple LKM for a gpio interrupt");
```

- Kernel macros that describe the module.
- `MODULE_LICENSE("GPL")` → ensures GPL compatibility, required for using some kernel symbols.
- `MODULE_AUTHOR` and `MODULE_DESCRIPTION` → informational, shown in `modinfo`.

---

### **2. Global variable**

```
unsigned int irq_number;
```

- Holds the **IRQ number** that the kernel assigns to GPIO 529 (which corresponds to physical GPIO17 on your Pi).
- Used later when requesting/freeing the interrupt.

---

### **3. Interrupt handler**

```
static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
	printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
	return IRQ_HANDLED;
}
```

- This is the **Interrupt Service Routine (ISR)**.
- Triggered when the configured GPIO interrupt occurs (here: rising edge).
- Prints a kernel log message and returns `IRQ_HANDLED` to tell the kernel the interrupt was processed.

---

### **4. Module init function**

```
static int __init ModuleInit(void) {
	printk("qpio_irq: Loading module... ");
```

- Called when the module is loaded using `insmod`.
- Logs that initialization has started.    

---

#### **4.1 Requesting GPIO**

```
    if(gpio_request(529, "rpi-gpio-17")) {
		printk("Error!\nCan not allocate GPIO 17\n");
		return -1;
	}
```

- Asks the kernel for exclusive access to GPIO 529.
    
- If already used by another driver, it fails.
    

---

#### **4.2 Configuring as input**

```
    if(gpio_direction_input(529)) {
		printk("Error!\nCan not set GPIO 17 to input!\n");
		gpio_free(529);
		return -1;
	}
```

- Sets GPIO 529 direction as **input** (needed for interrupts).
- On error, frees the pin and exits.

---

#### **4.3 Mapping GPIO → IRQ**

```
    irq_number = gpio_to_irq(529);
```

- Converts GPIO 529 into its **IRQ number**.
- This number is used by the interrupt controller.    

---

#### **4.4 Requesting IRQ**

```
    if(request_irq(
	    irq_number,
		gpio_irq_handler,
		IRQF_TRIGGER_RISING,
		"my_gpio_irq",
		NULL
	) != 0) {
		printk("Error!\nCan not request interrupt nr.: %d\n", irq_number);
		gpio_free(529);
		return -1;
	}
```

- Registers the interrupt with the kernel.
- Parameters:
    - `irq_number` → which interrupt line
    - `gpio_irq_handler` → function to call
    - `IRQF_TRIGGER_RISING` → trigger on rising edge
    - `"my_gpio_irq"` → name shown in `/proc/interrupts`
    - `NULL` → no custom device ID

- If it fails, the GPIO is freed.

---

#### **4.5 Success message**
```
    printk("Done!\n");
	printk("GPIO 17 is mapped to IRQ Nr.: %d\n", irq_number);
	return 0;
}
```

- Prints success logs and exits with `0`.

---

### **5. Module exit function**

```
static void __exit ModuleExit(void) {
	pr_info("gpio_irq: Unloading module... ");
	free_irq(irq_number, NULL);
	gpio_free(529);
}
```

- Called when the module is removed (`rmmod`).
- `free_irq` → releases the interrupt line.
- `gpio_free` → releases GPIO 529 for reuse.

---

### **6. Register init/exit**

```
module_init(ModuleInit);
module_exit(ModuleExit);
```

- Registers the entry (`ModuleInit`) and exit (`ModuleExit`) functions with the kernel.

---