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
MODULE_LICENSE("GPL");  MODULE_AUTHOR("Johannes 4 GNU/Linux");  MODULE_DESCRIPTION("A simple LKM for a GPIO interrupt");
```

- Kernel macros that describe the module.    
- `MODULE_LICENSE("GPL")` → ensures GPL compatibility, required for using some kernel symbols.
- `MODULE_AUTHOR` and `MODULE_DESCRIPTION` → informational, shown in `modinfo`.

---

### **2. GPIO device struct**

```
struct my_gpio_dev {
	int pin;
	char name[16];
}; 

static struct my_gpio_dev mydev = { .pin = 529, .name = "rpi-gpio-17" };
```

- Defines a struct to hold GPIO info (`pin` number and `name`).    
- `mydev` → global instance for this GPIO pin.

---

### **3. IRQ number and flag**

```
static unsigned int irq_number = 0; static bool irq_requested = false;
```

- `irq_number` → stores the IRQ number corresponding to GPIO 529.
- `irq_requested` → tracks if the IRQ was successfully requested to avoid double free on exit.

---

### **4. Interrupt handler**

```
static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
	struct my_gpio_dev *d = (struct my_gpio_dev *)dev_id;
	printk(KERN_INFO "gpio_irq: Interrupt was triggered!\n");
	printk(KERN_INFO "Interrupt from GPIO pin %d\n", d->pin);
	return IRQ_HANDLED; 
}
```

- ISR called when GPIO interrupt occurs (rising edge).
- `dev_id` → pointer to our `my_gpio_dev` struct.    
- Prints which GPIO triggered the interrupt.
- Returns `IRQ_HANDLED` to tell kernel the interrupt was handled.

---

### **5. Module init function**

```
static int __init ModuleInit(void) {
	int ret;
	printk(KERN_INFO "gpio_irq: Loading module... ");
```

- Called when module is loaded using `insmod`.
- Logs that initialization has started.    

---

#### **5.1 Request GPIO**

```
	ret = gpio_request(mydev.pin, mydev.name);
	if (ret) {
		printk(KERN_ERR "gpio_irq: Cannot allocate GPIO %d\n", mydev.pin);
		return ret;
}
```

- Requests exclusive access to GPIO 529.
- Fails if another driver already uses it.

---

#### **5.2 Set GPIO as input**

```
	ret = gpio_direction_input(mydev.pin);
	if (ret) {
		printk(KERN_ERR "gpio_irq: Cannot set GPIO %d as input\n", mydev.pin);
		gpio_free(mydev.pin);
		return ret;
	}
```

- Sets GPIO 529 as input (required for interrupt).
- Frees the pin and exits on error.

---

#### **5.3 Map GPIO → IRQ**

```
	irq_number = gpio_to_irq(mydev.pin);
	if (irq_number < 0) {
		printk(KERN_ERR "gpio_irq: Cannot map GPIO %d to IRQ\n", mydev.pin);
		gpio_free(mydev.pin);
		return irq_number;
	}
```

- Converts GPIO 529 into its IRQ number.
- Returns negative on error.

---

#### **5.4 Request IRQ**

```
	ret = request_irq(
		irq_number,
		gpio_irq_handler,
		IRQF_TRIGGER_RISING,
		"my_gpio_irq",
		&mydev
	);
	if (ret) {
		printk(KERN_ERR "gpio_irq: Cannot request IRQ %d\n", irq_number);
		gpio_free(mydev.pin);
		return ret;
	} 
	irq_requested = true;
```

- Registers interrupt with kernel. 
- `&mydev` → passed as `dev_id` to ISR.
- Sets `irq_requested` to true if successful, to safely free later.

---

#### **5.5 Success message**

```
	printk(KERN_INFO "gpio_irq: Module loaded successfully\n");
	printk(KERN_INFO "GPIO %d is mapped to IRQ %d\n", mydev.pin, irq_number);
	return 0;
```

- Prints success messages and returns 0.

---

### **6. Module exit function**

```
static void __exit ModuleExit(void) {
	printk(KERN_INFO "gpio_irq: Unloading module... ");
	if (irq_requested)
		free_irq(irq_number, &mydev);
	gpio_free(mydev.pin);
	printk(KERN_INFO "gpio_irq: Module unloaded\n"); 
}
```

- Called when module is removed with `rmmod`.
- Frees IRQ **only if it was requested successfully**.
- Frees GPIO pin for reuse.
- Logs unloading message.

---

### **7. Register init/exit functions**

```
module_init(ModuleInit);
module_exit(ModuleExit);
```

- Tells kernel which functions to call on module load and unload.