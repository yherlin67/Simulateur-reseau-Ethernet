#include <stdint.h>
#include <stdio.h>
#include "network.h"

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
    // attention aussi l'affichage pour l'utilisateur 
}


uint32_t convert_ip(const char *ipStr) {
    unsigned int octet1, octet2, octet3, octet4;
    
    if (sscanf(ipStr, "%u.%u.%u.%u", &octet1, &octet2, &octet3, &octet4) != 4) {
        return 0;
    }

    uint32_t ip_bin = ((uint32_t)octet1 << 24) | ((uint32_t)octet2 << 16) | ((uint32_t)octet3 << 8) | octet4;
    
    return ip_bin;
}

uint64_t convert_mac(const char *macStr) {
    unsigned int m1, m2, m3, m4, m5, m6;
    
    if (sscanf(macStr, "%x:%x:%x:%x:%x:%x", &m1, &m2, &m3, &m4, &m5, &m6) != 6) {
        return 0;
    }

    uint64_t mac_bin = ((uint64_t)m1 << 40) | 
                       ((uint64_t)m2 << 32) | 
                       ((uint64_t)m3 << 24) | 
                       ((uint64_t)m4 << 16) | 
                       ((uint64_t)m5 << 8)  | 
                       m6;
    
    return mac_bin;
}

void display_binary_ip(uint32_t ip) {
    for (int i = 0; i < 32; i++) {
        uint32_t bit = (ip >> (31 - i)) & 1;
        printf("%u", bit);
    }
}

void display_binary_mac(uint64_t mac) {
    for (int i = 0; i < 48; i++) {
        uint64_t bit = (mac >> (47 - i)) & 1;
        printf("%u", (unsigned int)bit);
    }
}

// retourne le nom d'un état de port stp
const char *port_status_str(enum device_type s)
{
    switch(s) {
        case ROOT: return "ROOT";
        case DESIGNED: return "DESIGNATED";
        case BLOCKED: return "BLOCKED";
        default: return "DEFAULT";
    }
}

// Affiche la MAC au format XX:XX:XX:XX:XX:XX à partir d'un entier 64 bits
void display_mac(uint64_t mac) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X", 
           (unsigned int)((mac >> 40) & 0xFF),
           (unsigned int)((mac >> 32) & 0xFF),
           (unsigned int)((mac >> 24) & 0xFF),
           (unsigned int)((mac >> 16) & 0xFF),
           (unsigned int)((mac >> 8)  & 0xFF),
           (unsigned int)(mac & 0xFF));
}

// Affiche l'IP au format XXX.XXX.XXX.XXX à partir d'un entier 32 bits
void display_ip(uint32_t ip) {
    printf("%u.%u.%u.%u", 
           (unsigned int)((ip >> 24) & 0xFF),
           (unsigned int)((ip >> 16) & 0xFF),
           (unsigned int)((ip >> 8)  & 0xFF),
           (unsigned int)(ip & 0xFF));
}