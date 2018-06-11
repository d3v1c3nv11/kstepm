#!/bin/bash

#check for root priv
if [ "$(id -u)" != "0" ]; then
        echo -ne "This script must be executed as root. Exiting\n" >&2
        exit 1
fi

SUDOUSER=$(who am i | awk '{print $1}')

if [ "$1" = "load" ];then

if lsmod | grep a20stepm &> /dev/null ; then
  echo "Module already loaded! Exiting..."
  exit 0
else
	insmod a20stepm.ko
	echo "Module loaded"
fi
		
if ls /dev | grep stepm &> /dev/null ; then
  echo "Node existed! Exiting..."
  exit 0
else
        mknod /dev/stepm c 210 0
        chgrp stepm /dev/stepm
        chmod 665 /dev/stepm

        if cat /etc/group | grep stepm &> /dev/null; then
	echo Group stepm exist. Skipping next...
	else
	groupadd stepm
	echo Group stepm added. Adding $SUDOUSER to group
        fi
	if id -Gn $SUDOUSER | grep stepm &> /dev/null; then
	echo $SUDOUSER already member to group stepm
	else 
	usermod -a -G stepm $SUDOUSER
	echo $SUDOUSER added to group stepm
	echo Loging again as $SUDOUSER
	echo exec su -l $SUDOUSER
	exec su -l $SUDOUSER
	fi
	echo "Node created"
fi
elif [ "$1" = "unload" ];then
if lsmod | grep stepm &> /dev/null ; then
	rmmod stepm.ko
	echo "Module unloaded"
fi
if ls /dev | grep stepm &> /dev/null ; then
	rm /dev/stepm
	echo "Node deleted"
fi
else
	echo "usage: stepm (un)load"
fi
