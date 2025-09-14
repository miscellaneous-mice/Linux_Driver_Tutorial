
- To see all the kernel logging options refer to [Linux kernel logging](https://www.kernel.org/doc/html/next/core-api/printk-basics.html) 
- Here we use `sudo dmesg -WT` to view all the kernel logs
- If you want to view only certain logs then we use the -l flag followed by the String number specified in [Linux kernel logging](https://www.kernel.org/doc/html/next/core-api/printk-basics.html). 
```
sudo dmesg -WT -l 1 -> View only the alert logs
sudo dmesg -WT -l 7 -> View only the debug logs
etc.
```