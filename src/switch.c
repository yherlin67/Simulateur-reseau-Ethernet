#include "switch.h"

enum type determine_type(struct eth_frame frame)
{
    uint16_t type = (frame.type[0] << 8) | frame.type[1];

    if (type <= 1500)
    {
        printf("IEEE 802.3 - longueur: %u octets\n", type);
        return IEEE802_3;
    }
    else
    {
        printf("Ethernet II - protocole: 0x%04x\n", type);
        return ETHERNET_II;
    }
}

void receive_frame(struct switch_t switch_t, struct eth_frame frame)
{
    enum type type = determine_type(frame);

    if(type == IEEE802_3)
    {
        //...
    }
    else if(type == ETHERNET_II)
    {
        //...
    }
}

void know_station(struct switch_t switch_t, struct eth_frame frame)
{
    bool know = false;
    for(int i = 0; i < 32; i++)
    {
        if(switch_t.tableCommutation[i].mac == frame.destination)
        {
            printf("Le switch connaît la station de destination, on peut envoyer la trame !");
            send_to(i, switch_t, frame.destination);
            know = true;
        }
    }
    if(!know)
    {
        printf("Le switch ne connaît pas la station de destination, envoyons la trame à tout le monde !");
        {
            send_to(-1, switch_t ,0x0000FFFFFFFFFFFF);
        }
    }
}

void send_to(int index, struct switch_t source, uint64_t destination)
{
    //je sais quoi faire à peu près
    //gérer si dest = broadcast
    //Boucle de i = 0 à nbPorts
    //si i = index c la source donc on renvoit pas
    //sinon ben on envoie à port 1, 2, 3 ...
}