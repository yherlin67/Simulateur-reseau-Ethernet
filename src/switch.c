#include "switch.h"

void determine_type(struct eth_frame frame)
{
    uint16_t type = (frame.type[0] << 8) | frame.type[1];

    if (type <= 1500)
    {
        printf("IEEE 802.3 - longueur: %u octets\n", type);
    }
    else
    {
        printf("Ethernet II - protocole: 0x%04x\n", type);
    }
}