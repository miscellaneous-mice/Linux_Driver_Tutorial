## GDB Basics

How to run gdb
```
gcc -g main.c -o main
gdb -tui ./main
```

Basic commands
- **`run` (or just `r`)** → start the program
- **`break main` (or `b main`)** → set a breakpoint at `main`
- **`break {line no.}` (or `b {line no.}`)** → set a breakpoint at `main`
- **`info breakpoints`** → See all the breakpoints
- **`step` (or `s`)** → step into the next line (go inside functions)
- **`next` (or `n`)** → step over the next line (don’t enter functions)
- **`print a` (or `p a`)** → print the current value of variable `a`
- **`continue` (or `c`)** → resume program until next breakpoint
- **`quit` (or `q`)** → exit gdb

Intermediate commands
- To do operations while debugging
```
(gdb) set var a = 5
(gdb) set var b = 7
(gdb) print print (a * 10) + (b / 2)
```

- Setting watchpoints
```
(gdb) break main
(gdb) run
(gdb) watch b
(gdb) continue
# stops when b changes
```

- Checking the current stack
```
(gdb) backtrace
(gdb) frame 1
(gdb) info locals
(gdb) finish
```

Useful commands in TUI mode
- `layout src` → show **source code** pane
- `layout regs` → show **registers**
- `layout asm` → show **assembly**
- `layout split` → source + assembly together


## Little bit about Tee

- `tee` is used to copy stdin to stdout _and_ file(s). Hence to save something to a file
```
$ echo "hello world" | tee file_msg.txt
$ cat file_msg.txt
hello world

$ echo "new message" | tee -a file_msg.txt
$ cat file_msg.txt
hello world
new message
```

## Reading and Writing to a character device

- Here we have 2 types of buffer
	- driver buffer : the data content of driver file
		- Can write to  `/dev/my_ch_dev` using `tee` command
	- user buffer : the data content of current user
		- Can be read from user buffer (in terminal) using `cat`, `echo`, etc.

- Hence we implement the read functionality to copy the data from **driver buffer to user buffer**. Here 
	- `filep` : just a pointer to the file struct (used to read, write to a file)
	- `user_buf` : user buffer
	- `len` : Length of the driver buffer which is to be copied from `driver_buffer` to `user_buffer`
	- `off` : current position offset of the driver buffer which we read from.
```
static ssize_t my_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off)
```

- Hence we implement the write functionality to copy the data from **user buffer to driver buffer**. Here 
	- `filep` : just a pointer to the file struct (used to read, write to a file)
	- `user_buf` : user buffer
	- `len` : Length of the driver buffer which is to be written from `user_buffer` to `driver_buffer`
	- `off` : current position offset of the driver buffer which we write to.
```
static ssize_t my_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off)
```

- **Notes**
	- `text[64]` : This is the driver buffer with max size of 64 bytes (`char` array)
	- `*off` : This offset is resets to `0` everytime the driver file is closed.

- Algorithm implemented to read the `driver_buffer`
	- For reading first we calculate the current position of the pointer file is to be read from (default 0)
	- Get the length of the `drive buffer` from the current position of pointer and the total length of driver buffer using `len` parameter 
	- Now we get all the driver buffer `text[64]` from the current file pointer (`*off`) using `cat` command and copy to user buffer
	- **NOTE** : 
		- `cat` can only read max of `131072` bytes at a time. Hence, if there is more than `131072` bytes in the `text` buffer then multiple reads are done till remaining length of buffer to be read is 0 (i.e. till the offset `*off` is at the EOF of the driver `/dev/my_ch_dev`)
		- Hence why we return the `remaning buffer length to be read` when there is still more buffer to be read, else we return 0 (which means stop further reading of driver buffer)
		- Here, reading refer to copying data from driver buffer means copy the data from driver buffer to user buffer
```
int not_copied, delta, to_copy = (len + *off) < sizeof(text) ? len : (sizeof(text) - *off);
not_copied = copy_to_user(user_buf, &text[*off], to_copy);
```

- Algorithm implemented to write the `driver_buffer`
	- For reading first we calculate the current position of the pointer file is to be written to (default 0)
	- Get the length of the `drive buffer` from the current position of pointer and the total length of driver buffer using `len` parameter 
	- Now get the user buffer (input from user using `tee` command) and copy to the driver buffer `text[64`
	- **NOTE** :
		- Till all the buffer from the user is written to the driver `text[64]`, we keep reading from the input buffer (`user buffer`)
		- Hence why we return the `remaning buffer length to be written` when there is still more buffer to be written else we return 0 (which means stop further writing to the driver buffer)
		- Here, writing refer to copying data from driver buffer means copy the data from user buffer to driver buffer
```
int not_copied, delta, to_copy = (len + *off) < sizeof(text) ? len : (sizeof(text) - *off);
not_copied = copy_to_user(user_buf, &text[*off], to_copy);
```

- Now the `test.c` is used to read each line of the `driver buffer`. Hence when `my_read` function returns 0 i.e. when offset `*off` reads to the end of the line (end of driver buffer), then we exit from the while loop as `read(fd, &c, 1)` gives out 0 (which means there is no further `driver buffer` to be copied to `user buffer`) . Hence the script stops executing.

- You can read via the `cat` command and write via the `tee` command
```
## terminal 1
sudo insmod main.ko
sudo mknod /dev/my_ch_dev c {dev_num} 0
echo "my bad" | sudo tee -a /dev/my_ch_dev
cat /dev/my_ch_dev

gcc test.c -o test -g
sudo gdb ./test

## terminal 2
sudo dmesg -WT
```

