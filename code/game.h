#ifndef GAME_H
#define GAME_H


/* services from platform layer */
void update_messages();

/* services to platform layer */
void game_init();
void game_update_and_render(float dt);

#endif GAME_H
