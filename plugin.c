#include <stdlib.h>
#include <soundpipe.h>
#include <sporth.h>

#include "spigot.h"

#ifndef BUILD_SPORTH_PLUGIN
extern spigot_graphics *global_gfx;
#endif

typedef struct {
    spigot_graphics *gfx;
    int load;
    spigot_state *state;
    runt_vm vm;
    runt_cell *cells;
    unsigned char *mem;
} spigot_stuff;

int sporth_spigot(plumber_data *pd, sporth_stack *stack, void **ud)
{
    const char *filename;
    SPFLOAT in;
    spigot_stuff *stuff;
    int rc;
    int zoom;
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
            runt_cell_pool_set(&stuff->vm, stuff->cells, 1024);
            runt_cell_pool_init(&stuff->vm);
            runt_memory_pool_set(&stuff->vm, stuff->mem, 5 * RUNT_MEGABYTE);

            zoom = 3;
            runt_load_stdlib(&stuff->vm);
            runt_mark_set(&stuff->vm);
            rc = spigot_load(pd, &stuff->vm, &zoom);
            rc = spigot_parse(&stuff->vm, filename, &stuff->state);

#ifndef BUILD_SPORTH_PLUGIN
            stuff->gfx = global_gfx;
            spigot_set_zoom(stuff->gfx, zoom);
#else
            stuff->gfx = spigot_gfx_new(zoom);
#endif

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

            if(stuff->load) {
                spigot_start_why_dont_you(stuff->gfx);
#ifdef BUILD_SPORTH_PLUGIN
                spigot_gfx_init(stuff->gfx);
                spigot_start(stuff->gfx);
#endif
            }

            break;

        case PLUMBER_COMPUTE:
            stuff = *ud;
            sporth_stack_pop_float(stack);
            in = sporth_stack_pop_float(stack);
            if(in != 0) {
                if(spigot_is_it_happening(stuff->gfx)) {
                    spigot_gfx_step(stuff->gfx);
                }
                stuff->state->step(stuff->state->ud);
            }

            stuff->state->compute(stuff->state->ud, in);

            break;

        case PLUMBER_DESTROY:
            stuff = *ud;
            if(runt_is_alive(&stuff->vm) == RUNT_OK) {
                stuff->state->free(stuff->state->ud);
            }
#ifdef BUILD_SPORTH_PLUGIN
            if(spigot_is_it_happening(stuff->gfx)) {
                spigot_stop(stuff->gfx);
            }
            spigot_gfx_free(stuff->gfx);
#endif
            runt_close_plugins(&stuff->vm);
            free(stuff->mem);
            free(stuff->cells);
            free(stuff);
            break;
    }
    return PLUMBER_OK;
}

#ifdef BUILD_SPORTH_PLUGIN
plumber_dyn_func sporth_return_ugen()
{
    return sporth_spigot;
}
#endif
