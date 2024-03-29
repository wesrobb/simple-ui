#include "console.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eva/eva.h"

#include "color.h"
#include "profiler.h"
#include "rect.h"
#include "render.h"
#include "text.h"
#include "ustr.h"
#include "vec2.h"

#define MAX_LOG_ENTRIES 1024

typedef struct log_entries {
    text *entries[MAX_LOG_ENTRIES];
    int32_t start;
    int32_t count;
} log_entries;

typedef struct console_ctx {
    bool visible;
    recti rect;

    log_entries logs;
    double font_size;
} console_ctx;

typedef struct scrollbar {
    double content_size;
    double window_size;
    double track_size;

    double window_content_ratio;
    double grip_size;
    double min_grip_size;

    double window_scroll_area_size;
    double window_pos;
    double window_pos_ratio;
    double track_scroll_area_size;
    double grip_pos_on_track;

    double mouse_y; // Store the last mouse position.

    bool active;
} scrollbar;

static scrollbar sb;
static console_ctx _ctx;

void console_init()
{
    _ctx.font_size = 12.0f;
    sb.window_pos = 0.0f;

//    const char *fmt = "Did you put the cap back on the toothpaste? %d\n";
//    for (int32_t i = 0; i < 30; i++) {
//        char *buf = 0;
//        size_t len = (size_t)snprintf(buf, 0, fmt, i);
//        buf = malloc(len + 1);
//        snprintf(buf, len + 1, fmt, i); 
//        _ctx.logs.entries[i].text = buf;
//        _ctx.logs.entries[i].len = len - 1;
//    }
//    _ctx.logs.start = 0;
//    _ctx.logs.count = 30;
}

static void write_entry(text *t)
{
    if (_ctx.logs.count == MAX_LOG_ENTRIES) {
        text *current = _ctx.logs.entries[_ctx.logs.start];
        text_destroy(current);
        _ctx.logs.entries[_ctx.logs.start] = t;
        _ctx.logs.start = ++_ctx.logs.start % MAX_LOG_ENTRIES;
    }
    else {
        _ctx.logs.entries[_ctx.logs.count++] = t;
    }
}

void console_logn(char *str, size_t len)
{
    assert(str);
    assert(len > 0);

    ustr *s = ustr_create_utf8(str, len);
    assert(s);
    text *t = text_create_ustr(s);
    text_add_attr(t, 0, 0, FONT_FAMILY_COURIER_NEW, _ctx.font_size,
                  &COLOR_WHITE);
    assert(t);
    ustr_destroy(s);

    write_entry(t);
}

void console_log(const char *fmt, ...)
{
    assert(fmt);

    va_list args;
    va_start(args, fmt);
    size_t text_len = (size_t)vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    text_len += 1; // Make space for the null terminator

    char *data = malloc(text_len);
    va_list args2;
    va_start(args2, fmt);
    vsnprintf(data, text_len, fmt, args2);
    va_end(args2);

    ustr *s = ustr_create_utf8(data, text_len - 1);
    assert(s);
    text *t = text_create_ustr(s);
    text_add_attr(t, 0, 0, FONT_FAMILY_COURIER_NEW, _ctx.font_size,
                  &COLOR_WHITE);
    assert(t);
    ustr_destroy(s);

    write_entry(t);

    free(data);
}

void console_mouse_moved(const vec2 *mouse_pos)
{
    if (sb.active) {
        double mouse_delta = mouse_pos->y - sb.mouse_y;
        sb.mouse_y = mouse_pos->y;

        double new_grip_pos = sb.grip_pos_on_track + mouse_delta;
        if (new_grip_pos < 0) {
            new_grip_pos = 0;
        }
        if (new_grip_pos > sb.track_scroll_area_size) {
            new_grip_pos = sb.track_scroll_area_size;
        }

        if (sb.track_scroll_area_size > 0.0f) {
            double new_grip_pos_ratio = new_grip_pos / sb.track_scroll_area_size;
            sb.window_pos = new_grip_pos_ratio * sb.window_scroll_area_size;
            eva_request_frame();
        }
    }
}

void console_mouse_pressed(const vec2 *mouse_pos)
{
    if (_ctx.visible) {
        int32_t padding = 10;
        eva_framebuffer fb = eva_get_framebuffer();
        int32_t track_center = (int32_t)fb.w - padding;

        int32_t grip_width = 10;
        rect grip_rect = {
            .x = (track_center - (grip_width / 2.0f)),
            .y = sb.grip_pos_on_track,
            .w = grip_width,
            .h = sb.grip_size,
        };

        if (rect_point_intersect(&grip_rect, mouse_pos)) {
            sb.mouse_y = mouse_pos->y;
            sb.active = true;
        }
    }
}

