#ifndef SPIGOT_H
#define SPIGOT_H

typedef struct spigot_pbrain spigot_pbrain;

typedef struct spigot_graphics spigot_graphics;

int spigot_init(spigot_pbrain *spb, const char *str);
int spigot_constant(spigot_pbrain *spb, unsigned short val);
int spigot_step(spigot_pbrain *spb);

char * spigot_get_code(spigot_pbrain *spb);
int spigot_get_length(spigot_pbrain *spb);
int spigot_get_pos(spigot_pbrain *spb);

spigot_pbrain * spigot_pbrain_new();
void spigot_pbrain_free(spigot_pbrain *spb);

spigot_graphics * spigot_gfx_new();
void spigot_start(spigot_graphics *spgt);
void spigot_stop(spigot_graphics *spgt);
void spigot_toggle_playback(spigot_pbrain *spb);

void spigot_gfx_free(spigot_graphics *gfx);

void spigot_gfx_step(spigot_graphics *gfx);

void spigot_gfx_pbrain_set(spigot_graphics *gfx, spigot_pbrain *spb);

#endif
