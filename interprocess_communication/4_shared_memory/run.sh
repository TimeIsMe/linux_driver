#!/bin/bash
gnome-terminal -x ./reader 666
./writer 666 hello
sleep(1)
./writer 666 world
sleep(1)
./writer 666 quit
