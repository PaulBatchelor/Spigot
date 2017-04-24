typedef struct spigot_graphics spigot_graphics;

int spigot_init(const char *str);
int spigot_constant(unsigned char val);
int spigot_step();

char * spigot_get_code();

spigot_graphics * spigot_gfx_new();
void spigot_start(spigot_graphics *spgt);
void spigot_stop(spigot_graphics *spgt);

void spigot_gfx_free(spigot_graphics *gfx);

void spigot_gfx_step(spigot_graphics *gfx);

char * spigot_get_code();
int spigot_get_length();
int spigot_get_pos();
