#pragma once
#include "packet.h"
#include <stdint.h>

// Fonctions de conversion
uint64_t convert_mac(const char *s);
uint32_t convert_ip(const char *s);

void display_mac(uint64_t mac);
void display_ip(uint32_t ip);

void display_binary_ip(uint32_t ip);

void display_binary_mac(uint64_t mac);

// Fonctions d'affichage
void print_mac(uint64_t mac);
void print_ip(uint32_t ip);
const char *port_status_str(enum device_type s);