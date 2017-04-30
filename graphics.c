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

#include "bitmaps.h"

#define WIDTH 193
#define ZOOM 4

typedef struct {
    char r, g, b;
} spigot_color;

struct spigot_graphics {
    int run;
    pthread_t thread;
    GLFWwindow *window;
    int please_draw;
    spigot_pbrain *pbrain;
    unsigned char *buf;
};

static void clear_color_rgb(uint32_t rgb)
{
    int r, g, b;
    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0x0000ff);
    glClearColor(r / 255.0, g / 255.0, b / 255.0, 1.0);
}

static void color_rgb(spigot_color *clr, uint32_t rgb)
{
    clr->r = (rgb & 0xff0000) >> 16;
    clr->g = (rgb & 0x00ff00) >> 8;
    clr->b = (rgb & 0x0000ff);
}

/* NOTE: dimensions should be multiples of 8. if not, you better zero
 * pad your bitmaps */
static void spigot_draw_bitmap(spigot_graphics *gfx, spigot_color *clr, 
        int x_pos, int y_pos, int w, int h, unsigned char *glyph)
{
    int x_bytes;
    int y_bytes;
    int x, y, c;
    unsigned char byte;
    int pos;
    int xp;
    int base;

    x_bytes = ((w - 1) >> 3) + 1;
    printf("x_bytes is %d\n", x_bytes);

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

static void parse_code(spigot_graphics *gfx)
{
    spigot_color clr;
    const char *code;
    spigot_pbrain *spb;
    int x_pos, y_pos;
    int len;
    int s;
    int off;

    spb = gfx->pbrain;
    len = spigot_get_length(spb);
    code = spigot_get_code(spb);

    x_pos= 0;
    y_pos = 0;
    s = 0;
    off = 0;

    color_rgb(&clr, 0x84de02);
    while(s < len) {
        switch(code[s]) {
            case '+':
                off = 0;
                break;
            case '-':
                off = 5;
                break;
            case '<':
                off = 10;
                break;
            case '>':
                off = 15;
                break;
            case '.':
                off = 20;
                break;
            case ',':
                off = 25;
                break;
            case '[':
                off = 30;
                break;
            case ']':
                off = 35;
                break;
            default:
                off = 40;
                break;
        }
        spigot_draw_bitmap(gfx, &clr, x_pos * 16 + 3, y_pos * 16 + 6, 5, 5, spigot_bitmaps + off);
        x_pos++;

        if(x_pos > 11) {
            x_pos = 0;
            y_pos++;
        }
        s++;
    }
    color_rgb(&clr, 0x84de02);

    spigot_draw_bitmap(gfx, &clr, 0, 0, 1, 17, spigot_box);
    spigot_draw_bitmap(gfx, &clr, 16, 0, 1, 17, spigot_box);
    spigot_draw_bitmap(gfx, &clr, 0, 16, 16, 1, spigot_box + 17);
    spigot_draw_bitmap(gfx, &clr, 0, 0, 16, 1, spigot_box + 17);
}

/* static void draw(spigot_pbrain *spb) */
static void draw(spigot_graphics *gfx)
{
/*
    const char *code;
    int len, s;
    int x, y;
    int pos;
    int off;
    int xoff;
    int yoff;
    int cnt;
    spigot_pbrain *spb;

    spb = gfx->pbrain;
    pos = spigot_get_pos(spb);

    x = pos % 12; 
    y = pos / 12;
    code = spigot_get_code(spb);
    len = spigot_get_length(spb);
    clear_color_rgb(0x000000);
    color_rgb(0x84de02);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(3, 11);
    s = 0;
    cnt = 0;
    while(s < len) {
        xoff = 16;
        yoff = 0;
        switch(code[s]) {
            case '+':
                off = 0;
                break;
            case '-':
                off = 5;
                break;
            case '<':
                off = 10;
                break;
            case '>':
                off = 15;
                break;
            case '.':
                off = 20;
                break;
            case ',':
                off = 25;
                break;
            case '[':
                off = 30;
                break;
            case ']':
                off = 35;
                break;
            default:
                off = 40;
                break;
        }
        if(cnt == 11) {
            cnt = 0;
            xoff = -176;
            yoff = -16;
        }
        glBitmap(8, 5, 0, 0, xoff, yoff, spigot_bitmaps + off);
        s++;
        cnt++;
    }
    
    glRasterPos2i(x * 16, 16 + y * 16);
    glBitmap(1, 17, 0, 0, 16, 0, spigot_box);
    glBitmap(1, 17, 0, 0, -16, 0, spigot_box);
    glBitmap(16, 1, 0, 0, 0, 15, spigot_box + 17);
    glBitmap(16, 1, 0, 0, 0, 0, spigot_box + 17);
*/
    glClear(GL_COLOR_BUFFER_BIT);
    /* glRasterPos2i(193, 193 * 4); */
    glRasterPos2i(0, 0);
    glPixelZoom(ZOOM, -ZOOM);
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
    spigot_graphics *gfx = glfwGetWindowUserPointer(window);

    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch(key) {
            case GLFW_KEY_SPACE:
                spigot_toggle_playback(gfx->pbrain);
                break;
            case GLFW_KEY_H:
                spigot_move_left(gfx->pbrain);
                gfx->please_draw = 1;
                break;
            case GLFW_KEY_L:
                spigot_move_right(gfx->pbrain);
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

    spgt->window = glfwCreateWindow(193 * ZOOM, 193 * ZOOM, "spigot", NULL, NULL);

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
    color_rgb(&clr, 0x000000);
    for(i = 0; i < 193 * 193 * 3; i+=3) {
        gfx->buf[i] = clr.r;
        gfx->buf[i+ 1] = clr.g;
        gfx->buf[i + 2] = clr.b;
    }

    parse_code(gfx);
}

spigot_graphics * spigot_gfx_new()
{
    spigot_graphics *gfx;
    spigot_color clr;
    int i;

    unsigned char tmp;
    int pos;

    tmp = 0b10101111;
    gfx = malloc(sizeof(spigot_graphics));
    gfx->buf = malloc(193 * 193 * 3 * sizeof(unsigned char));
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

void spigot_gfx_pbrain_set(spigot_graphics *gfx, spigot_pbrain *spb)
{
    gfx->pbrain = spb;
}
