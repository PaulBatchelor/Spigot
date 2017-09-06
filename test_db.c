#include <soundpipe.h>
#include <sporth.h>
#include <stdlib.h>
#include "spigot.h"

void process(sp_data *sp, void *ud)
{
    SPFLOAT out;
    plumber_data *pd = ud;
    plumber_compute(pd, PLUMBER_COMPUTE);
    out  = sporth_stack_pop_float(&pd->sporth.stack);
    sp_out(sp, 0, out);
}

int main()
{
    sp_data *sp;
    spigot_db *db;
    int fd;

    db = malloc(spigot_db_size());
    sp_create(&sp);
    plumber_data pd;
    plumber_register(&pd);
    plumber_init(&pd);
    pd.sp = sp;
    spigot_db_open(db, "data.db");
    fd = spigot_cdb_open(db, 1);

    plumber_ftmap_delete(&pd, 0);
    plumber_ftmap_add_userdata(&pd, "cdb", &fd);
    plumber_ftmap_delete(&pd, 1);
    plumber_parse_string(&pd, 
    "_kick 'kick' _cdb cdbtab "
    "1 metro dup _kick tbldur inv 0 tphasor "
    "1 0 0 _kick tabread "
    "swap _kick tbldur tgate * "
    );
    plumber_compute(&pd, PLUMBER_INIT);

    sp_process(sp, &pd, process);

    spigot_db_close(db);
    plumber_clean(&pd);
    sp_destroy(&sp);
    free(db);
    return 0;
}
