#!/bin/bash
#make
echo "Inserting device"
sudo insmod char_dev.ko
ls -l /dev/chardev
lsmod | grep "char*"

echo "1st open"
cat /dev/chardev
sudo tail -n 3 /var/log/messages

echo "1st read"
echo "Hello Horia 1." > /dev/chardev
sudo tail -n 3 /var/log/messages

echo "2nd read"
cat /dev/chardev
sudo tail -n 3 /var/log/messages

echo "2nd write"
echo "Hello Horia 2." > /dev/chardev
sudo tail -n 3 /var/log/messages

echo "last read"
cat /dev/chardev
sudo tail -n 3 /var/log/messages

#echo "Removing device"
#sudo rmmod char_dev
echo "Done"
#make clean
