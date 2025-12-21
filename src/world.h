#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

typedef struct
{
    double alpha;
    double deltaalpha;
    uint32_t loaded;
    uint32_t loadcontact;
    uint32_t loadpointshift;
    int loadpoint;
    int countdown;
    uint32_t crash;
    uint32_t shoot;
    uint32_t repetetive;
    uint32_t refueling;
    int speedx;
    int speedy;
    long absspeed;
    long oldabs;
    int kdir;
    int dir;
    int shipdx;
    int shipdy;
    int x;
    int y;
    int pixx;
    int pixy;
    int pblockx;
    int pblocky;
    int vx;
    int vy;
    int bildx;
    int bildy;
    int bblockx;
    int bblocky;
    int loadbx;
    int loadby;
    int colorr;
    int colorg;
    int colorb;
    double gamma_correction;
    uint32_t lenx;
    uint32_t leny;
    uint32_t lenx3;
    uint32_t leny3;
    int nodemo;
} world_state_t;

void world_init(void);
world_state_t* world_state(void);
void world_sync_blocks(void);
int world_is_loaded(void);
void world_set_loaded(int loaded);
int world_load_contact(void);
void world_set_load_contact(int contact);
void world_set_nodemo(int enabled);
int world_nodemo(void);

#endif /* WORLD_H */
