#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple LKM for a GPIO interrupt");

/* Struct to hold GPIO info */
struct my_gpio_dev {
    int pin;
    char name[16];
};

/* Global instance */
static struct my_gpio_dev mydev = {
    .pin = 529,
    .name = "rpi-gpio-17"
};

static unsigned int irq_number = 0;
static bool irq_requested = false;

/* Interrupt service routine */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    struct my_gpio_dev *d = (struct my_gpio_dev *)dev_id;
    printk(KERN_INFO "gpio_irq: Interrupt was triggered!\n");
    printk(KERN_INFO "Interrupt from GPIO pin %d\n", d->pin);
    return IRQ_HANDLED;
}

/* Module init */
static int __init ModuleInit(void)
{
    int ret;

    printk(KERN_INFO "gpio_irq: Loading module... ");

    /* Request GPIO */
    ret = gpio_request(mydev.pin, mydev.name);
    if (ret) {
        printk(KERN_ERR "gpio_irq: Cannot allocate GPIO %d\n", mydev.pin);
        return ret;
    }

    /* Set GPIO direction */
    ret = gpio_direction_input(mydev.pin);
    if (ret) {
        printk(KERN_ERR "gpio_irq: Cannot set GPIO %d as input\n", mydev.pin);
        gpio_free(mydev.pin);
        return ret;
    }

    /* Map GPIO to IRQ */
    irq_number = gpio_to_irq(mydev.pin);
    if (irq_number < 0) {
        printk(KERN_ERR "gpio_irq: Cannot map GPIO %d to IRQ\n", mydev.pin);
        gpio_free(mydev.pin);
        return irq_number;
    }

    /* Request IRQ */
    ret = request_irq(irq_number, gpio_irq_handler,
                      IRQF_TRIGGER_RISING, "my_gpio_irq", &mydev);
    if (ret) {
        printk(KERN_ERR "gpio_irq: Cannot request IRQ %d\n", irq_number);
        gpio_free(mydev.pin);
        return ret;
    }

    irq_requested = true; /* mark successful request */

    printk(KERN_INFO "gpio_irq: Module loaded successfully\n");
    printk(KERN_INFO "GPIO %d is mapped to IRQ %d\n", mydev.pin, irq_number);

    return 0;
}

/* Module exit */
static void __exit ModuleExit(void)
{
    printk(KERN_INFO "gpio_irq: Unloading module... ");

    /* Free IRQ only if requested */
    if (irq_requested)
        free_irq(irq_number, &mydev);

    gpio_free(mydev.pin);

    printk(KERN_INFO "gpio_irq: Module unloaded\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

