#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "encrypt.h"

#define PORT 9090
#define SA struct sockaddr

unsigned int key[16] = {0x58, 0x68, 0x61, 0x74, 0x73, 0x20, 0x6D, 0x79, 0x20, 0x4B, 0x75, 0x6E, 0x67, 0x20, 0x46, 0x75};

void communicate(int sockfd)
{
    unsigned int ciphertext[16];
    unsigned int plaintext[16];
    int i;
    for (;;)
    {
        bzero(plaintext, sizeof(unsigned int) * 16);

        read(sockfd, plaintext, sizeof(unsigned int) * 16);

        if (plaintext[0] == -1)
            return;

        /*printf("\nPlaintext recieved: ");
        for (i = 0; i < 16; i++)
            printf("%02X ", plaintext[i]);*/

        aesEncrypt(plaintext, ciphertext, key);

        write(sockfd, ciphertext, sizeof(unsigned int) * 16);
    }
}

int main()
{
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created...\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded...\n");

    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening...\n");
    len = sizeof(cli);

    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("Server accept the client...\n");

    //Sending t-table addresses to the client
    //write(connfd, taddress, sizeof(unsigned int *) * 4);

    //function for sending ciphertext to the client
    communicate(connfd);

    close(sockfd);

    printf("\nServer Exit...\n");

    return 0;
}
