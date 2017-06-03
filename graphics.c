#include <GLFW/glfw3.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <soundpipe.h>
#include <sporth.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "spigot.h"

#include "box.h"

#define WIDTH 193

struct spigot_graphics {
    int run;
    pthread_t thread;
    GLFWwindow *window;
    int please_draw;
    unsigned char *buf;
    spigot_state *state;
    int zoom;
};

void spigot_color_rgb(spigot_color *clr, uint8_t r, uint8_t g, uint8_t b)
{
    clr->r = r;
    clr->g = g;
    clr->b = b;
}

void spigot_color_rgb_hex(spigot_color *clr, long rgb)
{
    clr->r = (rgb & 0xff0000) >> 16;
    clr->g = (rgb & 0x00ff00) >> 8;
    clr->b = (rgb & 0x0000ff);
}

/* NOTE: dimensions should be multiples of 8. if not, you better zero
 * pad your bitmaps */
void spigot_draw_bitmap(spigot_graphics *gfx, spigot_color *clr, 
        int x_pos, int y_pos, int w, int h, const unsigned char *glyph)
{
    int x_bytes;
    int x, y, c;
    unsigned char byte;
    int pos;
    int xp;
    int base;

    x_bytes = ((w - 1) >> 3) + 1;

    base = y_pos * WIDTH * 3 + x_pos * 3;
    for(y = 0; y < h; y++) {
        xp = 0;
        for(x = 0; x < x_bytes; x++) {
            byte = glyph[y * x_bytes + x];
            for(c = 7; c >= 0; c--) {
                pos = base + y * WIDTH * 3 + xp * 3;
                if((byte & (1 << c))) {
                    gfx->buf[pos] = clr->r; 
                    gfx->buf[pos + 1] = clr->g; 
                    gfx->buf[pos + 2] = clr->b; 
                } 
                xp++;
            }
        }
    }

}

void spigot_draw_box(spigot_graphics *gfx, spigot_color *clr, int pos)
{
    int x, y;

    x = (pos % 12) * 16;
    y = (pos / 12) * 16;

    spigot_draw_bitmap(gfx, clr, x, y, 1, 17, spigot_box);
    spigot_draw_bitmap(gfx, clr, x + 16, y, 1, 17, spigot_box);
    spigot_draw_bitmap(gfx, clr, x, y + 16, 16, 1, spigot_box + 17);
    spigot_draw_bitmap(gfx, clr, x, y, 16, 1, spigot_box + 17);
}

/* static void draw(spigot_pbrain *spb) */
static void draw(spigot_graphics *gfx)
{
    spigot_state *state;

    state = gfx->state;

    state->draw(gfx, state->ud);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0, 0);
    glPixelZoom(gfx->zoom, -gfx->zoom);
    glDrawPixels(193, 193, GL_RGB, GL_UNSIGNED_BYTE, gfx->buf);
}

static void init(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

static void errorcb(int error, const char* desc)
{
    printf("GLFW error %d: %s\n", error, desc);
}

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    spigot_graphics *gfx;
    spigot_state *state;
    
    gfx = glfwGetWindowUserPointer(window);
    state = gfx->state;

    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch(key) {
            case GLFW_KEY_SPACE:
                state->toggle(state->ud);
                break;
            case GLFW_KEY_H:
                state->left(state->ud);
                gfx->please_draw = 1;
                break;
            case GLFW_KEY_L:
                state->right(state->ud);
                gfx->please_draw = 1;
                break;
            case GLFW_KEY_J:
                state->down(state->ud);
                gfx->please_draw = 1;
                break;
            case GLFW_KEY_K:
                state->up(state->ud);
                gfx->please_draw = 1;
                break;
            case GLFW_KEY_Z:
                state->reset(state->ud);
                gfx->please_draw = 1;
                break;
            default:
                break;
        }
    }
}

