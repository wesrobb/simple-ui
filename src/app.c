#include "app.h"

#include <stdio.h>
#include <string.h>

#include <dtype.h>
#include "eva/eva.h"

#include "color.h"
#include "console.h"
#include "profiler.h"
#include "rect.h"
#include "render.h"
#include "text.h"
#include "textfield.h"
#include "ustr.h"
#include "vec2.h"

#define TEXT_MAX_LEN 1000
#define MAX_BRANCHES 64

typedef struct app_ctx {
    double branch_pane_resize_range;
    rect branch_pane_rect;
    bool branch_pane_resizing;
    double branch_pane_min_size;

    text *branches[MAX_BRANCHES];
    vec2 text_positions[MAX_BRANCHES];
    int32_t num_branches;

    char text[TEXT_MAX_LEN];
    size_t text_index;

    bool text_select;
    size_t start_index;
    size_t end_index;

    textfield *tf;
} app_ctx;

static app_ctx _ctx;

void app_init()
{
    eva_framebuffer fb = eva_get_framebuffer();

    _ctx.branch_pane_resize_range = (int32_t)(5 * fb.scale_x);
    _ctx.branch_pane_rect.w = 400;
    _ctx.branch_pane_rect.h = (int32_t)fb.h;
    _ctx.branch_pane_min_size = 200;

    const char *branches[] = {
        "master",
        "develop",
        "feature/AV",
        "pppppppppp",
        "ffffffffff",
        "Iñtërnâtiônàližætiøn",
        "Ἰοὺ ἰού· τὰ πάντʼ ἂν ἐξήκοι σαφῆ",
        "有子曰：「其為人也孝弟，而好犯上者，鮮矣",
    };

    _ctx.num_branches = array_size(branches);
    double font_size_pt = 14.0;
    for (int32_t i = 0; i < _ctx.num_branches; i++) {
        text *t = text_create_cstr(branches[i]);
        _ctx.branches[i] = t;
        text_add_attr(t, 0, 0, FONT_FAMILY_MENLO, font_size_pt, &COLOR_WHITE);
    }

    // Cache text position for hit testing on mouse move.
    vec2 padding = { 10, 10 };
    vec2 cursor = padding;
    for (int32_t i = 0; i < _ctx.num_branches; i++) {
        text *t = _ctx.branches[i];
        vec2 extents;
        text_extents(t, &extents);
        _ctx.text_positions[i] = cursor;
        cursor.y += extents.y;
    }

    _ctx.tf = textfield_create(500, 18.0, 10.0);
    uint16_t data[] = {'X'};
    textfield_input_text(_ctx.tf, data, array_size(data));
}

void app_shutdown()
{
}

void app_keydown(int32_t key, uint32_t mods)
{
    textfield_keydown(_ctx.tf, key, mods);

    if (key == EVA_KEY_ENTER && _ctx.text_index > 0) {
        console_logn(_ctx.text, _ctx.text_index);
        _ctx.text_index = 0;
        eva_request_frame();
    }

    console_keydown(key, mods);
}

void app_text_input(const uint16_t *text, uint32_t len)
{
    (void)text;
    (void)len;
    //if (_ctx.text_index + len < TEXT_MAX_LEN) {
    //    memcpy(_ctx.text + _ctx.text_index, text, len);
    //    _ctx.text_index += len;
    //    ustr str;
    //    str.data = (char*)text;
    //    str.len = _ctx.text_index;
    //    int32_t count = ustr_num_graphemes(&str);
    //    console_log("num graphemes %i", count);
    //    eva_request_frame();
    //}
    
    textfield_input_text(_ctx.tf, text, len);
    eva_request_frame();
}