void console_mouse_released(const vec2 *mouse_pos)
{
    (void)(mouse_pos);

    if (sb.active) {
        sb.active = false;
    }
}

void console_scroll(double delta_x, double delta_y)
{
    (void)delta_x;
    double new_grip_pos = sb.grip_pos_on_track - delta_y;
    if (new_grip_pos < 0) {
        new_grip_pos = 0;
    }
    if (new_grip_pos > sb.track_scroll_area_size) {
        new_grip_pos = sb.track_scroll_area_size;
    }

    if (sb.track_scroll_area_size > 0.0f) {
        double new_grip_pos_ratio = new_grip_pos / sb.track_scroll_area_size;
        sb.window_pos = new_grip_pos_ratio * sb.window_scroll_area_size;
        eva_request_frame();
    }
}

void console_keydown(int32_t key, uint32_t mods)
{
    eva_key k = (eva_key)key;
    eva_mod_flags f = (eva_mod_flags)mods;

    // Ctrl+` opens/closes the console
    if (k == EVA_KEY_GRAVE_ACCENT && f == EVA_MOD_CONTROL) {
        _ctx.visible = !_ctx.visible;
        eva_request_frame();
    }
}

void console_draw(const eva_framebuffer *fb)
{
    if (_ctx.visible) {
        profiler_begin;

        rect r = {
            .x = 0,
            .y = 0,
            .w = fb->w,
            .h = fb->h / 2.0f
        };
        render_draw_rect(&r, &COLOR_BLACK);

        double total_height = 0;

        int32_t start = _ctx.logs.start;
        int32_t end = _ctx.logs.start + _ctx.logs.count;
        for (int32_t i = start; i < end; i++) {
            text *entry = _ctx.logs.entries[i % MAX_LOG_ENTRIES];
            vec2 extents;
            text_extents(entry, &extents);
            total_height += extents.y;
        }

        double padding = 10.0f;

        sb.content_size = total_height + padding;
        sb.window_size = r.h;
        sb.track_size = r.h;
        sb.window_content_ratio = sb.window_size / sb.content_size;
        sb.grip_size = sb.track_size * sb.window_content_ratio;
        sb.min_grip_size = 20.0f;

        sb.grip_size = max(sb.min_grip_size, sb.grip_size);
        sb.grip_size = min(sb.track_size, sb.grip_size);
        
        sb.window_scroll_area_size = sb.content_size - sb.window_size;
        sb.window_pos_ratio = sb.window_pos / sb.window_scroll_area_size;
        sb.track_scroll_area_size = sb.track_size - sb.grip_size;
        sb.grip_pos_on_track = sb.track_scroll_area_size * sb.window_pos_ratio;

        rect window_rect = {
            .x = r.x,
            .y = sb.window_pos,
            .w = r.w,
            .h = r.h,
        };

        double track_center = fb->w - padding;
        double track_width = 5.0f;
        rect track_rect = {
            .x = track_center - (track_width / 2.0f),
            .y = r.y,
            .w = track_width,
            .h = sb.track_size
        };

        double grip_width = 10.0f;
        rect grip_rect = {
            .x = track_center - (grip_width / 2.0f),
            .y = sb.grip_pos_on_track,
            .w = grip_width,
            .h = sb.grip_size,
        };

        render_draw_rect(&track_rect, &COLOR_WHITE);
        render_draw_rect(&grip_rect, &COLOR_LIGHT_GREY);

        rect text_box = { padding, padding, 0, 0 };
        double cursor_y = padding;
        for (int32_t i = start; i < end; i++) {
            text *entry = _ctx.logs.entries[i % MAX_LOG_ENTRIES];

            vec2 extents;
            text_extents(entry, &extents);

            text_box.w = extents.x;
            text_box.h = extents.y;

            if (rect_overlap(&text_box, &window_rect)) {
                rect bbox = text_box;
                bbox.y -= sb.window_pos;

                rect clip = r;

                render_draw_text(entry, &bbox, &clip); 
                cursor_y += extents.y;
            }

            text_box.y += extents.y;
        }

        profiler_end;
    }
}
