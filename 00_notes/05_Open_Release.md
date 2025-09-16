- This driver is basically used when we read or write device files for a specific device number.
- Here we have the `open` function which executes when the file is opened.
- `open` and `release` function has 2 arguments
	- [inode](https://elixir.bootlin.com/linux/v6.16.7/source/include/linux/fs.h#L2151) 
		- Using `inode` we can fetch the major device number and minor device number using `imajor` and `iminor` macros.
	- [file](https://elixir.bootlin.com/linux/v6.16.7/source/include/linux/fs.h#L1070) 
		- `f_mode` : Used to check the file permissions, file status
		- `f_pos` : Current position in open file (default is 0)
		- `file_operation` : To specify the file operations for a device file
		- `f_flag` : All the flags that are set when file is open. (We can also use `fmode` to check the file status)

- Now when the device file is opened or closed we can check with which permissions it's open what's the major device number, minor device number we opened, etc.
```
# bash 1 (To load the kernel modules and open/release the device files)
sudo insmod {filename}.ko // Get the device number
sudo mknod /dev/my_ch_dev c {dev_num} 0
g++ test.c -o test
./test /dev/my_ch_dev

# bash 2 (To monitor the logs)
sudo dmesg -WT
```

- Hence, here we can see each time the device file is opened we can view the permissions it's opened, position of the current file open, etc using the `open/release` functions from `file_operations` struct.

- Here to pass in the args we need to pass the 