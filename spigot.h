#ifndef SPIGOT_H
#define SPIGOT_H

typedef struct spigot_pbrain spigot_pbrain;

typedef struct spigot_graphics spigot_graphics;

typedef void (*spigot_fun)(void *);
typedef void (*spigot_drawfun)(spigot_graphics *, void *);

typedef struct {
    spigot_fun up;
    spigot_fun down;
    spigot_fun left;
    spigot_fun right;
    spigot_fun toggle;
    spigot_fun reset;
    spigot_fun free;
    spigot_drawfun draw;
    spigot_drawfun init;
    void *ud;
} spigot_state;

typedef struct {
    char r, g, b;
} spigot_color;

void spigot_color_rgb(spigot_color *clr, long rgb);

int spigot_init(spigot_pbrain *spb, const char *str);
int spigot_constant(spigot_pbrain *spb, unsigned short val);
int spigot_step(spigot_pbrain *spb);

char * spigot_get_code(spigot_pbrain *spb);
int spigot_get_length(spigot_pbrain *spb);
int spigot_get_pos(spigot_pbrain *spb);

spigot_pbrain * spigot_pbrain_new();
void spigot_pbrain_free(void *ud);

spigot_graphics * spigot_gfx_new();
void spigot_start(spigot_graphics *spgt);
void spigot_stop(spigot_graphics *spgt);

void spigot_state_init(spigot_state *state);

void spigot_gfx_init(spigot_graphics *spgt);
void spigot_gfx_free(spigot_graphics *gfx);
void spigot_gfx_step(spigot_graphics *gfx);
void spigot_gfx_pbrain_set(spigot_graphics *gfx, spigot_pbrain *spb);
void spigot_gfx_set_state(spigot_graphics *gfx, spigot_state *state);

void spigot_pbrain_state(plumber_data *pd, spigot_pbrain *spb, spigot_state *state);

void spigot_draw_box(spigot_graphics *gfx, spigot_color *clr, int pos);
void spigot_draw_bitmap(spigot_graphics *gfx, spigot_color *clr, 
        int x_pos, int y_pos, int w, int h, const unsigned char *glyph);

#endif
