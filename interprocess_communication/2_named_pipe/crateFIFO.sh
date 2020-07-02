#!/bin/bash
#This file just a test that creat named pipe. It's not used by c program.
curPath=$(pwd)
fifo_name="fifo"
ls -l | grep "$fifo_name" > /dev/null
r=$?
if [ $r == 1 ]; then
	mkfifo -m 660 fifo
elif [ $r == 0 ]; then
	#pass
	echo "" > /dev/null
fi

