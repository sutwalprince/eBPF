#!/bin/bash

function echo_green() {
    echo -e "\e[32m$1\e[0m"
}

function echo_red() {
    echo -e "\e[31m$1\e[0m"
}

## Building the applications
make
if [ $? -ne 0 ]; then
    echo_red "Make failed"
    exit $?
fi
echo_green "Make successful !!"

## Build your ioctl driver and load it here
sudo rmmod ioctl_kernel.ko
sudo insmod ioctl_kernel.ko
if [ $? -ne 0 ]; then
    echo_red "insmod failed"
    exit $?
fi
echo_green "insmod successfull !!"

###############################################

# Launching the control station
gcc -o control_station control_centre.c
gcc -o soldier foot_soldier.c
./control_station 15 &
c_pid=$!
echo "Control station PID: $c_pid"

sleep 5;

# Launching the soldiers
./soldier $c_pid 0 &
s_pid1=$!
echo "Soldier PID (exits): $s_pid1"
sleep 1;

./soldier $c_pid 1  &
s_pid2=$!
echo "Soldier PID (hangs): $s_pid2"
sleep 1;

./soldier $c_pid 1  &
s_pid3=$!
echo "Soldier PID (hangs): $s_pid3"
sleep 1;

./soldier $c_pid 0  &
s_pid4=$!
echo "Soldier PID (exits): $s_pid4"
sleep 1;

wait $c_pid

kill -9 $s_pid1
kill -9 $s_pid2
kill -9 $s_pid3
kill -9 $s_pid4

## Remove the driver here
sudo rmmod driver2.ko


sleep 2
echo_green "to view the relevant dmesg run:"
echo_green "sudo dmesg | grep '[DRIVER2.2]'"