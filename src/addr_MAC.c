#include "addr_MAC.h"

void print_MAC(struct MACaddress mac)
{
    //x pour héxadécimal, et au moins 2 caractères, complète avec des zéros sinon.
    printf("%02x,%02x,%02x,%02x,%02x,%02x,
            mac.octets[0],mac.octets[1],mac.octets[2]
            mac.octets[3],mac.octets[4],mac.octets[5]");
}