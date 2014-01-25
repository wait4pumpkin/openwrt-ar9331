#!/bin/sh
#
# File Name: scan_wlan0.sh
# System Environment: GNU/Linux
# Created Time: 2013-03-27
# Author: Franky
# E-mail: gaoshanzhishui@163.com
# Description: 
#
#########################################################################

SSID="$2"

encryption="$3"

SSIDPWD="$4"

BSSID=

scan_wlan0_interface ()
{
	SCAN=$(iw wlan0 scan | awk '/^BSS/{print $2}''/SSID:/{split($0,ssid,": "); print gensub(" ", "+", "g", ssid[2]) }')
	BSSID=$(echo  $SCAN | awk '{i=1; while(i<=NF){if($i=="'$SSID'"){print $(i-1)};i++}}') 
}

check_mode ()
{
	checkip=$(ifconfig wlan0 | awk '/inet /{print $2}')
	if [ "$checkip" == "" ]; then
		set_ap_mode
	else
		ntpd -q -p 210.72.145.44
		killall -9 upnpdevice
		/etc/init.d/upnpdevice restart;
		killall -9 shairport
		killall -9 avahi-publish-service
		/etc/init.d/airplay restart;
		killall -9 httpclient
		/sbin/httpclient restart;

		irsend SEND_ONCE NEC CONNECT;

#		killall -9 monitor
#		/etc/init.d/monitor restart
	fi
}

set_sta_mode ()
{
	if [ "$BSSID" != "" ]; then

		killall -9 monitor
		killall -9 upnpdevice
		killall -9 shairport
		killall -9 avahi-publish-service
		killall -9 httpclient

		uci set network.wwan=interface;
		uci set network.wwan.proto=dhcp; 
		uci commit network;

		uci set wireless.@wifi-device[0].txpower=27;
		uci set wireless.@wifi-device[0].channel=11;
		uci set wireless.@wifi-device[0].country=US;
		uci set wireless.@wifi-iface[0].network=wwan;
		uci set wireless.@wifi-iface[0].ssid="${SSID//+/ }";
		uci set wireless.@wifi-iface[0].device=radio0;
		uci set wireless.@wifi-iface[0].mode=sta;
		uci set wireless.@wifi-iface[0].bssid=$BSSID;

		if [ "$encryption" == "WEP" ]; then
			uci set wireless.@wifi-iface[0].encryption=wep;
		elif [ "$encryption" == "WPA" ]; then
			uci set wireless.@wifi-iface[0].encryption=psk;
		elif [ "$encryption" == "NONE" ]; then
			uci set wireless.@wifi-iface[0].encryption=node;
		else
			uci set wireless.@wifi-iface[0].encryption=psk2;
		fi

		if [ "$encryption" != "NONE" ]; then
			uci set wireless.@wifi-iface[0].key=$SSIDPWD;
		fi

		uci commit wireless;

		uci set firewall.@zone[1].forward=ACCEPT;
		uci set firewall.@zone[1].input=ACCEPT;
		uci set firewall.@zone[1].network='wan wwan';
		uci commit firewall;
	
		/etc/init.d/network restart;

		ifconfig br-lan 192.168.20.1;
	fi
	
	sleep 30
	check_mode
}

set_ap_mode ()
{
	tag=`awk -F"=" '{print $2}' /etc/webxml/ssid`
	SSID="Haiya-Music-$tag"

	killall -9 monitor
	killall -9 upnpdevice
	killall -9 shairport
	killall -9 avahi-publish-service
	killall -9 httpclient

	irsend SEND_ONCE NEC RESET;

	uci delete network.wwan;
	uci commit network;

	uci set wireless.@wifi-iface[0].network=lan;
	uci set wireless.@wifi-iface[0].encryption=node;
	uci set wireless.@wifi-device[0].channel=11;
	uci set wireless.@wifi-iface[0].ssid=$SSID;
	uci set wireless.@wifi-iface[0].mode=ap;
	uci delete wireless.@wifi-iface[0].bssid;
	uci delete wireless.@wifi-iface[0].key;
	uci delete wireless.radio0.txpower;
	uci delete wireless.radio0.country;
	uci commit wireless;

	uci set firewall.@zone[1].network='wan';
	uci commit firewall;
	/etc/init.d/network restart;

	/etc/init.d/upnpdevice restart;
	/etc/init.d/airplay restart;
	/sbin/httpclient restart;
}

case $1 in 
	"--ap"| "-ap")
	set_ap_mode
		;;
	"--sta"| "-sta")
	scan_wlan0_interface
	set_sta_mode
		;;
	"--check"| "-ch")
	check_mode
		;;
	*)
		if [ -z $1 ];then	
			scan_wlan0_interface
			else
			echo "Invalid option:$1"
		fi
		;;
esac 
