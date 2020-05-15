#pragma once

typedef struct Rect
{
    int x, y, w, h;
} Rect;

typedef struct Color
{
    unsigned char b, g, r, a;
} Color;
typedef Color Pixel;

typedef struct SDL_Surface SDL_Surface;
void Render_Init();
void Render_DrawRect(SDL_Surface *, Rect, Color);
void Render_DrawFont(SDL_Surface *, float dpiX, float dpiY);