void app_mouse_moved(const vec2 *mouse_pos)
{
    if (_ctx.text_select) {
        size_t str_index = 0;
        vec2 pos = _ctx.text_positions[0];
        vec2 mouse_pos_in_text_coords = vec2_sub(mouse_pos, &pos);
        bool hit = text_hit(_ctx.branches[0], &mouse_pos_in_text_coords,
                            &str_index);
        if (hit) {
            _ctx.end_index = str_index;
            //printf("Hits string index %zu\n", str_index);
            eva_request_frame();
        }
    }

    if (_ctx.branch_pane_resizing) {
        _ctx.branch_pane_rect.w = max(mouse_pos->x, _ctx.branch_pane_min_size);
        eva_request_frame();
    }

    rect resizeHandle = {
        .x = _ctx.branch_pane_rect.x + _ctx.branch_pane_rect.w -
             _ctx.branch_pane_resize_range,
        .y = _ctx.branch_pane_rect.y,
        .w = _ctx.branch_pane_resize_range * 2,
        .h = _ctx.branch_pane_rect.y + _ctx.branch_pane_rect.h,
    };
    (void)resizeHandle;

    console_mouse_moved(mouse_pos);
}

void app_mouse_pressed(const vec2 *mouse_pos)
{
    if (!_ctx.text_select) {
        size_t str_index = 0;
        vec2 pos = _ctx.text_positions[0];
        vec2 mouse_pos_in_text_coords = vec2_sub(mouse_pos, &pos);
        bool hit = text_hit(_ctx.branches[0], &mouse_pos_in_text_coords,
                            &str_index);
        if (hit) {
            _ctx.text_select = true;
            _ctx.start_index = str_index;
            _ctx.end_index = str_index;
            //printf("Started hit string index %zu\n", str_index);
            eva_request_frame();
        }
    }

    rect resizeHandle = {
        .x = _ctx.branch_pane_rect.x + _ctx.branch_pane_rect.w -
            _ctx.branch_pane_resize_range,
        .y = _ctx.branch_pane_rect.y,
        .w = _ctx.branch_pane_resize_range * 2,
        .h = _ctx.branch_pane_rect.y + _ctx.branch_pane_rect.h,
    };

    if (rect_point_intersect(&resizeHandle, mouse_pos)) {
        _ctx.branch_pane_resizing = true;
    }

    vec2 tf_pos = { 500, 500 };
    textfield_mouse_pressed(_ctx.tf, mouse_pos, &tf_pos);
    console_mouse_pressed(mouse_pos);
}

void app_mouse_released(const vec2 *mouse_pos)
{
    if (_ctx.text_select) {
        size_t str_index = 0;
        vec2 pos = _ctx.text_positions[0];
        vec2 mouse_pos_in_text_coords = vec2_sub(mouse_pos, &pos);
        bool hit = text_hit(_ctx.branches[0], &mouse_pos_in_text_coords,
                            &str_index);
        if (hit) {
            _ctx.text_select = false;
            _ctx.end_index = str_index;
            printf("Ended hit string index %zu\n", str_index);
            eva_request_frame();
        }
    }

    if (_ctx.branch_pane_resizing) {
        _ctx.branch_pane_resizing = false;
    }

    console_mouse_released(mouse_pos);
}

void app_scroll(double delta_x, double delta_y)
{
    console_scroll(delta_x, delta_y);
}

void app_window_resized(uint32_t width, uint32_t height)
{
    (void)width;
    (void)height;
    _ctx.branch_pane_rect.h = (int32_t)eva_get_window_height();
}

