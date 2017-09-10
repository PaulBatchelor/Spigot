#include <soundpipe.h>
#include <sporth.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "spigot.h"
#include "step16_assets.h"
#define STEP16_DATASIZE 128

typedef struct {
    spigot_color bgcolor;
    spigot_color fgcolor;
    int curpos;
    char data[STEP16_DATASIZE];
    SPFLOAT slice[8];
    int seqpos;
    int init;
    int play;
} spigot_step16;

static void step16_free(void *ud)
{
    spigot_step16 *step;
    step = ud;
    free(step);
}

static void gfx_init(spigot_graphics *gfx, void *ud)
{
}

static void init(void *ud)
{
    int d;
    spigot_step16 *s;

    s = ud;

    s->curpos = 0;
    for(d = 0; d < STEP16_DATASIZE; d++) s->data[d] = 0;

    s->seqpos = 0;
    s->init = 1;
    s->play = 1;
    for(d = 0; d < 8; d++) s->slice[d] = 0;
}

static void compute(void *ud, SPFLOAT in)
{
    spigot_step16 *s;
    int d;
    
    s = ud;
    if(in != 0) {
        for(d = 0; d < 8; d++) s->slice[d] *= in;
    } else {
        for(d = 0; d < 8; d++) s->slice[d] = 0;
    }
}

static void redraw(spigot_graphics *gfx, void *ud)
{
    /* fill background */
    spigot_step16 *s;
    int x;
    int y;
    int pos;
    char state;
    unsigned int off;

    s= ud;
    spigot_draw_fill(gfx, &s->bgcolor);
    /* draw scrollbar rect */

    off = 0;
    for(y = 0; y < 8; y++) {
        for(x = 0; x < 16; x++) {
            pos = y * 16 + x;
            state = s->data[pos];
            if(pos == s->curpos) {
                if(state) {
                    off = 3 * 8;
                } else {
                    off = 4 * 8;
                }
            } else {
                if(state) {
                    off = 16;
                } else {
                    off = 8;
                }
            }
            spigot_draw_glyph(gfx, 
                    &s->fgcolor, 
                    32 + 8*x, 32 + 16*y, 
                    8, 8, 
                    IMG_STEP16_ASSETS_WIDTH, 
                    step16_assets + off);
        }
    }

    spigot_draw_glyph(gfx, 
            &s->fgcolor, 
            32 + 8 * s->seqpos, 24, 
            8, 8, 
            IMG_STEP16_ASSETS_WIDTH, 
            step16_assets);
}

static void step(void *ud)
{
    spigot_step16 *s;
    int seqpos;
    int p;

    s = ud;
    if(s->play == 0) return;
    else if(s->play == -1) {
        s->play = 1;
        s->init = 1;
    }
    if(s->init) {
        s->seqpos = 0;
        s->init = 0;
    } else {
        s->seqpos = (s->seqpos + 1) % 16;
    }

    seqpos = s->seqpos;
    for(p = 0; p < 8; p++) {
        s->slice[p] = s->data[p * 16 + seqpos];
    }
}

static void toggle(void *ud)
{

    spigot_step16 *s;

    s = ud;
    if(s->play == 1) s->play = 0;
    else {
        s->play = -1;
    }

}

static void right(void *ud)
{
    spigot_step16 *s;

    s = ud;

    s->curpos++;
    if(s->curpos > (16 * 8) - 1) s->curpos = 0;
}

static void left(void *ud)
{
    spigot_step16 *s;
    s = ud;
    s->curpos--;
    if(s->curpos < 0) s->curpos = (16 * 8);
}

static void down(void *ud)
{
    spigot_step16 *s;
    s = ud;

    s->curpos += 16;

    s->curpos %= (16 * 8);
}

static void up(void *ud)
{
    spigot_step16 *s;
    s = ud;

    s->curpos -= 16;
    if(s->curpos < 0) {
        s->curpos += (16 * 8);
    }
}

static void keyhandler(spigot_graphics *gfx, void *ud, 
        int key, int scancode, int action, int mods)
{
    spigot_step16 *s;
    s = ud;

    switch(key) {
        case GLFW_KEY_F:
            if(s->data[s->curpos]) {
                s->data[s->curpos] = 0;
            } else {
                s->data[s->curpos] = 1;
            }
            spigot_gfx_step(gfx);
            break;
    }
}

static void step16_init(plumber_data *pd, runt_vm *vm, spigot_state *state)
{
    spigot_step16 *s;
    state->gfx_init = gfx_init;
    state->free = step16_free;
    state->init = init;
    state->ud = malloc(sizeof(spigot_step16));
    state->draw = redraw;
    state->step = step;
    state->toggle = toggle;
    state->right = right;
    state->left = left;
    state->up = up;
    state->down = down;
    state->key = keyhandler;
    state->compute = compute;

    s = state->ud;

    spigot_color_rgb(&s->bgcolor, 16, 16, 16);
    spigot_color_rgb(&s->fgcolor, 0, 130, 255);
}

static int rproc_step16(runt_vm *vm, runt_ptr p)
{
    runt_spigot_data *rsd;
    spigot_state *state;
    runt_int rc;
    runt_stacklet *s;
    plumber_data *pd;


    rsd = runt_to_cptr(p);
    pd = rsd->pd;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);

    step16_init(pd, vm, state);

    rc = runt_ppush(vm, &s);
    s->p = runt_mk_cptr(vm, state);
    return RUNT_OK;
}

static int rproc_trig(runt_vm *vm, runt_ptr p)
{
    runt_spigot_data *rsd;
    spigot_step16 *stp;
    runt_int rc;
    runt_stacklet *s;
    plumber_data *pd;
    const char *name;
    sp_ftbl *ft;

    rsd = runt_to_cptr(p);
    pd = rsd->pd;
    stp = rsd->state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    name = runt_to_string(s->p);

    sp_ftbl_bind(pd->sp, &ft, stp->slice, 16);
    plumber_ftmap_add(pd, name, ft);

    return RUNT_OK;
}

int spigot_step16_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "step16", 6, rproc_step16);
    spigot_word_define(vm, p, "step16_trig", 11, rproc_trig);
    return runt_is_alive(vm);
}
