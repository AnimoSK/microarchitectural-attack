# SummerProject

Flush-Reload Attack on AES' T-table implementation.
[*Note: In order to perform the attack in a normal environment follow just the server steps and replace server.c with attack.c file in the steps where needed accordingly*]

### Server:

- gcc calibration.c -o calibration

- ./calibration

[*Change the MIN_CACHE_MISS_CYCLES value accordingly in the client.c file*]

- gcc -c -Wall -Werror -fpic encrypt.c

- gcc -shared -o libencrypt.so encrypt.o

- nm libencrypt.so | grep Te[0-4]

[*To get the t-table addresses and change the values in probe array in client.c (attack.c if attack is performed in a normal environment) file accordingly*]

- gcc server.c -L/mnt/c/Users/Satyaki/Desktop/SummerProject/ -lencrypt -o server

[*Note: replace the above mentioned path to the path where libencrypt.so is located. Most cases it would be the path of your current directory. Also replace server.c with attack.c if attack is performed in a normal environment*]

- export LD_LIBRARY_PATH=/mnt/c/Users/Satyaki/Desktop/SummerProject/:\$LD_LIBRARY_PATH

[*Note: replace the above mentioned path to the path where libencrypt.so is located. Most cases it would be the path of your current directory*]

- ./server

### Client:

- gcc -o client client.c

- ./client
