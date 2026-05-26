#ifndef ADDR_MAC_H
#define ADDR_MAC_H
//#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct MACaddress {
    uint8_t octets[6];
}

void print_MAC(struct MACaddress mac);