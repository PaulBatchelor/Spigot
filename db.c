#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <soundpipe.h>
#include <sporth.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "spigot.h"

struct spigot_db {
    sqlite3 *db;
    plumber_data *pd;
};

int spigot_db_open(spigot_db *db, plumber_data *pd, const char *filename)
{
    const char *sampdir;
    char cwd[1024];
    /* TODO: implement me! */
    sampdir = getenv("SPIGOT_SAMPDIR");
    if(sampdir == NULL) {
        fprintf(stderr, "error: SAMPDIR must be set!\n");
        return 0;
    }
    getcwd(cwd, 1024);
    chdir(sampdir);
    fprintf(stderr, "cd-ing to %s\n", sampdir);
    sqlite3_open(filename, &db->db);
    chdir(cwd);
    db->pd = pd;
    return 0;
}

int spigot_cdb_open(spigot_db *db, unsigned int id)
{
    const char *sampdir;
    const char *path;
    char cwd[1024];
    int fd;
    int rc;
    sqlite3_stmt *stmt;
    sampdir = getenv("SPIGOT_SAMPDIR");
    getcwd(cwd, 1024);
    chdir(sampdir);
    
    sqlite3_prepare_v2(db->db, 
        "SELECT path FROM cdb WHERE(id == ?1);",
        -1,
        &stmt,
        NULL);
    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ERROR) {
        fprintf(stderr, "error: %s\n", sqlite3_errmsg(db->db));
    }
    path = (const char *)sqlite3_column_text(stmt, 0);
    fprintf(stderr, "opening path %s\n", path);

    fprintf(stderr, "cd-ing to %s\n", sampdir);
    chdir(sampdir);
    fd = open(path, O_RDONLY);

    chdir(cwd);

    sqlite3_finalize(stmt);
    return fd;
}

void spigot_cdb_close(spigot_db *db, int fd)
{
    close(fd);
}

int spigot_db_close(spigot_db *db)
{
    /* TODO implement me! */
    sqlite3_close(db->db);
    return 0;
}

size_t spigot_db_size()
{
    return sizeof(spigot_db);
}
