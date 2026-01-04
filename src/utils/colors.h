// ============================================================
// src/utils/colors.h
// ============================================================
#pragma once
#include <SDL2/SDL.h>

namespace Colors {
    inline SDL_Color Background()    { return {20, 20, 30, 255}; }
    inline SDL_Color Panel()         { return {26, 31, 46, 255}; }
    inline SDL_Color PanelDark()     { return {15, 20, 30, 255}; }
    inline SDL_Color Accent()        { return {78, 204, 163, 255}; }
    inline SDL_Color AccentHover()   { return {98, 224, 183, 255}; }
    inline SDL_Color Text()          { return {255, 255, 255, 255}; }
    inline SDL_Color TextDim()       { return {138, 155, 168, 255}; }
    inline SDL_Color Selected()      { return {15, 52, 96, 255}; }
    inline SDL_Color SelectedLight() { return {25, 72, 126, 255}; }
    inline SDL_Color Success()       { return {78, 204, 163, 255}; }
    inline SDL_Color Warning()       { return {255, 193, 7, 255}; }
    inline SDL_Color Error()         { return {244, 67, 54, 255}; }
    inline SDL_Color Border()        { return {45, 55, 72, 255}; }
}
