#ifndef GAME_H
#define GAME_H

#include "ref.h"
#include "scene.h"

/* local to game.cpp */
Background loadBackground(char const * file);
void drawRoom(refexport_t* re, Room* room);

/* services from platform layer */
void update_messages();


/* services to platform layer */
void game_init();
void game_update_and_render(float dt, refexport_t* re);

#endif GAME_H
