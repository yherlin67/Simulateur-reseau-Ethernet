#pragma once

//typedef uint8_t octet; ?

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

struct IPaddress {
    uint8_t octets[4];
};

void print_IP(struct IPaddress ip);