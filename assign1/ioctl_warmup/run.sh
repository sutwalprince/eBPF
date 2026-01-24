#!/bin/bash

# Build and test script for ioctl driver

set -e  # Exit on error

echo "======================================"
echo "Simple ioctl Driver - Build and Test"
echo "======================================"

# Check if running as root for module operations
check_root() {
    if [ "$EUID" -ne 0 ]; then 
        echo "Note: Module load/unload requires sudo"
    fi
}

# Build kernel module
build_module() {
    echo ""
    echo "[1/5] Building kernel module..."
    make clean
    make
    if [ -f simple_ioctl.ko ]; then
        echo "✓ Kernel module built successfully"
    else
        echo "✗ Failed to build kernel module"
        exit 1
    fi
}

# Build user space program
build_userspace() {
    echo ""
    echo "[2/5] Building user space program..."
    gcc -o test_ioctl test.c
    if [ -f test_ioctl ]; then
        echo "✓ User space program built successfully"
    else
        echo "✗ Failed to build user space program"
        exit 1
    fi
}

# Load module
load_module() {
    echo ""
    echo "[3/5] Loading kernel module..."
    sudo insmod simple_ioctl.ko
    sleep 1
    if lsmod | grep -q simple_ioctl; then
        echo "✓ Module loaded successfully"
    else
        echo "✗ Failed to load module"
        exit 1
    fi
}

# Check device file
check_device() {
    echo ""
    echo "[4/5] Checking device file..."
    if [ -c /dev/simple_ioctl ]; then
        echo "✓ Device file created: /dev/simple_ioctl"
        ls -l /dev/simple_ioctl
    else
        echo "✗ Device file not found"
        exit 1
    fi
}

# Run test
run_test() {
    echo ""
    echo "[5/5] Running test program..."
    echo "--------------------------------------"
    ./test_ioctl
    echo "--------------------------------------"
}

# Show kernel logs
show_logs() {
    echo ""
    echo "Recent kernel logs:"
    echo "--------------------------------------"
    sudo dmesg | grep simple_ioctl | tail -10
    echo "--------------------------------------"
}

# Cleanup function
cleanup() {
    echo ""
    echo "Cleaning up..."
    if lsmod | grep -q simple_ioctl; then
        sudo rmmod simple_ioctl
        echo "✓ Module unloaded"
    fi
}

# Main execution
main() {
    check_root
    build_module
    build_userspace
    load_module
    check_device
    run_test
    show_logs
    
    echo ""
    echo "======================================"
    echo "Test completed successfully!"
    echo "======================================"
    echo ""
    echo "To clean up, run: sudo rmmod simple_ioctl"
    echo "To view logs, run: sudo dmesg | grep simple_ioctl"
}

# Trap for cleanup on interrupt
trap cleanup EXIT INT TERM

main