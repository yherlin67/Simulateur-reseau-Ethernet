#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "network.h"
#include "switch.h"
#include "station.h"

extern void receive_frame_st(struct station *st, struct eth_frame *frame);

void scheduler_init(struct scheduler *s){
    s->head = NULL;
    s->tail = NULL;
    s->size = 0;
}

void scheduler_push(struct scheduler *s, struct eth_frame *frame, enum device_type dst_type, union equipment_union dst, uint8_t in_port)
{
    struct node_frame *node = malloc(sizeof(struct node_frame));
    if (!node){
        printf("[SCHEDULER] Erreur malloc !\n"); return; 
    }

    node->frame = *frame; 
    node->dst_type = dst_type;
    node->dst = dst;      
    node->in_port = in_port;
    node->next = NULL; 

    if (s->tail == NULL) {
        s->head = node;
        s->tail = node;
    } else {
        s->tail->next = node; 
        s->tail = node; 
    }
    s->size++;
}

struct node_frame *scheduler_pop(struct scheduler *s)
{
    if (s->head == NULL){
        return NULL;
    }

    struct node_frame *node = s->head;
    s->head = s->head->next;
 
    if (s->head == NULL)
        s->tail = NULL;
 
    s->size--;
    return node;
}

int scheduler_tick(struct scheduler *s)
{
    int size_at_start = s->size; 
    int processed = 0;

    for (int i = 0; i < size_at_start; i++)
    {
        struct node_frame *node = scheduler_pop(s);
        if (!node){
            break;
        }

        if (node->dst_type == SWITCH)
        {
            receive_frame(node->dst.switch_t, &node->frame, node->in_port, s);
        }
        else
        {
            receive_frame_st(node->dst.station, &node->frame);
        }

        free(node);
        processed++;
    }
    return processed;
}

void scheduler_clear(struct scheduler *s)
{
    struct node_frame *node;
    while ((node = scheduler_pop(s)) != NULL){
        free(node);
    }
}