#include "app.h"

#include <SDL.h>

#include "profiler.h"
#include "render.h"

typedef struct AppContext
{
    int32_t branchDrawerResizeRange;
    Rect branchDrawerRect;
    bool branchDrawerResizing;
    int32_t branchDrawerMinSize;

    SDL_Cursor *defaultCursor;
    SDL_Cursor *horizontalResizeCursor;
} AppContext;

AppContext g_appContext;

static bool PointInRect(int32_t x, int32_t y, Rect *r)
{
    return x >= r->x &&
           x <= (r->x + r->w) &&
           y >= r->y &&
           y <= (r->y + r->h);
}

void App_Init()
{
    int32_t frameWidth, frameHeight;
    float scaleFactorX, scaleFactorY;
    Render_GetDimensions(&frameWidth, &frameHeight, &scaleFactorX, &scaleFactorY);

    g_appContext.branchDrawerResizeRange = (int32_t)(5 * scaleFactorX);
    g_appContext.branchDrawerRect.w = 400;
    g_appContext.branchDrawerRect.h = frameHeight;
    g_appContext.branchDrawerMinSize = 200;

    g_appContext.defaultCursor = SDL_GetCursor(); // Store this switching back and forth between other cursors.
    g_appContext.horizontalResizeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
}

void App_Destroy()
{
    SDL_FreeCursor(g_appContext.horizontalResizeCursor);
}

void App_OnKeyPressed(SDL_Keysym *e)
{
    (void)e;
}

void App_OnKeyReleased(SDL_Keysym *e)
{
    (void)e;
}

void App_OnMouseMoved(SDL_MouseMotionEvent *e)
{
    if (g_appContext.branchDrawerResizing)
    {
        g_appContext.branchDrawerRect.w = max(e->x, g_appContext.branchDrawerMinSize);
    }

    Rect resizeHandle = {
        .x = g_appContext.branchDrawerRect.x + g_appContext.branchDrawerRect.w - g_appContext.branchDrawerResizeRange,
        .y = g_appContext.branchDrawerRect.y,
        .w = g_appContext.branchDrawerResizeRange * 2,
        .h = g_appContext.branchDrawerRect.y + g_appContext.branchDrawerRect.h,
    };

    if (PointInRect(e->x, e->y, &resizeHandle))
    {
        SDL_SetCursor(g_appContext.horizontalResizeCursor);
    }
    else
    {
        SDL_SetCursor(g_appContext.defaultCursor);
    }
}

void App_OnMousePressed(SDL_MouseButtonEvent *e)
{
    if (e->button & SDL_BUTTON_LEFT)
    {
        printf("Left mouse button pressed\n");
        Rect resizeHandle = {
            .x = g_appContext.branchDrawerRect.x + g_appContext.branchDrawerRect.w - g_appContext.branchDrawerResizeRange,
            .y = g_appContext.branchDrawerRect.y,
            .w = g_appContext.branchDrawerResizeRange * 2,
            .h = g_appContext.branchDrawerRect.y + g_appContext.branchDrawerRect.h,
        };

        if (PointInRect(e->x, e->y, &resizeHandle))
        {
            g_appContext.branchDrawerResizing = true;
        }
    }
}

void App_OnMouseReleased(SDL_MouseButtonEvent *e)
{
    if (e->button & SDL_BUTTON_LEFT)
    {
        printf("Left mouse button released\n");
        if (g_appContext.branchDrawerResizing)
        {
            g_appContext.branchDrawerResizing = false;
        }
    }
}

void App_OnWindowResized(int32_t width, int32_t height)
{
    (void)width;
    (void)height;
}

void App_Draw()
{
    Profiler_Begin;
    Color clearColor = {
        .r = 0.1f,
        .g = 0.1f,
        .b = 0.1f,
        .a = 1.0f
    };
    Color color = {
        .r = 0.3f,
        .g = 0.3f,
        .b = 0.3f,
        .a = 1.0f
    };

    Color white = {
        .r = 1.0f,
        .g = 1.0f,
        .b = 1.0f,
        .a = 1.0f,
    };

    Rect testRect = {
        .x = 500,
        .y = 500,
        .w = 100,
        .h = 100
    };

    Render_Clear(clearColor);
    Render_DrawRect(g_appContext.branchDrawerRect, color);
    Render_DrawHollowRect(testRect, white, 4);

    int32_t fontSizePt = 18;
    int32_t ascent, descent;
    Render_GetFontHeight(FONT_ROBOTO_REGULAR, fontSizePt, &ascent, &descent);

    const char* textLines[] = {
        "master",
        "develop",
        "feature/AV",
        "pppppppppp",
        "ffffffffff"
    };

    for (int32_t i = 0; (unsigned long)i < sizeof(textLines) / sizeof(textLines[0]); i++)
    {
        int32_t width = Render_GetTextWidth(FONT_ROBOTO_REGULAR, textLines[i], fontSizePt);
        int32_t x = 10;
        int32_t y = 20 + (i * (ascent - descent));
        Rect r = {
            .x = x,
            .y = y - ascent,
            .w = width,
            .h = ascent - descent
        };

        Render_DrawHollowRect(r, white, 2);
        Render_DrawFont(FONT_ROBOTO_REGULAR, textLines[i], x, y, fontSizePt, white);
    }

    Profiler_End;
}
