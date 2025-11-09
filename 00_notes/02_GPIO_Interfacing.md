- To see all the GPIO interfaces : `gpiodetect`
![[Screenshot 2025-11-10 at 12.31.44 AM.png]]

- To use the GPIO modules update your headers
```
sudo apt update
sudo apt install -y raspberrypi-kernel-headers
```

- Manual testing
```
sudo raspi-gpio set 21 op
sudo raspi-gpio set 21 dh
sudo raspi-gpio set 21 dl
```

- To run the kernel module
```
echo "21,0" > /proc/maverick # Turn GPIO 21 to High
echo "21,1" > /proc/maverick # Turn GPIO 21 to Low
```