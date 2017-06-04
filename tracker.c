#include <soundpipe.h>
#include <sporth.h>
#include <stdlib.h>

#include "spigot.h"
#include "tracker_assets.h"

#define PATSIZE 64
#define NCHAN 5
#define MAX_SEQUENCES 64
#define MAX_PAGES 64 
#define NROWS 19

typedef struct {
    int note;
    int op;
} tracker_note;

typedef struct {
    tracker_note notes[PATSIZE * NCHAN];
} tracker_page; 

typedef struct {
    spigot_color background;
    spigot_color foreground;
    spigot_color shade;
    spigot_color row_selected;
    spigot_color text_selected;
    spigot_color column_selected;

    tracker_page pages[MAX_PAGES];
    int seq[MAX_SEQUENCES];
    int offset;
    int chan;
    int page;
    int row;
    int column;
    SPFLOAT isplaying;
    int isstarted;
    SPFLOAT slice[NCHAN];
    SPFLOAT gates[NCHAN];
} spigot_tracker;

static void init_note(tracker_note *note)
{
    note->note = -1;
    note->op = 0;
}

static void init_page(tracker_page *page)
{
    int i;

    for(i = 0; i < PATSIZE * NCHAN; i++) {
        init_note(&page->notes[i]);
    }
}

static void init_sequence_data(spigot_tracker *st)
{
    int i;

    for(i = 0; i < MAX_PAGES; i++) {
        init_page(&st->pages[i]);
    }
}

static void init_tracker(void *ud)
{
    spigot_tracker *st;
    int i;

    st = ud;

    /* page offset (used in drawing) */
    st->offset = 0;
    st->chan = 0;
    st->page = 0;
    st->row = 0;
    st->column = 0;
    st->isplaying = 0;
    
    for(i = 0; i < NCHAN; i++) {
        st->slice[i] = 0;
        st->gates[i] = 0;
    }
}

static void insert_note(spigot_tracker *t, int pos, int note)
{
    t->pages[t->page].notes[pos + PATSIZE * t->chan].note = note;
}

static void draw_square_open(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{
    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            4, 4, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 10 * 8);
}

static void draw_square_closed(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{
    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            4, 4, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 11 * 8);
}

static void draw_arrow_left(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{
    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH);
}

static void draw_arrow_right(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{

    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 16);
}

static void draw_arrow_up(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{

    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 12 * 8);
}

static void draw_arrow_down(spigot_graphics *gfx, 
        spigot_color *c, int x, int y)
{

    spigot_draw_glyph(gfx, 
            c, 
            x, y, 
            16, 16, 
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 14 * 8);
}

static void draw_number(spigot_graphics *gfx, spigot_color *clr,
        int x, int y, int n) {

        spigot_draw_glyph(gfx, clr, 
                x, y, 
                4, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + 4 * (n / 10));

        spigot_draw_glyph(gfx, clr, 
                x + 4, y, 
                4, 5, 
                IMG_TRACKER_ASSETS_WIDTH,
                tracker_assets + 4 * (n % 10));
}

static void draw_note(spigot_graphics *gfx, spigot_color *clr, 
        int chan, int pos,
        int note,
        int op,
        int oct,
        int off) {

        int x, y;

        int note_off;
        int op_off;
        int oct_off;
        x = 26 + 32 * chan;
        y = 18 + 8 * (pos - off);

        if(note >= 0) {

            note_off = 64 + 4 * note;
            op_off = 48 + op * 5;
            oct_off = oct * 4;

            spigot_draw_glyph(gfx, clr, 
                    x, y, 
                    4, 5, 
                    IMG_TRACKER_ASSETS_WIDTH,
                    tracker_assets + note_off);
            
            spigot_draw_glyph(gfx, clr, 
                    x + 4, y, 
                    5, 5, 
                    IMG_TRACKER_ASSETS_WIDTH,
                    tracker_assets + op_off);
            
            spigot_draw_glyph(gfx, clr, 
                    x + 10, y, 
                    4, 5, 
                    IMG_TRACKER_ASSETS_WIDTH,
                    tracker_assets + oct_off);
        } else if(note == -2) { 
            spigot_draw_glyph(gfx, clr, 
                    x - 3, y - 1, 
                    19, 5, 
                    IMG_TRACKER_ASSETS_WIDTH,
                    tracker_assets + 
                    IMG_TRACKER_ASSETS_WIDTH * 24);
        } else {
            spigot_draw_glyph(gfx, clr, 
                    x - 3, y - 1, 
                    19, 5, 
                    IMG_TRACKER_ASSETS_WIDTH,
                    tracker_assets + 12 * 8);
        }
}

