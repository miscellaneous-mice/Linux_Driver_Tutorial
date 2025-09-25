#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("STM32 SPI driver with read/write helpers");

/* -------------------------------
 * Global SPI device pointer
 * ------------------------------- */
static struct spi_device *stm32_dev;

/* -------------------------------
 * Write helper
 * ------------------------------- */
static int stm32_write(struct spi_device *spi, u8 *data, size_t len)
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

/* -------------------------------
 * Read helper
 * ------------------------------- */
static int stm32_read(struct spi_device *spi, u8 *tx_data, u8 *rx_data, size_t len)
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

/* -------------------------------
 * Probe function
 * ------------------------------- */
static int stm32_probe(struct spi_device *spi)
{
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

/* -------------------------------
 * Remove function
 * ------------------------------- */
static void stm32_remove(struct spi_device *spi)
{
    printk(KERN_INFO "STM32 SPI device removed\n");
}

/* -------------------------------
 * SPI driver definition
 * ------------------------------- */
static struct spi_driver stm32_spi_driver = {
    .driver = {
        .name = "stm32",
        .owner = THIS_MODULE,
    },
    .probe = stm32_probe,
    .remove = stm32_remove,
};

/* -------------------------------
 * Module init/exit using old style
 * ------------------------------- */
static int __init ModuleInit(void)
{
    printk(KERN_INFO "STM32 SPI module registering driver...\n");
    return spi_register_driver(&stm32_spi_driver);
}

static void __exit ModuleExit(void)
{
    spi_unregister_driver(&stm32_spi_driver);
    printk(KERN_INFO "STM32 SPI module unregistered\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

