#include "station.h"

void receive_frame(struct station *st, struct eth_frame *frame, uint8_t num_port)
{
    // on fait un tableau de trame ou de messages pour garder les infos ? 
    //Sinon elle ignore (trame pas pour elle)
}

void station_send(struct station *src, struct station *dst, const char *message)
{
    
}