static void note_to_args(tracker_note *note, int *n, int *op, int *oct)
{
    int step;

    if(note->note < 0) {
        *n = note->note;
        *op = 0;
        *oct = 0;
    }

    step = note->note % 12;
    *oct = (note->note / 12) - 1;
    *op = 0;

    switch(step) {
        case 0: /* C */
            *n = 2;
            *op = 2;
            break;
        case 1: /* C# */
            *n = 2;
            *op = 0;
            break;
        case 2: /* D */
            *n = 3;
            *op = 2;
            break;
        case 3: /* D# */
            *n = 3;
            *op = 0;
            break;
        case 4: /* E */
            *n = 4;
            *op = 2;
            break;
        case 5: /* F */
            *n = 5;
            *op = 2;
            break; 
        case 6: /* F# */
            *n = 5;
            *op = 0;
            break;
        case 7: /* G */
            *n = 6;
            *op = 2;
            break;
        case 8: /* G# */
            *n = 6;
            *op = 0;
            break;
        case 9: /* A */
            *n = 0;
            *op = 2;
            break;
        case 10: /* A# */
            *n = 0;
            *op = 0;
            break;
        case 11: /* B */
            *n = 1;
            *op = 2;
            break;
    }

}

static void draw_page(spigot_graphics *gfx, spigot_tracker *t)
{
    tracker_note *nt;
    tracker_page *pg;
    int pos;
    int n, op, oct;
    int row;
    int chan;
    int row_min;
    int row_max;

    pg = &t->pages[t->page];

    row_min = t->offset;
    row_max = t->offset + NROWS; 
    if(row_max > PATSIZE) row_max = PATSIZE;
    for(chan = 0; chan < NCHAN; chan++) {
        for(row = row_min; row < row_max; row++) {
            pos = chan * PATSIZE + row;
            nt = &pg->notes[pos];
            note_to_args(nt, &n, &op, &oct);
            draw_note(gfx, &t->foreground, chan, row, n, op, oct, t->offset);
        }
    }
}

static void draw_row_numbers(spigot_graphics *gfx, spigot_tracker *t)
{
    int i;
    int num;
    for(i = 0; i < 19; i++) {
        num = i + t->offset;
        if(num < PATSIZE) {
            draw_number(gfx, &t->foreground, 5, 18 + 8 * i, num); 
        }
    }
}


static void draw_selected_row(spigot_graphics *gfx, spigot_tracker *t)
{
    int row;
    if(t->isplaying) {
        row = (t->row < 0) ? 0 : t->row;
        row = row % NROWS;
        spigot_draw_rect(gfx, &t->row_selected, 16, 16 + row * 8, 160, 8);
    }
}

