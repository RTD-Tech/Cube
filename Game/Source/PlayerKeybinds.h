#pragma once
#include "SDL3/SDL_scancode.h"

// player keybinds
inline SDL_Scancode move_left_bind = SDL_SCANCODE_A;
inline SDL_Scancode move_right_bind = SDL_SCANCODE_D;
inline SDL_Scancode look_up_bind = SDL_SCANCODE_W;
inline SDL_Scancode look_down_bind = SDL_SCANCODE_S;
inline SDL_Scancode heal_bind = SDL_SCANCODE_E;
inline SDL_Scancode jump_bind = SDL_SCANCODE_SPACE;
inline SDL_Scancode dash_bind = SDL_SCANCODE_LSHIFT;
inline SDL_Scancode pause_bind = SDL_SCANCODE_ESCAPE;

// debug keybinds
inline SDL_Scancode debug_collision = SDL_SCANCODE_T;
inline SDL_Scancode debug_save = SDL_SCANCODE_P;
inline SDL_Scancode debug_hot_reload = SDL_SCANCODE_F5;
inline SDL_Scancode debug_mouse_cursor = SDL_SCANCODE_Y;
inline SDL_Scancode debug_restart_game = SDL_SCANCODE_N;
inline bool mouse_visible = true;