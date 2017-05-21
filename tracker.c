#include <soundpipe.h>
#include <sporth.h>

#include "spigot.h"
#include "mockup.h"

static void init_tracker_gfx(spigot_graphics *gfx, void *ud)
{
   unsigned char *buf;
   unsigned int pos;
   unsigned int x, y;

   buf = spigot_graphics_get_buf(gfx);

   y = 0;
   x = 0;
   for(y = 0; y < 193; y++) {
       for(x = 0; x < 193; x++) {
            pos = y * 193 * 3 + x * 3;
            buf[pos] = mockup[pos];
            buf[pos + 1] = mockup[pos + 1];
            buf[pos + 2] = mockup[pos + 2];
       }
   }
}

void spigot_tracker_state(plumber_data *pd, spigot_state *state)
{
    state->gfx_init = init_tracker_gfx;
}
