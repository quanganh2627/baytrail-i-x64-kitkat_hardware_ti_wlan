#!/bin/bash

OLDSWITCH_PATH=`pwd`
#croot 
[ $? -ne 0 ] && BAD_ENV=1 || BAD_ENV=0
ABSP_HOME="$HOME/gingerbread-stable"

#builtin source $HOME/gingerbread-stable/build/envsetup.sh

if [ $BAD_ENV -eq 1 ]
then
 	echo "Please source \$ABSP/build/envsetup.sh - Exiting early."
	SRC_TEST=`echo $0 | sed s/sh/ok/ | grep ok ; echo $?`
	[ "x$SRC_TEST" != "x" ] && echo "..and please source this script."
else
	echo Found ABSP in $ABSP_HOME
	adb="${ABSP_HOME}/out/host/linux-x86/bin/adb"
	MODPATH="/system/bin"
	LSMOD=`$adb shell lsmod | grep tiwlan_drv`


if [ -n "$LSMOD" ]
then
	echo "Phone is currently in STA mode"
	STATUS="sta"
else
	echo "Phone is currently in AP mode"
	STATUS="ap"
fi

if [ "$#" -ne 1 ] ; then
    echo "Usage: $0 [ap|sta]"
elif [ "$1" == "ap" -a "$STATUS" == "sta" ]
then
        echo "Phone switching to AP mode"
	sleep 1
	echo "Taking down tiwlan0 "
	$adb shell ifconfig tiwlan0 down
	$adb shell sleep 1
	PID_DHCP=`$adb shell ps | grep dhcp | awk '{print $1}'`
	if [ -n "$PID_DHCP" ]
	then
		echo "Killing dhcp daemon $PID_DHCP"
		$adb shell kill $PID_DHCP
		sleep 1
	fi
	$adb shell ${MODPATH}/rmmod tiwlan_drv
	$adb shell ${MODPATH}/insmod /lib/modules/tiap_drv.ko
	$adb shell /system/bin/tiap_loader -f /etc/wifi/firmware_ap.bin -i /etc/wifi/tiwlan_ap.ini -e /etc/wifi/nvs_map.bin
	$adb shell ifconfig tiap0 192.168.0.99 up
	$adb shell sleep 2
	PID_HOSTAP=`$adb shell ps | grep hostap`
	if [ "x$PID_HOSTAP" == "x" ]
	then
		echo "Launching hostap daemon"
		$adb shell hostap -d /etc/wifi/hostapd.conf > /dev/null & 
	fi
	echo "Launching dnsmasq daemon"
	$adb shell dnsmasq --no-daemon --no-resolv --no-poll --dhcp-range=192.168.0.25,192.168.0.30 &
	sleep 5
	echo "Phone is in AP mode, IF: tiap0, IP: 192.168.0.99, SSID: ap_test"
	STATUS="ap"
elif [ "$1" == "ap" -a "$STATUS" == "ap" ]
then
	echo "Phone in AP mode"
	sleep 1
	echo "Keeping up tiap0 "
	$adb shell ifconfig tiap0 192.168.0.99 up
	$adb shell sleep 2
	PID_HOSTAP=`$adb shell ps | grep hostap | awk '{print $1}' `
	if [ "x$PID_HOSTAP" != "x" ]
	then
		echo "Killing hostap deamon"
		$adb shell killall hostap
		sleep 1
		echo "Re-Launching hostap daemon"
		$adb shell hostap -d /etc/wifi/hostapd.conf > /dev/null & 
	else
		echo "Can't find hostap deamon !"
		echo "Re-Launching hostap daemon"
		$adb shell hostap -d /etc/wifi/hostapd.conf > /dev/null & 
	fi
	PID_DNSMASQ=`$adb shell ps | grep dnsmasq | awk '{print $1}'`
	if [ -n "$PID_DNSMASQ" ]
	then
		echo "Kill dnsmasq $PID_DNSMASQ"
		$adb shell kill $PID_DNSMASQ
		sleep 2
	else 
		echo "Can't find dnsmaq process !"
	fi
	echo "Re-Launching dnsmasq daemon"
	$adb shell dnsmasq --no-daemon --no-resolv --no-poll --dhcp-range=192.168.0.25,192.168.0.30 &
	sleep 5
	echo "Phone is STILL in AP mode, IF: tiap0, IP: 192.168.0.99, SSID: ap_test"
elif [ "$1" == "sta" -a "$STATUS" == "ap" ]
then
        echo "Phone switching to STA mode"*
	sleep 1
	echo "Taking down tiap0"
	$adb shell ifconfig tiap0 down
	$adb shell sleep 1

	PID_HOSTAP=`$adb shell ps | grep hostap | awk '{print $1}'`
	if [ -n "$PID_HOSTAP" ]
	then
		echo "Killing hostap daemon $PID_HOSTAP"
		$adb shell kill $PID_HOSTAP
		sleep 1
	fi
	PID_DNSMASQ=`$adb shell ps | grep dnsmasq | awk '{print $1}'`
	if [ -n "$PID_DNSMASQ" ]
	then
		echo "Kill dnsmasq $PID_DNSMASQ"
		$adb shell kill $PID_DNSMASQ
		sleep 2
	fi
	$adb shell ${MODPATH}/rmmod tiap_drv
	$adb shell sleep 1
	$adb shell ${MODPATH}/insmod /lib/modules/tiwlan_drv.ko
	$adb shell sleep 1
	echo "Starting wlan_loader"
	$adb shell /system/bin/wlan_loader -f /system/etc/wifi/firmware.bin -i /system/etc/wifi/tiwlan.ini -e /system/etc/wifi/nvs_map.bin
	$adb shell sleep 1
	$adb shell ifconfig tiwlan0 up
	echo "Phone is in STA mode, IF: tiwlan0"
	STATUS="sta"
else
	echo "Usage: $0 [ap|sta]"
fi
fi

builtin cd ${OLDSWITCH_PATH}


