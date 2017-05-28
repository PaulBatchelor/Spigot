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

static void init_tracker_gfx(spigot_graphics *gfx, void *ud)
{
   spigot_tracker *t;

   t = ud;

   spigot_color_rgb(&t->background, 130, 195, 255);
   spigot_color_rgb(&t->foreground, 0, 65, 130);
   spigot_color_rgb(&t->shade, 52, 158, 255);
   spigot_color_rgb(&t->row_selected, 255, 119, 171);
   spigot_color_rgb(&t->text_selected, 255, 255, 255);

   spigot_draw_fill(gfx, &t->background);
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
