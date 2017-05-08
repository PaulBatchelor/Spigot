#include <stdlib.h>
#include <soundpipe.h>
#include <sporth.h>

#include "spigot.h"

typedef struct {
    spigot_graphics *gfx;
    spigot_pbrain *pbrain;
    int load;
    spigot_state state;
} spigot_stuff;

static void null_fun(void *ud)
{

}

static void null_drawfun(spigot_graphics *gfx, void *ud)
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

    state->draw = null_drawfun;
    state->init= null_drawfun;
}

static int sporth_spigot(plumber_data *pd, sporth_stack *stack, void **ud)
{
    const char *filename;
    SPFLOAT in;
    int tick;
    SPFLOAT val;
    spigot_stuff *stuff;
    switch(pd->mode) {
        case PLUMBER_CREATE:
            if(sporth_check_args(stack, "fs") != SPORTH_OK) {
                fprintf(stderr,"Not enough arguments for spigot\n");
                stack->error++;
                return PLUMBER_NOTOK;
            }
            stuff = malloc(sizeof(spigot_stuff));
            stuff->gfx = spigot_gfx_new();
            stuff->pbrain = spigot_pbrain_new();
            spigot_state_init(&stuff->state);
            spigot_pbrain_state(pd, stuff->pbrain, &stuff->state);
            spigot_gfx_set_state(stuff->gfx, &stuff->state);
            *ud = stuff;

            sporth_stack_pop_string(stack);
            sporth_stack_pop_float(stack);
            sporth_stack_pop_float(stack);
            sporth_stack_pop_float(stack);
            sporth_stack_push_float(stack, 0.0); 
            break;
        case PLUMBER_INIT:
            stuff = *ud;
            filename = sporth_stack_pop_string(stack);
            stuff->load = sporth_stack_pop_float(stack);
            sporth_stack_pop_float(stack);
            sporth_stack_pop_float(stack);
            spigot_init(stuff->pbrain, filename);
            if(stuff->load != 0) {
                spigot_gfx_init(stuff->gfx);
                spigot_start(stuff->gfx);
            }
            sporth_stack_push_float(stack, 0.0);
            break;

        case PLUMBER_COMPUTE:
            stuff = *ud;
            sporth_stack_pop_float(stack);
            in = sporth_stack_pop_float(stack);
            val = sporth_stack_pop_float(stack);
            spigot_constant(stuff->pbrain, val);
            tick = 0;
            if(in != 0) {
                if(stuff->load != 0) {
                    spigot_gfx_step(stuff->gfx);
                }
                tick = spigot_step(stuff->pbrain);
            }

            if(tick > 0) {
                sporth_stack_push_float(stack, tick);
            } else {
                sporth_stack_push_float(stack, 0.0);
            }

            break;

        case PLUMBER_DESTROY:
            stuff = *ud;
            if(stuff->load != 0) {
                spigot_stop(stuff->gfx);
            }
            spigot_gfx_free(stuff->gfx);
            stuff->state.free(stuff->state.ud);
            free(stuff);
            break;
    }
    return PLUMBER_OK;
}

plumber_dyn_func sporth_return_ugen()
{
    return sporth_spigot;
}
