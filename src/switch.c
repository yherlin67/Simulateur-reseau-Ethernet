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

void know_station(struct switch_t sw, struct eth_frame frame, struct network net)
{
    //IMPORTANT : i-ème port su switch = i-ème station du réseau !
    bool know = false;
    for(int i = 0; i < 32; i++)
    {
        //memcmp ( const void * pointer1, const void * pointer2, size_t size )
        //Compare le contenu de deux blocs mémoires octets par octets sur une taille de size
        //Si ca renvoie autre chose que 0 : les octets diffèrent
        if(memcmp(&sw.tableCommutation[i]->mac, &frame.destination, 6) == 0)
        {
            //On récup le numéro de port = numéro de la station
            uint8_t num_port = sw.tableCommutation[i]->port; 
            send_to(num_port, frame, net);  // envoie sur ce port
            know = true;
        }
    }
    if(!know)
    {
        printf("Le switch ne connaît pas la station de destination, envoyons la trame à tout le monde !");
        {
            //Broadcast
            send_to(-1, frame, net);
        }
    }
}

void send_to(int8_t num_port, struct eth_frame frame, struct network net)
{
    if(num_port == -1)
    {
        // Broadcast, envoie à toutes les stations
        for(int i = 0; i < net.nbStations; i++)
        {
            receive_frame_st(net.stations[i], frame);
        }
    }
    else
    {
        // Envoie uniquement à la station connectée sur ce port
        receive_frame_st(net.stations[num_port], frame);
    }
}