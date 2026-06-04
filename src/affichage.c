#include <stdint.h>

// Affichage d'une adresse ip
void print_ip(uint32_t ip)
{
    uint8_t octet_mask = 000011111111; //plutôt 0xFF = 255 ? ou juste 255 ?
    printf("%u.%u.%u.%u",(ip >> 24) & octet_mask,(ip >> 16) & octet_mask,(ip >>  8) & octet_mask,(ip >>  0) & octet_mask);
    // décale les 8 bits de poids fort en position basse → premier octet
}

// Affichage d'une adresse mac
void print_mac(uint64_t mac){
    uint8_t octet_mask = 000011111111; //plutôt 0xFF = 255 ? ou juste 255 ?
    printf("%02X:%02X:%02X:%02X:%02X:%02X",(mac >> 40) & octet_mask,(mac >> 32) & octet_mask,(mac >> 24) & octet_mask,(mac >> 16) & octet_mask,(mac >>  8) & octet_mask,(mac >>  0) & octet_mask);
}