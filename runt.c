#include <soundpipe.h>
#include <sporth.h>
#include "spigot.h"

typedef struct {
    plumber_data *pd;
    spigot_state *state;
} runt_spigot_data;

static runt_int rproc_pbrain(runt_vm *vm, runt_ptr p)
{
    runt_spigot_data *rsd;
    spigot_state *state;
    const char *code;
    const char *var;
    runt_int rc;
    runt_stacklet *s;
    plumber_data *pd;

    rsd = runt_to_cptr(p);
    pd = rsd->pd;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    var = runt_to_string(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    code = runt_to_string(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);

    spigot_pbrain_state(pd, state);
    spigot_pbrain_string(state, code);
    spigot_pbrain_bind(pd, state, var);

    rc = runt_ppush(vm, &s);
    s->p = runt_mk_cptr(vm, state);
    return RUNT_OK;
}

static runt_int rproc_state(runt_vm *vm, runt_ptr p)
{
    runt_uint rc;
    runt_stacklet *s;
    spigot_state *state;
    runt_spigot_data *rsd;

    rsd = runt_to_cptr(p);
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);
    rsd->state = state;
    return RUNT_OK;
}

static runt_int rproc_newstate(runt_vm *vm, runt_ptr p)
{
    runt_uint rc;
    runt_stacklet *s;
    spigot_state *state;

    rc = runt_ppush(vm, &s);
    RUNT_ERROR_CHECK(rc);
    
    runt_malloc(vm, sizeof(spigot_state), (void **)&state);
    /* make sure pool doesn't release the memory */
    spigot_state_init(state);
    runt_mark_set(vm);
    state->magic=666;
    s->p = runt_mk_cptr(vm, state);

    return RUNT_OK;
}

static runt_int rproc_tracker(runt_vm *vm, runt_ptr p)
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

    spigot_tracker_state(pd, state);

    rc = runt_ppush(vm, &s);
    s->p = runt_mk_cptr(vm, state);
    return RUNT_OK;
}


static void spigot_word_define(runt_vm *vm,
    const char *str,
    runt_uint size,
    runt_proc proc,
    runt_ptr p)
{
    runt_uint id;
    id = runt_word_define(vm, str, size, proc);
    runt_word_bind_ptr(vm, id, p);
}

void spigot_load(plumber_data *pd, runt_vm *vm, 
        spigot_state **state, const char *filename)
{
    runt_spigot_data *rsd;
    runt_ptr p;

    runt_malloc(vm, sizeof(runt_spigot_data), (void **)&rsd);
    rsd->pd = pd;
    p = runt_mk_cptr(vm, rsd);

    runt_mark_set(vm);
    runt_load_stdlib(vm);
    spigot_word_define(vm, "pbrain", 6, rproc_pbrain, p);
    spigot_word_define(vm, "spigot_state", 12, rproc_state, p);
    spigot_word_define(vm, "new_state", 9, rproc_newstate, p);
    spigot_word_define(vm, "tracker", 7, rproc_tracker, p);

    runt_mark_set(vm);
    runt_set_state(vm, RUNT_MODE_INTERACTIVE, RUNT_ON);
    runt_parse_file(vm, filename);
    
    *state = rsd->state;
}
