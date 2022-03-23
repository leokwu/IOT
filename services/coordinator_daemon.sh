#!/bin/sh
while true ; do
    iscs_detectorAlive=`ps -ef | grep coordinator_service | grep -v grep`
    if [ "$iscs_detectorAlive" == "" ];then
        killall -9 coordinator_service
        nohup /data/coordinator_service > /dev/null 2>&1 &
		echo "restart coordinator_service"
        sleep 1
    fi
    sleep 1
done
