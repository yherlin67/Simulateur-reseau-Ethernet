#ifndef ADDR_IP_H
#define ADDR_IP_H
//#pragma once

//typedef uint8_t octet; ?

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct IPaddress {
    uint8_t octets[4];
}

void print_IP(struct IPaddress ip);