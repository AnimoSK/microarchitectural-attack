#ifndef ENCRYPT_H
#define ENCRYPT_H

void aesEncrypt(unsigned int *, unsigned int *, unsigned int *);

void tableAddress(unsigned int **);

void convertKey(unsigned int *, int);

void addKey(int **, int **);

void substitution(int *);

void shift(int *, int);

void mixColumns(int **);

void keySchedule(int **, int);

void printMatrix(int **);

#endif