#!/bin/sh

NET_CARD=eth0
cmd1=`ifconfig $NET_CARD up`
cmd2=`dhcp_client_service stop $NET_CARD`
cmd3=`dhcp_client_service start $NET_CARD`
cmd4=`sleep 2`
cmd5=`ifconfig $NET_CARD netmask  255.255.255.0`

a=`ifconfig $NET_CARD | grep "inet addr:"  | grep -v 127.0.0.1`
b=${a%Bcast*}
c=${b##*addr:}
d=${c%.*}
e=$d.1
echo $e

cmd6=`route add default gw $e`

