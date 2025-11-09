Includes these header files : 
```
#include <linux/mod.h> // Used to get the linux system modules
#include <linux/init.h> // Used to get the linux init modules
```

- Create a init module :
	- It should return
		- 0 -> the module of loaded successfully
		- -ve flag (number) -> Issue while loading the module
	- Use module_init(my_init) to initialize the driver

- Create a exit module : 
	- To clean stop the driver execution
	- Use module_exit(my_exit) to stop the driver

To load the module into linux system
- create a makefile to compile code into a loadable module (.ko file)
- Monitor logs -> `sudo dmesg -W`
- Load the module -> `sudo insmod {filename.ko}`
- Remove the module -> `sudo rmmod {filename.ko}`
- To check whether driver is loaded -> `lsmod | grep {modulename}`
- To check the description of the driver -> `modinfo {filename.ko}`
	- Inbuilt kernel modules have a intree flag as true (eg. modinfo industrialio)
- To load a kernel module with all it's dependencies -> `modprobe {modulename}`
	- The kernel modules which are in /lib/modules can only be loaded using modprobe
