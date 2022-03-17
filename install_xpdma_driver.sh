#!/bin/bash
set -e

cd driver
make

DEVICE="xpdma"
DEV_PATH="/dev/${DEVICE}"

# Remove module (if loaded)
sudo make unload > /dev/null || echo "xpdma has not been loaded!"

# Insert module 
sudo make load

cd ../software
make
