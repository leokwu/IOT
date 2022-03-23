#!/bin/sh

cmd1=`/data/config_net.sh`

iscs_daemonAlive=`ps -ef | grep coordinator_daemon.sh | grep -v grep`
if [ "$iscs_daemonAlive" == "" ];then
    killall -9 coordinator_daemon.sh
    nohup /data/coordinator_daemon.sh > /dev/null 2>&1 &
    echo "restart coordinator_daemon.sh"
    sleep 1
fi
