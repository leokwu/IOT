rm -rf serial_sample
gcc main.c src/serial_control.c src/list_control.c -I ./include/ -L /usr/local/lib/ -lpthread -lsqlite3 -o serial_sample
