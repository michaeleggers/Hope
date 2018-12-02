#ifndef GAME_H
#define GAME_H

#include "ref.h"
#include "scene.h"


/* local to game.cpp */
Background loadBackground(char * file);
void addEntity(Entity * entity);
Sprite * loadSprite(refexport_t* re,
                    char * textureID,
                    char * texturename,
                    int textureWidth, int textureHeight,
                    int xOffset, int yOffset,
                    int spriteWidth, int spriteHeight);

/* services from platform layer */
void update_messages();
bool fileExists(char const * file);

/* services to platform layer */
void game_init(refexport_t* re);
void game_update_and_render(float dt, refexport_t* re);

#endif GAME_H
