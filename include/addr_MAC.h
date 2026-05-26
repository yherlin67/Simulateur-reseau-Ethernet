#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

struct MACaddress {
    uint8_t octets[6];
};

void print_MAC(struct MACaddress mac);