static void redraw(spigot_graphics *gfx, void *ud)
{
    spigot_tracker *t;
    int i;
    
    t = ud;
   
    /* fill background */
    spigot_draw_fill(gfx, &t->background);
   
    /* draw dark row lines */

    for(i = 0; i < 10; i++) {
        spigot_draw_rect(gfx, &t->shade, 16, 16 + 16 * i, 160, 8);
    }

    draw_selected_row(gfx, t);

    /* draw border */
    spigot_draw_hline(gfx, &t->foreground, 0, 0, 193);
    spigot_draw_hline(gfx, &t->foreground, 192, 0, 193);
    spigot_draw_vline(gfx, &t->foreground, 0, 0, 193);
    spigot_draw_vline(gfx, &t->foreground, 192, 0, 193);

    /* draw header divider */
    spigot_draw_hline(gfx, &t->foreground, 16, 0, 193);

    /* draw status bar space divider */

    spigot_draw_hline(gfx, &t->foreground, 168, 0, 193);
    spigot_draw_hline(gfx, &t->foreground, 176, 0, 193);

    /* draw channel dividers */
    spigot_draw_vline(gfx, &t->foreground, 16, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 48, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 80, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 112, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 144, 16, 152);
    spigot_draw_vline(gfx, &t->foreground, 176, 16, 152);

    /* draw pattern number dividers */

    /* draw channel numbers */

    for(i = 0; i < 18; i++) {
        spigot_draw_hline(gfx, &t->foreground, 24 + 8 * i, 0, 16);
    }

    /* 0 0 1 */
    draw_square_open(gfx, &t->foreground, 24, 10);
    draw_square_open(gfx, &t->foreground, 30, 10);
    draw_square_closed(gfx, &t->foreground, 36, 10);
    
    /* 0 1 0 */
    draw_square_open(gfx, &t->foreground, 56, 10);
    draw_square_closed(gfx, &t->foreground, 62, 10);
    draw_square_open(gfx, &t->foreground, 68, 10);
    
    /* 0 1 1 */
    draw_square_open(gfx, &t->foreground, 88, 10);
    draw_square_closed(gfx, &t->foreground, 94, 10);
    draw_square_closed(gfx, &t->foreground, 100, 10);
    
    /* 1 0 0 */
    draw_square_closed(gfx, &t->foreground, 120, 10);
    draw_square_open(gfx, &t->foreground, 126, 10);
    draw_square_open(gfx, &t->foreground, 132, 10);
    
    /* 1 0 1 */
    draw_square_closed(gfx, &t->foreground, 152, 10);
    draw_square_open(gfx, &t->foreground, 158, 10);
    draw_square_closed(gfx, &t->foreground, 164, 10);

    /* draw pattern sequencer dividers */
    for(i = 0; i < 11; i++) {
        spigot_draw_vline(gfx, &t->foreground, 16 + 16 * i, 22 * 8, 16);
    }

    draw_row_numbers(gfx, t);

    /* draw scroll bar dividers */
    spigot_draw_hline(gfx, &t->foreground, 32, 22 * 8, 16);
    spigot_draw_hline(gfx, &t->foreground, 19 * 8, 22 * 8, 16);

    /* draw scollbar arrows */
    draw_arrow_left(gfx, &t->foreground, 16, 22 * 8);
    draw_arrow_right(gfx, &t->foreground, 20 * 8, 22 * 8);
    
    draw_arrow_up(gfx, &t->foreground, 22 * 8, 2 * 8);
    draw_arrow_down(gfx, &t->foreground, 22 * 8, 19 * 8);

    /* t->pages[0].notes[1].note = 71; */
    draw_page(gfx, t);
}

static void init_tracker_gfx(spigot_graphics *gfx, void *ud)
{
    spigot_tracker *t;

    t = ud;

    spigot_color_rgb(&t->background, 130, 195, 255);
    spigot_color_rgb(&t->foreground, 0, 65, 130);
    spigot_color_rgb(&t->shade, 52, 158, 255);
    spigot_color_rgb(&t->row_selected, 255, 119, 171);
    spigot_color_rgb(&t->text_selected, 255, 255, 255);

    redraw(gfx, t);
}

static void spigot_tracker_free(void *ud)
{
    spigot_tracker *t;
    t = ud;
    free(t);
}

static int rproc_chan(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    runt_int chan;

    rsd = runt_to_cptr(p);

    if(rsd->loaded == 0) {
        runt_print(vm, "tracker_chan: state not set yet!\n");
        return RUNT_NOT_OK;
    } else if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "tracker_chan: this is not a tracker!\n");
    }

    t = rsd->state->ud;
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    chan = s->f;
    t->chan = chan;

    return RUNT_OK;
}

static tracker_page * get_current_page(spigot_tracker *t)
{
    return &t->pages[t->page];
}

static tracker_note * get_note_from_page(tracker_page *pg, int row, int col)
{
    return &pg->notes[row + col * PATSIZE];
}

static void tracker_step(void *ud)
{
    spigot_tracker *t;
    tracker_page *pg;
    tracker_note *nt;
    int i;
    t = ud;
    pg = get_current_page(t);
    if(t->isplaying) {
        if(t->isstarted == 1) {
            t->isstarted = 0;
        } else {
            t->row++;

            if(t->row == PATSIZE) {
                t->offset = 0;
                t->row = 0;
            } else if((t->row - t->offset) >= NROWS){
                t->offset += NROWS;
            }
        }

        for(i = 0; i < NCHAN; i++) {
            nt = get_note_from_page(pg, t->row, i);
            if(nt->note >= 0) {
                t->slice[i] = nt->note;
                t->gates[i] = 1.0;
            } else if (nt->note == -2) {
                t->gates[i] = 0.0;
            }
        }
    }
}

