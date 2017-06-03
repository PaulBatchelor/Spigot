#include <stdlib.h>
#include <soundpipe.h>
#include <sporth.h>

#include "spigot.h"

typedef struct {
    spigot_graphics *gfx;
    int load;
    spigot_state *state;
    runt_vm vm;
    runt_cell *cells;
    unsigned char *mem;
} spigot_stuff;

static void null_fun(void *ud)
{

}

static void null_drawfun(spigot_graphics *gfx, void *ud)
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
    state->constant = null_constant;

    state->draw = null_drawfun;
    state->gfx_init= null_drawfun;
}

static int sporth_spigot(plumber_data *pd, sporth_stack *stack, void **ud)
{
    const char *filename;
    SPFLOAT in;
    spigot_stuff *stuff;
    int rc;
    switch(pd->mode) {
        case PLUMBER_CREATE:
            if(sporth_check_args(stack, "ffs") != SPORTH_OK) {
                fprintf(stderr,"Not enough arguments for spigot\n");
                stack->error++;
                return PLUMBER_NOTOK;
            }
            filename = sporth_stack_pop_string(stack);
            sporth_stack_pop_float(stack);
            sporth_stack_pop_float(stack);

            stuff = malloc(sizeof(spigot_stuff));
            stuff->cells = malloc(1024 * sizeof(runt_cell));
            stuff->mem = malloc(5 * RUNT_MEGABYTE * sizeof(unsigned char));

            runt_init(&stuff->vm);
            runt_cell_pool_set(&stuff->vm, stuff->cells, 512);
            runt_cell_pool_init(&stuff->vm);
            runt_memory_pool_set(&stuff->vm, stuff->mem, 5 * RUNT_MEGABYTE);

            rc = spigot_load(pd, &stuff->vm, &stuff->state, filename);

            stuff->gfx = spigot_gfx_new();

            spigot_gfx_set_state(stuff->gfx, stuff->state);

            *ud = stuff;

            if(rc != PLUMBER_OK) {
                plumber_print(pd, "Spigot: error reading file %s\n", filename);
                stack->error++;
                return PLUMBER_NOTOK;
            }


            break;
        case PLUMBER_INIT:
            stuff = *ud;
            sporth_stack_pop_string(stack);
            stuff->load = sporth_stack_pop_float(stack);
            sporth_stack_pop_float(stack);
            stuff->state->init(stuff->state->ud);

            if(stuff->load != 0) {
                spigot_gfx_init(stuff->gfx);
                spigot_start(stuff->gfx);
            }

            break;

        case PLUMBER_COMPUTE:
            stuff = *ud;
            sporth_stack_pop_float(stack);
            in = sporth_stack_pop_float(stack);
            if(in != 0) {
                if(stuff->load != 0) {
                    spigot_gfx_step(stuff->gfx);
                }
                stuff->state->step(stuff->state->ud);
            }

            break;

        case PLUMBER_DESTROY:
            stuff = *ud;
            if(stuff->load != 0) {
                spigot_stop(stuff->gfx);
            }
            spigot_gfx_free(stuff->gfx);
            if(runt_is_alive(&stuff->vm) == RUNT_OK) {
                stuff->state->free(stuff->state->ud);
            }
            free(stuff->mem);
            free(stuff->cells);
            free(stuff);
            break;
    }
    return PLUMBER_OK;
}

plumber_dyn_func sporth_return_ugen()
{
    return sporth_spigot;
}
