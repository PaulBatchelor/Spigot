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

struct spigot_graphics {
    int run;
    pthread_t thread;
    GLFWwindow *window;
    int please_draw;
};

static void clear_color_rgb(uint32_t rgb)
{
    int r, g, b;
    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0x0000ff);
    glClearColor(r / 255.0, g / 255.0, b / 255.0, 1.0);
}

static void color_rgb(uint32_t rgb)
{
    int r, g, b;
    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0x0000ff);
    glColor3f(r / 255.0, g / 255.0, b / 255.0);
}

static void draw()
{
    const char *code;
    int len, s;
    int x, y;
    int pos;
    int off;
    int xoff;
    int yoff;
    int cnt;

    pos = spigot_get_pos();

    x = pos % 12; 
    y = pos / 12;
    code = spigot_get_code();
    len = spigot_get_length();
    clear_color_rgb(0x181818);
    color_rgb(0xF5AA9D);
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

    spgt->window = glfwCreateWindow(193, 193, "spigot", NULL, NULL);

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
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, h, 0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);

        if(spgt->please_draw) {
            spgt->please_draw = 0;
            draw();
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

spigot_graphics * spigot_gfx_new()
{
    return malloc(sizeof(spigot_graphics));
}

void spigot_gfx_free(spigot_graphics *gfx)
{
    free(gfx);
}

void spigot_gfx_step(spigot_graphics *gfx)
{
    gfx->please_draw = 1;
}
