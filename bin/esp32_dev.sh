#!/bin/bash
sudo chgrp dialout /dev/ttyS*
sudo chmod g+w /dev/ttyS*

export PATH=$PATH:$HOME/esptool/crosstool-NG/builds/xtensa-esp32-elf/bin
export IDF_PATH=~/esp/esp-idf

cd ~/esp/ltag
echo $@
exec $@