static int rproc_note(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    runt_int note;
    runt_int row;

    rsd = runt_to_cptr(p);

    if(rsd->loaded == 0) {
        runt_print(vm, "tracker_note: state not set yet!\n");
        return RUNT_NOT_OK;
    } else if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "tracker_note: this is not a tracker!\n");
    }

    t = rsd->state->ud;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    row = s->f;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    note = s->f;

    insert_note(t, row, note);
    return RUNT_OK;
}

static int rproc_noteoff(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    runt_int row;

    rsd = runt_to_cptr(p);

    if(rsd->loaded == 0) {
        runt_print(vm, "tracker_note: state not set yet!\n");
        return RUNT_NOT_OK;
    } else if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "tracker_note: this is not a tracker!\n");
    }

    t = rsd->state->ud;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    row = s->f;

    insert_note(t, row, -2);
    return RUNT_OK;
}

static int rproc_notes(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    const char *str;
    plumber_data *pd;
    sp_ftbl *ft;

    rsd = runt_to_cptr(p);
    pd = rsd->pd;

    if(rsd->loaded == 0) {
        runt_print(vm, "Please load a state first.\n");
        return RUNT_NOT_OK;
    }

    if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "State type is not a tracker.\n");
        return RUNT_NOT_OK;
    }

    t = rsd->state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    str = runt_to_string(s->p);

    sp_ftbl_bind(pd->sp, &ft, t->slice, NCHAN);
    plumber_ftmap_add(pd, str, ft);

    return RUNT_OK;
}

static int rproc_gates(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    const char *str;
    plumber_data *pd;
    sp_ftbl *ft;

    rsd = runt_to_cptr(p);
    pd = rsd->pd;

    if(rsd->loaded == 0) {
        runt_print(vm, "Please load a state first.\n");
        return RUNT_NOT_OK;
    }

    if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "State type is not a tracker.\n");
        return RUNT_NOT_OK;
    }

    t = rsd->state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    str = runt_to_string(s->p);

    sp_ftbl_bind(pd->sp, &ft, t->gates, NCHAN);
    plumber_ftmap_add(pd, str, ft);

    return RUNT_OK;
}

static int rproc_play(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    const char *str;
    plumber_data *pd;

    rsd = runt_to_cptr(p);
    pd = rsd->pd;

    if(rsd->loaded == 0) {
        runt_print(vm, "Please load a state first.\n");
        return RUNT_NOT_OK;
    }

    if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "State type is not a tracker.\n");
        return RUNT_NOT_OK;
    }
    
    t = rsd->state->ud;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    str = runt_to_string(s->p);

    plumber_ftmap_delete(pd, 0);
    plumber_ftmap_add_userdata(pd, str, &t->isplaying);
    plumber_ftmap_delete(pd, 1);

    return RUNT_OK;
}

static void toggle(void *ud)
{
    spigot_tracker *t;
    t = ud;
    if(t->isplaying) {
        t->isplaying = 0;
    } else {
        t->isplaying = 1;
        t->isstarted = 1;
        t->row = 0;
        t->offset = 0;
    }

}

int spigot_tracker_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "tracker_note", 12, rproc_note);
    spigot_word_define(vm, p, "tracker_chan", 12, rproc_chan);
    spigot_word_define(vm, p, "tracker_notes", 13, rproc_notes);
    spigot_word_define(vm, p, "tracker_noteoff", 15, rproc_noteoff);
    spigot_word_define(vm, p, "tracker_gates", 13, rproc_gates);
    spigot_word_define(vm, p, "tracker_play", 12, rproc_play);
    return runt_is_alive(vm);
}

void spigot_tracker_state(plumber_data *pd, spigot_state *state)
{
    spigot_tracker *t;
    state->gfx_init = init_tracker_gfx;
    state->free = spigot_tracker_free;
    state->init = init_tracker;
    state->ud = malloc(sizeof(spigot_tracker));
    state->draw = redraw;
    state->step = tracker_step;
    state->toggle = toggle;

    state->type = SPIGOT_TRACKER;
    t = state->ud;
    init_sequence_data(t);
}
