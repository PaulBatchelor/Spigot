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
#include <string.h>
#include "plumbstream.h"

struct spigot_db {
    sqlite3 *db;
    plumber_data *pd;
};

int spigot_db_open(spigot_db *db, const char *filename)
{
    const char *sampdir;
    char cwd[1024];
    /* TODO: implement me! */
    sampdir = getenv("SPIGOT_SAMPDIR");
    if(sampdir == NULL) {
        fprintf(stderr, "error: SPIGOT_SAMPDIR must be set!\n");
        return 0;
    }
    getcwd(cwd, 1024);
    chdir(sampdir);
    fprintf(stderr, "cd-ing to %s\n", sampdir);
    sqlite3_open(filename, &db->db);
    chdir(cwd);
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

static runt_int rproc_db(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    spigot_db *db;

    rc = runt_malloc(vm, spigot_db_size(), (void **)&db);
    if(rc == 0) return RUNT_NOT_OK;
   
    rc = runt_ppush(vm, &s);
    RUNT_ERROR_CHECK(rc);
    s->p = runt_mk_cptr(vm, db);
    runt_mark_set(vm);

    return RUNT_OK;
}

static runt_int rproc_db_open(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    spigot_db *db;
    const char *filename;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    filename = runt_to_string(s->p);

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    db = runt_to_cptr(s->p);

    spigot_db_open(db, filename);

    rc = runt_ppush(vm, &s);
    RUNT_ERROR_CHECK(rc);
    s->p = runt_mk_cptr(vm, db);

    return RUNT_OK;
}

static runt_int rproc_db_close(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    spigot_db *db;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    db = runt_to_cptr(s->p);
    
    spigot_db_close(db);

    return RUNT_OK;
}

static runt_int rproc_cdb(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    spigot_db *db;
    runt_uint id;
    int *fd;
    plumber_data *pd;
    plumber_stream *stream;
    const char *ftname;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    pd = runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    stream = runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    db = runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    ftname = runt_to_string(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    id = s->f;
  
    /* allocate fd using malloc... plumber will free it later */
    fd = malloc(sizeof(int));
    *fd = spigot_cdb_open(db, id);

    /* add plumber data to stream */

    plumber_stream_append_data(pd, 
        stream, 
        ftname, 
        strlen(ftname), 
        fd,
        1,
        PTYPE_USERDATA);

    rc = runt_ppush(vm, &s);
    RUNT_ERROR_CHECK(rc);
    s->f = *fd;

    return RUNT_OK;
}

static runt_int rproc_cdb_close(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_int fd;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    fd = s->f;

    close(fd);

    return RUNT_OK;
}

size_t spigot_db_runt(runt_vm *vm, runt_ptr p)
{
    /* create new db */
    spigot_word_define(vm, p, "spigot_db", 9, rproc_db);
    /* open a db */
    spigot_word_define(vm, p, "spigot_db_open", 14, rproc_db_open);
    /* close the db */
    spigot_word_define(vm, p, "spigot_db_close", 15, rproc_db_close);
    /* open cdb file from db */
    spigot_word_define(vm, p, "spigot_cdb", 10, rproc_cdb);
    spigot_word_define(vm, p, "spigot_cdb_close", 16, rproc_cdb_close);
    return runt_is_alive(vm);
}
