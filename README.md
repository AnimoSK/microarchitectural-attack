# SummerProject

Flush-Reload Attack on AES' T-table implementation in a client-server scenario

### Server:

- gcc -c -Wall -Werror -fpic encrypt.c

- gcc -shared -o libencrypt.so encrypt.o

- readelf -a libencrypt.so > encrypt.txt

- vi encrypt.txt

[*Type /Te0 and note the offset addresses of T-tables and replace the values in probe array in client.c*]

- gcc server.c -L/mnt/c/Users/Satyaki/Desktop/SummerProject/ -lencrypt -o server

[*Note: replace the above mentioned path to the path where libencrypt.so is located. Most cases it would be the path of your current directory*]

- export LD_LIBRARY_PATH=/mnt/c/Users/Satyaki/Desktop/SummerProject/:\$LD_LIBRARY_PATH

[*Note: replace the above mentioned path to the path where libencrypt.so is located. Most cases it would be the path of your current directory*]

- ./server

### Client:

- gcc calibration.c -o calibration

- ./calibration

[*Change the MIN_CACHE_MISS_CYCLES value accordingly in the client.c file*]

- gcc -o client client.c

- ./client
