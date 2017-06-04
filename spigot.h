#ifndef SPIGOT_H
#define SPIGOT_H
#include <runt.h>

typedef struct spigot_pbrain spigot_pbrain;

typedef struct spigot_graphics spigot_graphics;

typedef void (*spigot_fun)(void *);
typedef void (*spigot_constfun)(void *, SPFLOAT);
typedef void (*spigot_drawfun)(spigot_graphics *, void *);
typedef void (*spigot_keyfun)(spigot_graphics *, void *, int, int, int, int);

enum {
SPIGOT_NONE,
SPIGOT_PBRAIN,
SPIGOT_TRACKER
};

typedef struct {
    spigot_fun up;
    spigot_fun down;
    spigot_fun left;
    spigot_fun right;
    spigot_fun toggle;
    spigot_fun reset;
    spigot_fun free;
    spigot_fun step;
    spigot_fun init;
    spigot_constfun constant;

    spigot_drawfun draw;
    spigot_drawfun gfx_init;
    spigot_keyfun key;
    int type;
    void *ud;
} spigot_state;

typedef struct {
    plumber_data *pd;
    spigot_state *state;
    runt_int loaded;
    int *zoom;
} runt_spigot_data;

typedef struct {
    char r, g, b;
} spigot_color;

void spigot_color_rgb_hex(spigot_color *clr, long rgb);
void spigot_color_rgb(spigot_color *clr, uint8_t r, uint8_t g, uint8_t b);

char * spigot_get_code(spigot_pbrain *spb);
int spigot_get_length(spigot_pbrain *spb);
int spigot_get_pos(spigot_pbrain *spb);

spigot_pbrain * spigot_pbrain_new();
void spigot_pbrain_free(void *ud);

spigot_graphics * spigot_gfx_new(int zoom);
void spigot_start(spigot_graphics *spgt);
void spigot_stop(spigot_graphics *spgt);

void spigot_state_init(spigot_state *state);

void spigot_gfx_init(spigot_graphics *spgt);
void spigot_gfx_free(spigot_graphics *gfx);
void spigot_gfx_step(spigot_graphics *gfx);
void spigot_gfx_pbrain_set(spigot_graphics *gfx, spigot_pbrain *spb);
void spigot_gfx_set_state(spigot_graphics *gfx, spigot_state *state);

void spigot_pbrain_state(plumber_data *pd, spigot_state *state);
void spigot_pbrain_string(spigot_state *state, const char *str);
void spigot_pbrain_id(spigot_state *state, runt_uint id);

void spigot_draw_box(spigot_graphics *gfx, spigot_color *clr, int pos);
void spigot_draw_bitmap(spigot_graphics *gfx, spigot_color *clr, 
        int x_pos, int y_pos, 
        int w, int h, 
        const unsigned char *glyph);

void spigot_draw_hline(spigot_graphics *gfx, 
        spigot_color *clr, int pos, int start, int len);
void spigot_draw_vline(spigot_graphics *gfx, 
        spigot_color *clr, int pos, int start, int len);
void spigot_draw_glyph(spigot_graphics *gfx, spigot_color *clr, 
        int x_pos, int y_pos, 
        int w, int h, int stride, 
        const unsigned char *glyph);

void spigot_draw_fill(spigot_graphics *gfx, spigot_color *clr); 

void spigot_draw_rect(spigot_graphics *gfx, spigot_color *clr,
        int pos_x, int pos_y, int w, int h); 


int spigot_load(plumber_data *pd, runt_vm *vm, 
        spigot_state **state, const char *filename, int *zoom);

void spigot_pbrain_bind(plumber_data *pd, spigot_state *state, const char *var);

void spigot_tracker_state(plumber_data *pd, spigot_state *state);
int spigot_tracker_runt(runt_vm *vm, runt_ptr p);

unsigned char * spigot_graphics_get_buf(spigot_graphics *gfx);
void spigot_word_define(runt_vm *vm, runt_ptr p,
    const char *str,
    runt_uint size,
    runt_proc proc);

#endif
