#include <stdlib.h>

#include "assets.h"
#include "assets_state.h"
#include "thrust.h"
#include "things.h"

static assets_state_t asset_state;

static int all_allocated(void)
{
    return asset_state.bulletmap && asset_state.ship && asset_state.shieldship && asset_state.shipstorage &&
           asset_state.bulletstorage && asset_state.fragmentstorage && asset_state.fuelmap &&
           asset_state.fuelstorage && asset_state.loadmap && asset_state.loadstorage && asset_state.wirestorage;
}

int assets_allocate(void)
{
    asset_state.bulletmap = (uint8_t*)malloc(256);
    asset_state.ship = (uint8_t*)malloc(8192);
    asset_state.shieldship = (uint8_t*)malloc(8192);
    asset_state.shipstorage = (uint8_t*)malloc(256);
    asset_state.bulletstorage = (uint8_t*)malloc(maxbullets * 16);
    asset_state.fragmentstorage = (uint8_t*)malloc(maxfragments * 4);
    asset_state.fuelmap = (uint8_t*)malloc(256);
    asset_state.fuelstorage = (uint8_t*)malloc(256);
    asset_state.loadmap = (uint8_t*)malloc(64);
    asset_state.loadstorage = (uint8_t*)malloc(64);
    asset_state.wirestorage = (uint8_t*)malloc(64);

    if (!all_allocated())
    {
        assets_cleanup();
        return (0);
    }

    asset_state.blocks = bin_blks;

    return (1);
}

void assets_cleanup(void)
{
    free(asset_state.bulletmap);
    asset_state.bulletmap = NULL;
    free(asset_state.ship);
    asset_state.ship = NULL;
    free(asset_state.shieldship);
    asset_state.shieldship = NULL;
    free(asset_state.shipstorage);
    asset_state.shipstorage = NULL;
    free(asset_state.bulletstorage);
    asset_state.bulletstorage = NULL;
    free(asset_state.fragmentstorage);
    asset_state.fragmentstorage = NULL;
    free(asset_state.fuelmap);
    asset_state.fuelmap = NULL;
    free(asset_state.fuelstorage);
    asset_state.fuelstorage = NULL;
    free(asset_state.loadmap);
    asset_state.loadmap = NULL;
    free(asset_state.loadstorage);
    asset_state.loadstorage = NULL;
    free(asset_state.wirestorage);
    asset_state.wirestorage = NULL;
    asset_state.blocks = NULL;
}

uint8_t* assets_bulletmap(void)
{
    return asset_state.bulletmap;
}

uint8_t* assets_blocks(void)
{
    return asset_state.blocks;
}

uint8_t* assets_ship(void)
{
    return asset_state.ship;
}

uint8_t* assets_shieldship(void)
{
    return asset_state.shieldship;
}

uint8_t* assets_shipstorage(void)
{
    return asset_state.shipstorage;
}

uint8_t* assets_bulletstorage(void)
{
    return asset_state.bulletstorage;
}

uint8_t* assets_fragmentstorage(void)
{
    return asset_state.fragmentstorage;
}

uint8_t* assets_fuelmap(void)
{
    return asset_state.fuelmap;
}

uint8_t* assets_fuelstorage(void)
{
    return asset_state.fuelstorage;
}

uint8_t* assets_loadmap(void)
{
    return asset_state.loadmap;
}

uint8_t* assets_loadstorage(void)
{
    return asset_state.loadstorage;
}

uint8_t* assets_wirestorage(void)
{
    return asset_state.wirestorage;
}

assets_state_t* assets_state(void)
{
    return &asset_state;
}
