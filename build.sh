rm -rf serial_sample
gcc main.c src/serial_control.c src/device_manager.c -I ./include/ -L /usr/local/lib/ -lpthread -lcjson -o serial_sample
