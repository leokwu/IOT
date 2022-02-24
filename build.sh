rm -rf serial_sample
gcc main.c src/serial_control.c src/device_manager.c src/package_control.c src/message_control.c src/mqtt_async_control.c -I ./include/ -L /usr/local/lib/ -lpthread -lcjson -lpaho-mqtt3as -o serial_sample
#rm -rf mqtt_sample
#gcc src/mqtt_async_control.c -I ./include/ -L /usr/local/lib/ -lpthread -lcjson -lpaho-mqtt3as -o mqtt_sample
