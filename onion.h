#ifndef ONION_H
#define ONION_H

#include <stdio.h>

typedef struct {
    char name[64];
    int alive;
    int code;
    long took;
} onion_info;

onion_info check_onion(char *onion, int timeout, int port);
int is_onion(char *addr);
void check_batch(FILE *in, int timeout, int port, char *outfile, int quiet);
void print_result(onion_info *r, int quiet);
void save_to_csv(onion_info *r, char *filename);

#endif