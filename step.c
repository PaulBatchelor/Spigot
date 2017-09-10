#include <soundpipe.h>
#include <sporth.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <unistd.h>
#include "plumbstream.h"

#include "spigot.h"
#include "step16_assets.h"
#define STEP16_DATASIZE 128

typedef struct {
    runt_vm *vm;
    spigot_color bgcolor;
    spigot_color fgcolor;
    int curpos;
    char data[STEP16_DATASIZE];
    SPFLOAT slice[8];
    int seqpos;
    int init;
    int play;
    const char *filename;
    int loaded;
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

static void load_file(spigot_step16 *s)
{
    if(!s->loaded) {
        runt_print(s->vm, "No filename loaded!\n");
        return;
    }
    if(access(s->filename, F_OK) != -1) {
        runt_print(s->vm, "Loading file %s\n", s->filename);
        runt_parse_file(s->vm, s->filename);
    }
}

static void save_file(spigot_step16 *s)
{
    FILE *fp;
    int x, y;
    int off;
    int pos;

    if(!s->loaded) {
        runt_print(s->vm, "No filename loaded!\n");
        return;
    }

    fp = fopen(s->filename, "w");

    if(fp == NULL) {
        runt_print(s->vm, "There was a problem writing to %s.\n", s->filename);
        return;
    }
    
    runt_print(s->vm, "File saved to %s.\n", s->filename);
    
    for(y = 0; y < 8; y++) {
        off = 16 * y;
        fprintf(fp, "%d step16_row\n\n", y);
        for(x = 0; x < 16; x++) {
           pos = off + x;
           if(s->data[pos]) {
                fprintf(fp, "%d step16_on\n", x);
           }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
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
        case GLFW_KEY_S:
            save_file(s);
            break;
    }
}

static void step16_init(plumber_data *pd, runt_vm *vm, spigot_state *state)
{
    spigot_step16 *s;
    int d;

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

    for(d = 0; d < STEP16_DATASIZE; d++) s->data[d] = 0;
    s->vm = vm;
    s->loaded = 0;

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

static int rproc_strig(runt_vm *vm, runt_ptr p)
{
    runt_spigot_data *rsd;
    spigot_step16 *stp;
    runt_int rc;
    runt_stacklet *s;
    plumber_data *pd;
    const char *name;
    sp_ftbl *ft;
    plumber_stream *stream;

    rsd = runt_to_cptr(p);
    pd = rsd->pd;
    stp = rsd->state->ud;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    stream = runt_to_cptr(s->p);

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    name = runt_to_string(s->p);

    sp_ftbl_bind(pd->sp, &ft, stp->slice, 16);
    plumber_stream_append_data(pd, stream, name, strlen(name), ft, 1, PTYPE_TABLE);

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

static int rproc_row(runt_vm *vm, runt_ptr p)
{
    int row;
    int rc;
    runt_spigot_data *rsd;
    runt_stacklet *s;

    spigot_step16 *stp;

    rsd = runt_to_cptr(p);
    stp = rsd->state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    row = s->f;

    stp->curpos = row * 16;
    return RUNT_OK;
}


static int rproc_on(runt_vm *vm, runt_ptr p)
{
    int off;
    int rc;
    runt_spigot_data *rsd;
    runt_stacklet *s;

    spigot_step16 *stp;

    rsd = runt_to_cptr(p);
    stp = rsd->state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    off = s->f;

    stp->data[stp->curpos + off] = 1;
    return RUNT_OK;
}

static int rproc_load(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_step16 *stp;
    const char *str;
    
    rsd = runt_to_cptr(p);
    stp = rsd->state->ud;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    str = runt_to_string(s->p);
    /* save string onto stack... careful */
    runt_mark_set(vm);
    runt_cell_undo(vm);
    
    stp->loaded = 1;
    stp->filename = str;
    
    load_file(stp);

    return RUNT_OK;
}

int spigot_step16_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "step16", 6, rproc_step16);
    spigot_word_define(vm, p, "step16_trig", 11, rproc_trig);
    spigot_word_define(vm, p, "step16_strig", 12, rproc_strig);
    spigot_word_define(vm, p, "step16_row", 10, rproc_row);
    spigot_word_define(vm, p, "step16_on", 9, rproc_on);
    spigot_word_define(vm, p, "step16_open", 11, rproc_load);
    return runt_is_alive(vm);
}
