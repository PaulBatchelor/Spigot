/* pbrain interpreter in old-style C 
   This is an interpreter for Paul M. Parks's pbrain programming language,
       a variant of Urban Mueller's brainfuck programming language.
   Do anything you want with this program.
   I welcome bug reports and feature requests.
   Daniel B Cristofani (http://www.hevanet.com/cristofd/)
*/

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <soundpipe.h>
#include <sporth.h>

#include "bitmaps.h"
#include "spigot.h"

#define SIZE 65536
#define CA(x) case x: fprintf(stderr, "Error: " 

struct spigot_pbrain {
    unsigned short a[SIZE];
    int curpos;
    int s[SIZE];
    int sp;
    int ptable[USHRT_MAX+1];
    int t[SIZE];
    int p;
    int q;
    int length;
    char code[SIZE];
    unsigned short in;
    const char *f;
    int play;
    int prev;
    SPFLOAT *out;
    runt_uint fid;
    runt_vm *vm;
};

static void e(spigot_pbrain *spb, int i){
    switch(i){
        CA(2) "call to undefined procedure (%hu)", spb->a[spb->p]); break;
        CA(3) "pointer too far %s", spb->p>0?"right":"left"); break;
        CA(4) "unmatched '[' at byte %d of %s", spb->s[spb->sp], spb->f); 
            break;
        CA(5) "unmatched ']' at byte %d of %s", spb->q, spb->f); 
            break;
        CA(6) "unmatched '(' at byte %d of %s", spb->s[spb->sp], spb->f); 
            break;
        CA(7) "unmatched ')' at byte %d of %s", spb->q, spb->f); 
            break;
        CA(8) "can't open %s", spb->f); 
            break;
    }
    printf(".\n");
}

static void spigot_reset(void *ud)
{
    spigot_pbrain *spb;
    spb = ud;
    spb->curpos = 0;
    spb->q = 0;
    spb->play = 1;
    spb->sp = 0;
    spb->in = 0;
    spb->p = 0;
    spb->prev = 0;
    memset(spb->a, 0, sizeof(short) * SIZE);
}


static void init(spigot_pbrain *spb)
{
    memset(spb->s, 0, sizeof(int) * SIZE);
    memset(spb->t, 0, sizeof(int) * SIZE);
    memset(spb->ptable, 0, sizeof(int) * (USHRT_MAX + 1));
    /*
    if(!(input = fopen(f=filename, "r"))) e(8);
    length = fread(code, 1, SIZE, input);
    fclose(input);
    */
    spigot_reset((void *)spb);
    for(spb->q=0;spb->q<spb->length;spb->q++){
        switch(spb->code[spb->q]){
            case '(': case '[': spb->s[spb->sp++]=spb->q ; break;
            case ')': if(!spb->sp--||spb->code[spb->s[spb->sp]]!='(') e(spb, 7); 
                spb->t[spb->s[spb->sp]]=spb->q; 
                break;
            case ']': 
                if(!spb->sp--||
                    spb->code[spb->t[spb->t[spb->s[spb->sp]]=spb->q]=spb->s[spb->sp]]!='[') 
                    e(spb, 5); 
                break;
        }
    }
    if(spb->sp) e(spb, spb->code[spb->s[--spb->sp]]=='['?4:6);
    for(spb->q=0;spb->q<=USHRT_MAX;spb->q++) spb->ptable[spb->q]=-1;
}

static unsigned short pop(runt_vm *vm)
{
    runt_uint rc;
    runt_stacklet *s;
    rc = runt_ppop(vm, &s);
    if(rc != RUNT_OK) {
        return 0;
    } else {
        return s->f;
    }
}

static int step(spigot_pbrain *spb)
{
        switch(spb->code[spb->q]){
            case '+': spb->a[spb->p]++; return 2;
            case '-': spb->a[spb->p]--; return 3;
            case '<': if(--spb->p<0) e(spb, 3); return 4;
            case '>': if(++spb->p>=SIZE) e(spb, 3); return 5;
            case ',': 
                runt_cell_id_exec(spb->vm, spb->fid);
                spb->a[spb->p]=pop(spb->vm); 
                return 0;

            case '.': return 1;
            case '[': if(!spb->a[spb->p]) spb->q=spb->t[spb->q]; return 6;
            case ']': if(spb->a[spb->p]) spb->q=spb->t[spb->q]; return 7;
            case '(': spb->ptable[spb->a[spb->p]]=spb->q; spb->q=spb->t[spb->q]; break;
            case ')': spb->q=spb->s[--spb->sp]; break;
            case ':': spb->s[spb->sp++]=spb->q; 
              if((spb->q=spb->ptable[spb->a[spb->p]])<0) e(spb, 2); 
              break;
        }
        return -1;
}

static void spigot_step(void *ud)
{
    spigot_pbrain *spb;
    int s;

    s = -1;
    spb = ud;
    if(spb->play == 0) {
        *spb->out = 0;
        return;
    }

    while(s == -1) {
        if(spb->q >= spb->length) {
            *spb->out = 0;
            return;
        }
        spb->curpos = spb->q;
        s = step(spb);
        spb->q++;
    }
    *spb->out = s;
}

