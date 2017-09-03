#include <soundpipe.h>
#include <sporth.h>
#include "spigot.h"
/* needs to be included after spigot to get runt.h */
#include <runt_plumber.h>

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

    spigot_pbrain_state(pd, vm, state);
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
    rsd->loaded = 1;
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

    spigot_tracker_state(pd, vm, state);

    rc = runt_ppush(vm, &s);
    s->p = runt_mk_cptr(vm, state);
    return RUNT_OK;
}

static int rproc_zoom(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;

    rsd = runt_to_cptr(p);

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    *rsd->zoom = s->f;

    return RUNT_OK;
}

static runt_int rproc_plumb(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;

    rsd = runt_to_cptr(p);
    
    rc = runt_ppush(vm, &s);
    RUNT_ERROR_CHECK(rc);
    s->p = runt_mk_cptr(vm, rsd->pd);

    return RUNT_OK;
}

static runt_int rproc_plumber(runt_vm *vm, runt_ptr p)
{
    runt_load_plumber(vm);
    runt_mark_set(vm);
    return runt_is_alive(vm);
}

static runt_int rproc_ugen(runt_vm *vm, runt_ptr p)
{
    const char *name;
    spigot_state *state;
    plumber_data *pd;
    runt_int rc;
    runt_stacklet *s;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    pd = runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    name = runt_to_string(s->p);

    spigot_add_ugen(pd, name, state);

    return RUNT_OK;
}

static runt_int rproc_step(runt_vm *vm, runt_ptr p)
{
    spigot_state *state;
    runt_stacklet *s;
    runt_int rc;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);

    state->step(state->ud);

    return RUNT_OK;
}

static runt_int rproc_init(runt_vm *vm, runt_ptr p)
{
    spigot_state *state;
    runt_stacklet *s;
    runt_int rc;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);

    state->init(state->ud);

    return RUNT_OK;
}

static runt_int rproc_free(runt_vm *vm, runt_ptr p)
{
    spigot_state *state;
    runt_stacklet *s;
    runt_int rc;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = runt_to_cptr(s->p);

    state->free(state->ud);

    return RUNT_OK;
}

void spigot_word_define(runt_vm *vm, runt_ptr p,
    const char *str,
    runt_uint size,
    runt_proc proc)
{
    runt_uint id;
    id = runt_word_define(vm, str, size, proc);
    runt_word_bind_ptr(vm, id, p);
}

int spigot_parse(runt_vm *vm, const char *filename, spigot_state **state)
{
    runt_spigot_data *rsd;

    rsd = spigot_get_runt_data(vm);
    runt_set_state(vm, RUNT_MODE_INTERACTIVE, RUNT_ON);
    if(runt_parse_file(vm, filename) != RUNT_OK) {
        runt_seppuku(vm);
        return PLUMBER_NOTOK;
    }
    
    *state = rsd->state;
    rsd->state->vm = vm;
    return PLUMBER_OK;
}

/* TODO: remove zoom here */
int spigot_load(plumber_data *pd, runt_vm *vm, int *zoom)
{
    runt_spigot_data *rsd;
    runt_ptr p;

    runt_malloc(vm, sizeof(runt_spigot_data), (void **)&rsd);
    rsd->pd = pd;
    rsd->loaded = 0;
    rsd->zoom = zoom;
    p = runt_mk_cptr(vm, rsd);

    spigot_word_define(vm, p, "pbrain", 6, rproc_pbrain);
    spigot_word_define(vm, p, "spigot_state", 12, rproc_state);
    spigot_word_define(vm, p, "new_state", 9, rproc_newstate);
    spigot_word_define(vm, p, "tracker", 7, rproc_tracker);
    spigot_word_define(vm, p, "spigot_zoom", 11, rproc_zoom);
    spigot_word_define(vm, p, "spigot_plumb", 12, rproc_plumb);
    spigot_word_define(vm, p, "spigot_plumber", 14, rproc_plumber);
    spigot_word_define(vm, p, "spigot_ugen", 11, rproc_ugen);
    spigot_word_define(vm, p, "spigot_step", 11, rproc_step);
    spigot_word_define(vm, p, "spigot_init", 11, rproc_init);
    spigot_word_define(vm, p, "spigot_free", 11, rproc_free);
    spigot_tracker_runt(vm, p);
    spigot_pbrain_runt(vm, p);
    runt_mark_set(vm);

    return runt_is_alive(vm);
}

runt_spigot_data *spigot_get_runt_data(runt_vm *vm)
{
    runt_spigot_data *rsd;
    runt_entry *entry;
    runt_cell *cell;

    /* retrieve "new_state" entry */

    runt_word_search(vm, "new_state", 9, &entry);

    cell = entry->cell;

    rsd = runt_to_cptr(cell->p);

    return rsd;
}
