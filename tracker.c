#include <soundpipe.h>
#include <sporth.h>
#include <stdlib.h>

#include "spigot.h"
#include "tracker_assets.h"

typedef struct {
    spigot_color background;
    spigot_color foreground;
    spigot_color shade;
    spigot_color row_selected;
    spigot_color text_selected;
    spigot_color column_selected;
} spigot_tracker;


static void draw_square_open(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{
    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            4, 4, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 10 * 8);
}

static void draw_square_closed(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{
    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            4, 4, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 11 * 8);
}

static void draw_arrow_left(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{
    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH);
}

static void draw_arrow_right(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{

    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 16);
}

static void draw_arrow_up(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{

    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 12 * 8);
}

static void draw_arrow_down(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{

    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 14 * 8);
}

static void draw_number(spigot_graphics *gfx, spigot_color *clr,
        int x, int y, int n) {

        spigot_draw_glyph(gfx, clr, 
                x, y, 
                4, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + 4 * (n / 10));

        spigot_draw_glyph(gfx, clr, 
                x + 4, y, 
                4, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + 4 * (n % 10));
}

static void draw_note(spigot_graphics *gfx, spigot_color *clr, 
        int chan, int pos,
        int note,
        int op,
        int oct) {

        int x, y;

        int note_off;
        int op_off;
        int oct_off;

        x = 25 + 32 * chan;
        y = 18 + 8 * pos;

        note_off = 64 + 4 * note;
        op_off = 48 + op * 5;
        oct_off = oct * 4;

        spigot_draw_glyph(gfx, clr, 
                x, y, 
                4, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + note_off);
        
        spigot_draw_glyph(gfx, clr, 
                x + 4, y, 
                5, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + op_off);
        
        spigot_draw_glyph(gfx, clr, 
                x + 10, y, 
                4, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + oct_off);
}

static void init_tracker_gfx(spigot_graphics *gfx, void *ud)
{
    int i;
    spigot_tracker *t;

    t = ud;

    spigot_color_rgb(&t->background, 130, 195, 255);
    spigot_color_rgb(&t->foreground, 0, 65, 130);
    spigot_color_rgb(&t->shade, 52, 158, 255);
    spigot_color_rgb(&t->row_selected, 255, 119, 171);
    spigot_color_rgb(&t->text_selected, 255, 255, 255);

    spigot_draw_fill(gfx, &t->background);

    /* draw dark row lines */

    for(i = 0; i < 10; i++) {
        spigot_draw_rect(gfx, &t->shade, 16, 16 + 16 * i, 160, 8);
    }
      
    /* draw row selected (temporary: for demo purposes) */
    spigot_draw_rect(gfx, &t->row_selected, 16, 24, 160, 8);

    /* draw border */
    spigot_draw_hline(gfx, &t->foreground, 0, 0, 193);
    spigot_draw_hline(gfx, &t->foreground, 192, 0, 193);
    spigot_draw_vline(gfx, &t->foreground, 0, 0, 193);
    spigot_draw_vline(gfx, &t->foreground, 192, 0, 193);

    /* draw header divider */
    spigot_draw_hline(gfx, &t->foreground, 16, 0, 193);

    /* draw status bar space divider */

    spigot_draw_hline(gfx, &t->foreground, 168, 0, 193);
    spigot_draw_hline(gfx, &t->foreground, 176, 0, 193);

    /* draw channel dividers */
    spigot_draw_vline(gfx, &t->foreground, 16, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 48, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 80, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 112, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 144, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 176, 16, 152);

    /* draw pattern number dividers */

    /* draw channel numbers */

    for(i = 0; i < 18; i++) {
        spigot_draw_hline(gfx, &t->foreground, 24 + 8 * i, 0, 16);
    }

    /* 0 0 1 */
    draw_square_open(gfx, &t->foreground, 24, 10);
    draw_square_open(gfx, &t->foreground, 30, 10);
    draw_square_closed(gfx, &t->foreground, 36, 10);
    
    /* 0 1 0 */
    draw_square_open(gfx, &t->foreground, 56, 10);
    draw_square_closed(gfx, &t->foreground, 62, 10);
    draw_square_open(gfx, &t->foreground, 68, 10);
    
    /* 0 1 1 */
    draw_square_open(gfx, &t->foreground, 88, 10);
    draw_square_closed(gfx, &t->foreground, 94, 10);
    draw_square_closed(gfx, &t->foreground, 100, 10);
    
    /* 1 0 0 */
    draw_square_closed(gfx, &t->foreground, 120, 10);
    draw_square_open(gfx, &t->foreground, 126, 10);
    draw_square_open(gfx, &t->foreground, 132, 10);
    
    /* 1 0 1 */
    draw_square_closed(gfx, &t->foreground, 152, 10);
    draw_square_open(gfx, &t->foreground, 158, 10);
    draw_square_closed(gfx, &t->foreground, 164, 10);

    /* draw pattern sequencer dividers */
    for(i = 0; i < 11; i++) {
        spigot_draw_vline(gfx, &t->foreground, 16 + 16 * i, 22 * 8, 16);
    }

    /* draw scroll bar dividers */
    spigot_draw_hline(gfx, &t->foreground, 32, 22 * 8, 16);
    spigot_draw_hline(gfx, &t->foreground, 19 * 8, 22 * 8, 16);

    /* draw scollbar arrows */
    draw_arrow_left(gfx, &t->foreground, 16, 22 * 8);
    draw_arrow_right(gfx, &t->foreground, 20 * 8, 22 * 8);
    
    draw_arrow_up(gfx, &t->foreground, 22 * 8, 2 * 8);
    draw_arrow_down(gfx, &t->foreground, 22 * 8, 19 * 8);

    /* draw row numbers */
    for(i = 0; i < 19; i++) {
        draw_number(gfx, &t->foreground, 5, 18 + 8 * i, i); 
    }

    /* draw note */
    draw_note(gfx, &t->foreground, 1, 4, 0, 0, 0);

}

static void spigot_tracker_free(void *ud)
{
    spigot_tracker *t;
    t = ud;
    free(t);
}

void spigot_tracker_state(plumber_data *pd, spigot_state *state)
{
    state->gfx_init = init_tracker_gfx;
    state->free = spigot_tracker_free;
    state->ud = malloc(sizeof(spigot_tracker));
}
