#include <sqlite3.h>
#include <soundpipe.h>
#include <sporth.h>
#include "spigot.h"

struct spigot_db {
    sqlite3 *db;
    plumber_data *pd;
};

int spigot_db_open(spigot_db *db, plumber_data *pd, const char *filename)
{
    /* TODO: implement me! */
    sqlite3_open(filename, &db->db);
    db->pd = pd;
    return 0;
}

int spigot_cdb_open(spigot_db *db, unsigned int id)
{
    /* TODO: implement me! */

    /* return fid */
    return 0;
}

void spigot_cdb_close(spigot_db *db, int fd)
{
    /* TODO implmement me! */
}

int spigot_db_close(spigot_db *db)
{
    /* TODO implement me! */
    sqlite3_close(db->db);
    return 0;
}
