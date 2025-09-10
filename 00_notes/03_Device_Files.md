
## Overview of device files

- You can list normal files using 
- Listing normal files : 
```
ls -ltr /home/pi/Programming/Linux_Driver_Tutorial

drwxr-xr-x 2 pi pi 4096 Sep  2 21:28 01_hello
drwxr-xr-x 2 pi pi 4096 Sep  5 20:25 00_notes
drwxr-xr-x 2 pi pi 4096 Sep  8 17:38 02_GPIO_Control
```

- Listing devices files (Drivers) : 
```
ls -lh mmcblk0 mmcblk0p1 tty50 gpiochip0

crw-rw---- 1 root gpio 254,  0 Sep  5 20:47 gpiochip0
brw-rw---- 1 root disk 179,  0 Sep  5 20:46 mmcblk0
brw-rw---- 1 root disk 179,  1 Sep  5 20:47 mmcblk0p1
crw--w---- 1 root tty    4, 50 Sep  5 20:46 tty50
```

- You can see while listing drivers, it shows different content
	- Permissions
		- c (crw-rw----) : stands for [character device](References#Character%20device%20file) file (gpiochip0 is gpio driver)
		- b (brw-rw----) : stands for [block device file](References#Block%20device%20file) (mmcblk0 is sd card driver)
	- Device number (major device number, minor device number)
		- Major device number : Main device number
		- Minor device number : Partitioned device number
		- Examples : 
			- gpiochip0
				- 254 - Major device number
				- 0 - Minor device number
			- [mmcblk0](References#SD%20card%20driver) 
				- 179 - Major device number : sd card driver number
				- 0 - Minor device number : sd card as a whole partition
			- [mmcblk0p1](References#SD%20card%20driver)
				- 179 - Major device number : sd card driver number
				- 1 - Minor device number : partition 1 of sd card

- Serial drivers (tty)
```
$ ls -ltr /dev/tty{1,2,3,4} 
crw--w---- 1 root tty 4, 2 Sep  9 05:51 /dev/tty2
crw--w---- 1 root tty 4, 3 Sep  9 05:51 /dev/tty3
crw--w---- 1 root tty 4, 4 Sep  9 05:51 /dev/tty4
crw--w---- 1 root tty 4, 1 Sep  9 05:51 /dev/tty1
```

- You can check ```/proc/devices``` to view all the character /block devices currently active
```
cat /proc/devices | grep tty
```

- You can view the first bytes of sd card using hexdump on the specific driver
```
$ sudo hexdump /dev/mmcblk0 | head
0000000 b8fa 1000 d08e 00bc b8b0 0000 d88e c08e
0000010 befb 7c00 00bf b906 0200 a4f3 21ea 0006
0000020 be00 07be 0438 0b75 c683 8110 fefe 7507
0000030 ebf3 b416 b002 bb01 7c00 80b2 748a 8b01
0000040 024c 13cd 00ea 007c eb00 00fe 0000 0000
0000050 0000 0000 0000 0000 0000 0000 0000 0000
*
00001b0 0000 0000 0000 0000 030f c803 0000 0000
00001c0 8001 030c ffe0 4000 0000 0000 0010 0300
00001d0 ffe0 7f83 ffff 4000 0010 e400 03a6 0000
```

- You can make the device file using ```mknod``` 
	- Major device number
	- Minor device number
	- Device file name
	- Type of device file (character 'c' or block 'b')
```
$ sudo mknod my_mmc b 179 0
```

- Now you can check the memory of the device file with major device number 179 and minor device number 0
	- In the below example we are reading the contents of sd card as major device number of sd card is 179 and minor device number is 0
```
$ sudo hexdump my_mmc | head
0000000 b8fa 1000 d08e 00bc b8b0 0000 d88e c08e
0000010 befb 7c00 00bf b906 0200 a4f3 21ea 0006
0000020 be00 07be 0438 0b75 c683 8110 fefe 7507
0000030 ebf3 b416 b002 bb01 7c00 80b2 748a 8b01
0000040 024c 13cd 00ea 007c eb00 00fe 0000 0000
0000050 0000 0000 0000 0000 0000 0000 0000 0000
*
00001b0 0000 0000 0000 0000 030f c803 0000 0000
00001c0 8001 030c ffe0 4000 0000 0000 0010 0300
00001d0 ffe0 7f83 ffff 4000 0010 e400 03a6 0000
```

- You can also read and write to device file using major and minor device number
	- In the below case we are writing to serial port and reading to terminal using [`screen`](References#Screen%20command) 
```
$ sudo mknod myserial c 4 64
$ sudo screen myserial 9600
```

## Creating a character device file

- To register a new character device file we use `register_chrdev`
	- param 1 : major device number
		- When we put major device number as 64 here, then all the character devices with major device number 64 and minor device numbers from 0 to 256 will be controlled
		- When we put major device number as 0, then by default kernel will look for a free device number and assign to it.
	- param 2 : name of the character device file
	- param 3 : Do include the struct which does file related operations
```
register_chrdev(0, "hello_cdev", &fops);
```

- Here `fops` is a struct defined with read capabilities, hence we can read the logs from  character device file using `cat` command
```
static ssize_t my_read(struct file *f, char __user *u, size_t l, loff_t *o)
{
	printk("hello_cdev - Read is called\n");
	return 0;
}


static struct file_operations fops = {
	.read = my_read
};
```


- First compile the project and get the device number
```
**bash win 1**
sudo dmesg -W 

**bash win 2**
make
sudo insmod {filename}.ko -- Here get the free device number from bash win 1
sudo rmmod {filename}
```

- Here we have to create the character device files
```
sudo mknod /dev/my_ch_dev c {dev_num} 0
sudo mknod /dev/my_ch_dev0 c {dev_num} 1
```

- Then you can see the logs printing in `bash win 1`
```
sudo insmod {filename}.ko
cat /dev/my_ch_dev
cat /dev/my_ch_dev0
sudo rmmod {filename}
```