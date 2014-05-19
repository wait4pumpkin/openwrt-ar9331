#!/bin/sh

wget $1 -O /tmp/firmware

md5sum=$(md5sum /tmp/123 | awk '{print $1}')

if [ "$md5sum"=="$2" ]; then
	/sbin/sysupgrade /tmp/firmware  
else
	rm /tmp/firmware
fi