void test_dtype(const eva_framebuffer *fb)
{
    DT_DTENGINE engine;

    DT_MDC dc_mem;

    DT_STYLE_ATTRIBS style = {{0, 0}, {255, 0, 0, 0}, 0, DV_NULL};
    /*
        Same as
        style.ep[0] = 0;
        style.ep[1] = 0;
        style.rgbt[0] = 255;
        style.rgbt[1] = 0;
        style.rgbt[2] = 0;
        style.rgbt[3] = 0;
        style.reserved = 0;
        style.palette = DV_NULL;
    */ 

    DT_TYPE_ATTRIBS type = {0, 0, 0, 0, 0, {{100, 100, 0, 0, 0}}};
    /*
        Same as
        type.font_index = 0;
        type.thickness = 0;
        type.segment = 0;
        type.reserved = 0;
        type.descriptor = 0;
        type.transform.params.size_h = 100;
        type.transform.params.size_v = 100;
        type.transform.params.skew_h = 0;
        type.transform.params.skew_v = 0;
        type.transform.params.rotation = 0;
    */ 


    /* Initialize D-Type Font Engine. Exit if an error occurs. */ 
    DT_STREAM_FILE(sd_ini, "dtype.inf");
    if (dtEngineIniViaStream(&engine, &sd_ini, DV_NULL) == 0) exit(0);

    /* Add new Adobe Type 1 font to the Font Catalog */ 
    DT_STREAM_FILE(sd_font, "fonts/pfb/helvetica.pfb");
    type.font_index = dtFontAddViaStream(engine, DV_FONT_TYPE1_ADOBE, DV_NULL, 0, -1, 0, 1, &sd_font);

    /* Exit if an error occurs */ 
    if (type.font_index < 0) exit(0);


    /* Create a 640x480-pixel 24-bpp memory surface */ 
    dc_mem.w = 640;
    dc_mem.h = 480,
    dc_mem.l = 4 * dc_mem.w * dc_mem.h;
    if ((dc_mem.m = (DT_UBYTE*)malloc((uint64_t)dc_mem.l)) == DV_NULL) exit(0);
    memset(dc_mem.m, 255, dc_mem.l);

    /* Redirect all D-Type output to that surface and define clipping rect */ 
    dtOutputSetAsMDC(engine, DV_FORMAT_32, 0, &dc_mem, 0, 0, 640, 480);

    /* Select color */ 
    dtOutputSetStyleAttribs(engine, &style, 0);

    /* Select type */ 
    dtTypesetterSetTypeAttribs(engine, &type, 0);


    /* Draw Hello World at coordinates (200, 200) */ 
    //dtxTextDoOutput_ANSI(engine, 200, 200, 0, DV_TEXTMODE_KERN_ROUND_ADD, DV_NULL, "Hello World");

    /* Copy surface to screen or save as image */ 


    /* Free memory surface */ 
    free(dc_mem.m);

    /* And destroy D-Type Engine */ 
    dtEngineExt(engine);
}

void app_draw(const eva_framebuffer *fb)
{
    profiler_begin;

    vec2 padding = { 10, 10 };

    render_clear(&COLOR_LIGHT_GREY);
    render_draw_rect(&_ctx.branch_pane_rect, &COLOR_GREY);

    for (int32_t i = 0; i < _ctx.num_branches; i++) {
        text *t = _ctx.branches[i];
        vec2 *pos = &_ctx.text_positions[i];
        vec2 extents;
        text_extents(t, &extents);

        if (i == 0 && _ctx.text_select) {
            double offset_a = text_index_offset(t, _ctx.start_index);
            double offset_b = text_index_offset(t, _ctx.end_index);
            double start_offset = min(offset_a, offset_b);
            double end_offset = max(offset_a, offset_b);
            double width = end_offset - start_offset;
            rect highlight = {
                .x = pos->x + start_offset,
                .y = pos->y,
                .w = width,
                .h = extents.y,
            };
            render_draw_rect(&highlight, &COLOR_LIGHT_BLUE);
        }


        rect bbox = {
            .x = pos->x,
            .y = pos->y,
            .w = _ctx.branch_pane_rect.w - (padding.x * 2),
            .h = extents.y
        };
        rect clip = {
            .x = _ctx.branch_pane_rect.x,
            .y = _ctx.branch_pane_rect.y,
            .w = _ctx.branch_pane_rect.w - (padding.x * 2),
            .h = _ctx.branch_pane_rect.h
        };

        render_draw_text(t, &bbox, &clip);
    }

    vec2 tf_pos = { 1000, 500 };
    textfield_draw(_ctx.tf, &tf_pos);

    console_draw(fb);

    test_dtype(fb);

    profiler_end;
}
