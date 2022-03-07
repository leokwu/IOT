rm -rf coordinator_process
gcc main.c src/serial_control.c src/device_manager.c src/package_control.c src/message_control.c src/mqtt_async_control.c -I ./include/ -I /usr/local/include/cjson/ -L /usr/local/lib/ -lpthread -lcjson -lpaho-mqtt3as -o coordinator_process
rm -rf mqtt_sample
gcc mqtt_async_homeassistant.c -I ./include/ -I /usr/local/include/cjson/ -L /usr/local/lib/ -lpthread -lcjson -lpaho-mqtt3as -o mqtt_sample
