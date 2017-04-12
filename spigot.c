#include <stdlib.h>
#include <soundpipe.h>
#include <sporth.h>

#include "spigot.h"

static int sporth_spigot(plumber_data *pd, sporth_stack *stack, void **ud)
{
    const char *filename;
    SPFLOAT in;
    int tick;
    switch(pd->mode) {
        case PLUMBER_CREATE:
            if(sporth_check_args(stack, "fs") != SPORTH_OK) {
                fprintf(stderr,"Not enough arguments for spigot\n");
                stack->error++;
                return PLUMBER_NOTOK;
            }
            /* malloc and assign address to user data */
            /*foo = malloc(sizeof(foo_data));
            *ud = foo;*/
            sporth_stack_pop_string(stack);
            sporth_stack_pop_float(stack);
            sporth_stack_push_float(stack, 0.0); 
            break;
        case PLUMBER_INIT:
            filename = sporth_stack_pop_string(stack);
            spigot_init(filename);
            sporth_stack_pop_float(stack);
            sporth_stack_push_float(stack, 0.0);
            break;

        case PLUMBER_COMPUTE:
            in = sporth_stack_pop_float(stack);
            tick = 0;
            if(in != 0) {
                tick = spigot_step();
            }

            if(tick) {
                sporth_stack_push_float(stack, 1.0);
            } else {
                sporth_stack_push_float(stack, 0.0);
            }

            break;

        case PLUMBER_DESTROY:
            /*
            foo = *ud;
            free(foo);
            */
            break;
    }
    return PLUMBER_OK;
}

plumber_dyn_func sporth_return_ugen()
{
    return sporth_spigot;
}
