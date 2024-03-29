#pragma once

typedef struct color {
    float r,g,b,a;
} color;

#define COLOR_ZERO ((color){0.0f, 0.0f, 0.0f, 0.0f})
#define COLOR_BLACK ((color){0.0f, 0.0f, 0.0f, 1.0f})
#define COLOR_WHITE ((color){1.0f, 1.0f, 1.0f, 1.0f})
#define COLOR_BLUE ((color){0.0f, 0.0f, 1.0f, 1.0f})
#define COLOR_LIGHT_BLUE ((color){0.5f, 0.5f, 1.0f, 1.0f})
#define COLOR_GREY ((color){0.1f, 0.1f, 0.1f, 1.0f})
#define COLOR_LIGHT_GREY ((color){0.3f, 0.3f, 0.3f, 1.0f})