char * spigot_get_code(spigot_pbrain *spb)
{
    return spb->code;
}

int spigot_get_length(spigot_pbrain *spb)
{
    return spb->length;
}

int spigot_get_pos(spigot_pbrain *spb)
{
    return spb->curpos;
}

spigot_pbrain * spigot_pbrain_new()
{
    return malloc(sizeof(spigot_pbrain));
}

void spigot_pbrain_free(void *ud)
{
    spigot_pbrain *spb = ud;
    free(spb);
}

static void spigot_toggle_playback(void *ud)
{
    spigot_pbrain *spb;
    spb = ud;
    if(spb->play) {
        spb->play = 0;
    } else {
        spb->play = 1;
    }
}

static void spigot_move_left(void *ud)
{
    spigot_pbrain *spb;

    spb = ud;
    if(spb->q != 0) {
        /* 
        spb->q--;
        spb->curpos = spb->q;
        */
        spb->curpos--;
        spb->q = spb->curpos;
    } 
}

static void spigot_move_right(void *ud)
{
    spigot_pbrain *spb;

    spb = ud;
    /*
    spb->q++;
    spb->curpos = spb->q;
    */
    spb->curpos++;
    spb->q = spb->curpos;
}

static void spigot_move_up(void *ud)
{
    int val;
    spigot_pbrain *spb;
    spb = ud;

    val = spb->curpos - 12;

    if(val >= 0) {
        spb->curpos = val;
    }
}

static void spigot_move_down(void *ud)
{
    int val;
    spigot_pbrain *spb;

    spb = ud;

    val = spb->curpos + 12;
    if(val <= 96) {
        spb->curpos = val;
    }
}

static void pbrain_draw(spigot_graphics *gfx, void *ud)
{
    spigot_pbrain *pbrain;
    spigot_color fg;
    spigot_color bg;
    int pos;

    pbrain = ud;

    pos = spigot_get_pos(pbrain);

    spigot_color_rgb_hex(&fg, 0x84de02);
    spigot_color_rgb_hex(&bg, 0x000000);

    spigot_draw_box(gfx, &bg, pbrain->prev);
    spigot_draw_box(gfx, &fg, pos);
    pbrain->prev = pos;
}

static void parse_code(spigot_graphics *gfx, void *ud)
{
    spigot_color clr;
    const char *code;
    spigot_pbrain *spb;
    int x_pos, y_pos;
    int len;
    int s;
    int off;

    spb = ud;
    len = spigot_get_length(spb);
    code = spigot_get_code(spb);

    x_pos= 0;
    y_pos = 0;
    s = 0;
    off = 0;

    spigot_color_rgb_hex(&clr, 0x84de02);
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
}

static void spigot_pbrain_init(void *ud)
{
    spigot_pbrain *spb = ud;

    spb->q = 0;
}

void spigot_pbrain_state(plumber_data *pd, runt_vm *vm, spigot_state *state)
{
    spigot_pbrain *spb;
    state->up = spigot_move_up;
    state->down = spigot_move_down;
    state->left = spigot_move_left;
    state->right = spigot_move_right;
    state->toggle = spigot_toggle_playback;
    state->reset = spigot_reset;
    state->draw = pbrain_draw;
    state->gfx_init = parse_code;
    state->free = spigot_pbrain_free;
    state->step = spigot_step;
    state->init = spigot_pbrain_init;

    spb = spigot_pbrain_new();
    spb->fid = 0;
    state->ud = spb;
    state->type = SPIGOT_PBRAIN;
    spb->vm = vm;
}

void spigot_pbrain_string(spigot_state *state, const char *str)
{
    spigot_pbrain *spb;

    spb = state->ud;
    spb->length = strlen(str);
    memset(spb->code, 0, sizeof(char) * SIZE);
    strncpy(spb->code, str, spb->length);
    init(spb);
}

void spigot_pbrain_bind(plumber_data *pd, spigot_state *state, const char *var)
{
    spigot_pbrain *spb;
    spb = state->ud;
    plumber_create_var(pd, var, &spb->out);
}

void spigot_pbrain_id(spigot_state *state, runt_uint id)
{
    spigot_pbrain *spb;
    spb = state->ud;
    spb->fid = id;
}

static runt_int rproc_pbrain_input(runt_vm *vm, runt_ptr p)
{
    runt_uint rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_state *state;
    spigot_pbrain *spb;

    rsd= runt_to_cptr(p);

    if(!rsd->loaded) {
        runt_print(vm, "State not loaded.\n");
        return RUNT_NOT_OK;
    }

    state = rsd->state;

    if(state->type != SPIGOT_PBRAIN) {
        runt_print(vm, "Spigot type is not pbrain.\n");
        return RUNT_NOT_OK;
    }

    spb = state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);

    spb->fid = s->f;

    return RUNT_OK;
}


int spigot_pbrain_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "pbrain_input", 12, rproc_pbrain_input);
    return runt_is_alive(vm);
}
