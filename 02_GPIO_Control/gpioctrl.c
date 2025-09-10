#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/gpio.h>

static struct gpio_desc *led;
static struct task_struct *blink_thread;


static int blink_fn(void *arg)
{
    while (!kthread_should_stop()) {
        gpiod_set_value(led, 1);
        msleep(500);
        gpiod_set_value(led, 0);
        msleep(500);
    }
    return 0;
}

static int __init my_init(void)
{
    int status;

    // led = gpiod_get(NULL, "led", GPIOD_OUT_LOW);  // "led" must be defined in DT overlay
    led = gpio_to_desc(21);
    if (IS_ERR(led)) {
        pr_err("Failed to get LED GPIO\n");
        return PTR_ERR(led);
    }

    blink_thread = kthread_run(blink_fn, NULL, "blink_thread");
    if (IS_ERR(blink_thread)) {
        pr_err("Failed to start blink thread\n");
        gpiod_put(led);
        return PTR_ERR(blink_thread);
    }

    pr_info("LED blink module loaded\n");
    return 0;
}

static void __exit my_exit(void)
{
    kthread_stop(blink_thread);
    gpiod_set_value(led, 0);
    gpiod_put(led);
    pr_info("LED blink module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
