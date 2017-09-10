#include <stdlib.h>
#include <soundpipe.h>
#include <sporth.h>

#include "spigot.h"

static void null_fun(void *ud)
{

}

static void null_drawfun(spigot_graphics *gfx, void *ud)
{

}

static void null_keyfun(spigot_graphics *gfx, void *ud, 
        int a, int b, int c, int d)
{

}

static void null_constant(void *ud, SPFLOAT val)
{

}

void spigot_state_init(spigot_state *state)
{
    state->up = null_fun;
    state->down = null_fun;
    state->left = null_fun;
    state->right = null_fun;
    state->toggle = null_fun;
    state->reset = null_fun;
    state->free = null_fun;
    state->step = null_fun;
    state->init = null_fun;
    state->compute = null_constant;
    state->key = null_keyfun;

    state->constant = null_constant;
    state->draw = null_drawfun;
    state->gfx_init= null_drawfun;
    state->type = SPIGOT_NONE;

}

int sporth_spigot_wrapper(plumber_data *pd, sporth_stack *stack, void **ud)
{
    SPFLOAT in;
    spigot_state *state;
    switch(pd->mode) {
        case PLUMBER_CREATE:
            if(sporth_check_args(stack, "f") != SPORTH_OK) {
                fprintf(stderr,"Not enough arguments for spigot wrapper\n");
                stack->error++;
                return PLUMBER_NOTOK;
            }
            sporth_stack_pop_float(stack);


            break;
        case PLUMBER_INIT:
            state = *ud;
            sporth_stack_pop_float(stack);
            state->init(state->ud);
            break;

        case PLUMBER_COMPUTE:
            state = *ud;
            in = sporth_stack_pop_float(stack);
            if(in != 0) {
                state->step(state->ud);
            }
            state->compute(state->ud, in);

            break;

        case PLUMBER_DESTROY:
            state = *ud;
            if(runt_is_alive(state->vm) == RUNT_OK) {
                state->free(state->ud);
            }
            break;
    }
    return PLUMBER_OK;
}

void spigot_add_ugen(plumber_data *pd, const char *name, spigot_state *state)
{
    plumber_ftmap_delete(pd, 0);
    plumber_ftmap_add_function(pd, "spigot", sporth_spigot_wrapper, state);
    plumber_ftmap_delete(pd, 1);
}
