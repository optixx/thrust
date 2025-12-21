#include <string.h>

#include "graphics.h"
#include "world.h"

static world_state_t current_world;

void world_init(void)
{
    memset(&current_world, 0, sizeof(current_world));
    current_world.gamma_correction = 1.0;
}

world_state_t* world_state(void)
{
    return &current_world;
}

void world_sync_blocks(void)
{
    world_state_t* world = &current_world;
    world->pixx = world->x >> 3;
    world->pixy = world->y >> 3;
    world->bildx = (world->pixx + PBILDX - 4) % PBILDX + 4;
    world->bildy = world->pixy % PBILDY;
    world->pblockx = world->pixx >> 3;
    world->pblocky = world->pixy >> 3;
    world->bblockx = world->bildx >> 3;
    world->bblocky = world->bildy >> 3;
}

int world_is_loaded(void)
{
    return current_world.loaded;
}

void world_set_loaded(int loaded)
{
    current_world.loaded = !!loaded;
}

int world_load_contact(void)
{
    return current_world.loadcontact;
}

void world_set_load_contact(int contact)
{
    current_world.loadcontact = !!contact;
}

void world_set_nodemo(int enabled)
{
    current_world.nodemo = !!enabled;
}

int world_nodemo(void)
{
    return current_world.nodemo;
}
