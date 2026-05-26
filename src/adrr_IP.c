#include "addr_IP.h"

void print_IP(struct IPaddress ip)
{
    printf("%u.%u.%u.%u",ip.octets[0],ip.octets[1],ip.octets[2],ip.octets[3]);
}

