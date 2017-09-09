#include <soundpipe.h>
#include <sporth.h>
#include <stdlib.h>

#include "spigot.h"

typedef struct {
    int foo;
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

}

static void redraw(spigot_graphics *gfx, void *ud)
{

}

static void step(void *ud)
{

}

static void toggle(void *ud)
{

}

static void right(void *ud)
{

}

static void left(void *ud)
{

}

static void down(void *ud)
{

}

static void up(void *ud)
{

}

static void step16_init(plumber_data *pd, runt_vm *vm, spigot_state *state)
{
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

int spigot_step16_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "step16", 6, rproc_step16);
    return runt_is_alive(vm);
}
