#ifndef GAME_H
#define GAME_H


/* services from platform layer */
struct Rect;
Rect get_window_dimensions();
void update_messages();

/* services to platform layer */
void game_init();
void game_render();

#endif GAME_H
