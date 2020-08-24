# SummerProject

Flush-Reload attack

## Server Steps:

gcc -c -Wall -Werror -fpic encrypt.c

gcc -shared -o libencrypt.so encrypt.o

readelf -a libencrypt.so > encrypt.txt

vi encrypt.txt
[Note: Type /Te0 and note the offset addresses of T-tables and replace the values in probe array in client.c]

gcc server.c -L/mnt/c/Users/Satyaki/Desktop/SummerProject/ -lencrypt -o server

export LD_LIBRARY_PATH=/mnt/c/Users/Satyaki/Desktop/SummerProject/:\$LD_LIBRARY_PATH

./server

## Client Steps:

gcc -o client client.c

./client