static void * run_loop(void *ud)
{
    spigot_graphics *spgt = (spigot_graphics *)ud;

    int w, h;

    if (!glfwInit()) {
        printf("Failed to init GLFW.");
        pthread_exit(NULL);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    spgt->window = glfwCreateWindow(193 * spgt->zoom, 193 * spgt->zoom, 
            "spigot", NULL, NULL);

    if (!spgt->window) {
        glfwTerminate();
        pthread_exit(NULL);
    }

    glfwSetKeyCallback(spgt->window, key);
    glfwSetErrorCallback(errorcb);
    glfwMakeContextCurrent(spgt->window);

    glfwSwapInterval(0);
    glfwSetTime(0);
    init();
    glfwSetWindowUserPointer(spgt->window, spgt);
    while(spgt->run) {
        glfwPollEvents();
        glfwGetFramebufferSize(spgt->window, &w, &h);

        glViewport(0, 0, (GLsizei) w, (GLsizei) h); 
        /* glViewport(w, h, (GLsizei) w, (GLsizei) h); */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        /* glOrtho(0, w, h, 0, -1.0, 1.0); */
        glOrtho(0, w, h, 0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);

        if(spgt->please_draw) {
            spgt->please_draw = 0;
            draw(spgt);
            glfwSwapBuffers(spgt->window);
        }
        usleep(8000);
    }


    glfwTerminate();
    pthread_exit(0);
    return NULL;
}

void spigot_start(spigot_graphics *spgt)
{
    spgt->run = 1;
    pthread_create(&spgt->thread, NULL, run_loop, spgt);
}

void spigot_stop(spigot_graphics *spgt)
{
    if(spgt->run) {
        spgt->run = 0;
        pthread_join(spgt->thread, NULL);
    }
}

void spigot_gfx_init(spigot_graphics *gfx)
{
    int i;
    spigot_color clr;
    spigot_state *state;
    spigot_color_rgb_hex(&clr, 0x000000);
    state = gfx->state;
    for(i = 0; i < 193 * 193 * 3; i+=3) {
        gfx->buf[i] = clr.r;
        gfx->buf[i+ 1] = clr.g;
        gfx->buf[i + 2] = clr.b;
    }
    /* parse_code(gfx); */

    state->gfx_init(gfx, state->ud);
}

spigot_graphics * spigot_gfx_new(int zoom)
{
    spigot_graphics *gfx;
    gfx = malloc(sizeof(spigot_graphics));
    gfx->buf = malloc(193 * 193 * 3 * sizeof(unsigned char));
    gfx->zoom = zoom;
    return gfx;
}

void spigot_gfx_free(spigot_graphics *gfx)
{
    free(gfx->buf);
    free(gfx);
}

void spigot_gfx_step(spigot_graphics *gfx)
{
    gfx->please_draw = 1;
}

void spigot_gfx_set_state(spigot_graphics *gfx, spigot_state *state)
{
    gfx->state = state;
}

unsigned char * spigot_graphics_get_buf(spigot_graphics *gfx)
{
    return gfx->buf;
}

void spigot_draw_hline(spigot_graphics *gfx, 
        spigot_color *clr, int pos, int start, int len)
{
    int p;
    int off;
    unsigned char *buf;

    buf = spigot_graphics_get_buf(gfx);

    for(p = 0; p < len; p++) {
        off = (p + start) * 3 + pos * 193 * 3;
        buf[off] = clr->r;
        buf[off + 1] = clr->g;
        buf[off + 2] = clr->b;
    }
    
}

void spigot_draw_vline(spigot_graphics *gfx, 
        spigot_color *clr, int pos, int start, int len)
{
    int p;
    unsigned int off;
    unsigned char *buf;

    buf = spigot_graphics_get_buf(gfx);

    for(p = 0; p < len; p++) {
        off = (p + start) * 3 * 193 + pos * 3;
        buf[off] = clr->r;
        buf[off + 1] = clr->g;
        buf[off + 2] = clr->b;
    }
}

void spigot_draw_glyph(spigot_graphics *gfx, spigot_color *clr, 
        int x_pos, int y_pos, 
        int w, int h, 
        int stride,
        const unsigned char *glyph)
{
    unsigned int glyph_pos;
    unsigned int img_pos;
    unsigned int x, y;
    unsigned char *buf;
    
    buf = spigot_graphics_get_buf(gfx);

    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            glyph_pos =  (y*stride) + x;
            if(glyph[glyph_pos]) {
                img_pos = ((y + y_pos)*193*3) + (x + x_pos)*3;
                buf[img_pos] = clr->r;
                buf[img_pos + 1] = clr->g;
                buf[img_pos + 2] = clr->b;
            } 
        }
    }
}

void spigot_draw_fill(spigot_graphics *gfx, spigot_color *clr)
{
    unsigned char *buf;
    unsigned int pos;
    unsigned int x, y;

    buf = spigot_graphics_get_buf(gfx);

    for(y = 0; y < 193; y++) {
       for(x = 0; x < 193; x++) {
            pos = y * 193 * 3 + x * 3;
            buf[pos] = clr->r;
            buf[pos + 1] = clr->g;
            buf[pos + 2] = clr->b;
       }
    }
}

void spigot_draw_rect(spigot_graphics *gfx, spigot_color *clr,
        int pos_x, int pos_y, int w, int h)
{
    int x, y;
    int pos;
    unsigned char *buf;
    buf = spigot_graphics_get_buf(gfx);

    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            pos = (pos_y + y) * 193 * 3 + (pos_x + x) * 3;
            buf[pos] = clr->r;        
            buf[pos + 1] = clr->g;
            buf[pos + 2] = clr->b;
        }
    }
}
