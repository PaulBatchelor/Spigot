#include <soundpipe.h>
#include <math.h>
#include <sporth.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <string.h>
#include "spigot.h"
#include "tracker_assets.h"
#include "plumbstream.h"

#define PATSIZE 64
#define NCHAN 5
#define MAX_SEQUENCES 64
#define MAX_PAGES 64 
#define NROWS 19

enum {
PLAYMODE_PAGE,
PLAYMODE_SONG,
PLAYMODE_INPLACE,
};

typedef struct {
    int note;
    int op;
} tracker_note;

typedef struct {
    tracker_note notes[PATSIZE * NCHAN];
    char active;
} tracker_page; 

typedef struct {
    runt_vm *vm;
    spigot_color background;
    spigot_color foreground;
    spigot_color shade;
    spigot_color row_selected;
    spigot_color text_selected;

    tracker_page pages[MAX_PAGES];
    int seq[MAX_SEQUENCES];
    int offset;
    int chan;
    int page;
    int row;
    int column;
    SPFLOAT _isplaying;
    SPFLOAT *isplaying;
    SPFLOAT _isrolling;
    SPFLOAT *isrolling;
    int isstarted;
    SPFLOAT slice[NCHAN];
    SPFLOAT gates[NCHAN];
    int oct;
    const char *filename;
    int loaded;
    int step;
    int nseq;
    int seqpos;
    int playmode;
    int seq_offset;
} spigot_tracker;

static void init_note(tracker_note *note)
{
    note->note = -1;
    note->op = 0;
}

static void init_page(tracker_page *page)
{
    int i;

    page->active = 0;
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
    st->row = -1;
    st->column = 0;
    *st->isrolling = 1;
    *st->isplaying = 1;
    st->isstarted = 0;
    
    for(i = 0; i < NCHAN; i++) {
        st->slice[i] = 0;
        st->gates[i] = 0;
    }
    st->oct = 4;
    st->step = 1;
    st->seqpos = 0;
    st->playmode = PLAYMODE_SONG;
    st->seq_offset = 0;
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
    spigot_color *clr;

    pg = &t->pages[t->page];
    
    row_min = t->offset;
    row_max = t->offset + NROWS; 
    if(row_max > PATSIZE) row_max = PATSIZE;
    for(chan = 0; chan < NCHAN; chan++) {
        for(row = row_min; row < row_max; row++) {
            pos = chan * PATSIZE + row;
            nt = &pg->notes[pos];
            note_to_args(nt, &n, &op, &oct);
            if(chan == t->chan && row == t->row && !*t->isplaying) {
                clr = &t->text_selected;
            } else {
                clr = &t->foreground;
            }
            draw_note(gfx, clr, chan, row, n, op, oct, t->offset);
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
    row = (t->row < 0) ? 0 : t->row;
    row = row % NROWS;
    if(*t->isplaying) {
        spigot_draw_rect(gfx, &t->row_selected, 16, 16 + row * 8, 160, 8);
    } else {
        spigot_draw_rect(gfx, &t->foreground, 16 + 
            t->chan * 32, 16 + row * 8, 32, 8);
    }
}

static void calculate_offsets(spigot_tracker *t)
{
    if(t->row == PATSIZE) {
        t->offset = 0;
    } else if((t->row - t->offset) >= NROWS){
        t->offset += NROWS;
    } else if((t->row < t->offset) &&  t->row >= 0) {
        while(t->row < t->offset) {
            t->offset -= NROWS;
        }
    }
}

static void redraw(spigot_graphics *gfx, void *ud)
{
    spigot_tracker *t;
    int i;
    float size;
    float progress;
    int off;
    
    t = ud;
  
    calculate_offsets(t);
    /* fill background */
    spigot_draw_fill(gfx, &t->background);
   

    /* draw scrollbar rect */
    spigot_draw_rect(gfx, &t->shade, 22 * 8, 32, 16, 15 * 8); 

    size = ((float)NROWS / PATSIZE) * 15 * 8;
    progress = (14 * 8)   * ((float)t->offset / PATSIZE);

    if(progress + size > 14 * 8) progress = floor((15 * 8) - size + 0.5) + 1;

    spigot_draw_rect(gfx, &t->background, 
        22 * 8, 
        32 + progress, 
        16, size); 
    

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

    /* figure out seq_offset for drawing */

    if((t->seqpos - t->seq_offset) > 7) {
        t->seq_offset++;
    } else if((t->seqpos - t->seq_offset) < 0) {
        t->seq_offset--;
    }
    
    /* draw scollbar arrows */
    if(t->seq_offset == 0) {
        draw_arrow_left(gfx, &t->shade, 16, 22 * 8);
    } else {
        draw_arrow_left(gfx, &t->foreground, 16, 22 * 8);
    }

    draw_arrow_right(gfx, &t->foreground, 20 * 8, 22 * 8);
   
    if(t->offset == 0) {
        draw_arrow_up(gfx, &t->shade, 22 * 8, 2 * 8);
    } else {
        draw_arrow_up(gfx, &t->foreground, 22 * 8, 2 * 8);
    }

    if(t->offset >= 57) {
        draw_arrow_down(gfx, &t->shade, 22 * 8, 19 * 8);
    } else {
        draw_arrow_down(gfx, &t->foreground, 22 * 8, 19 * 8);
    }

    for(i = 0; i < (t->nseq > 8 ? 8 : t->nseq); i++) {
        draw_number(gfx, &t->foreground, 32 + 5 + 16*i, 22 * 8 + 6, 
                t->seq[i + t->seq_offset]);
    }


    if((t->seqpos - t->seq_offset) > 7) {
        off = 7;
    } else {
        off = t->seqpos - t->seq_offset;
    }

    spigot_draw_glyph(gfx, &t->foreground, 
            32 + 16*off, 
            22 * 8, 
            17, 17,
            IMG_TRACKER_ASSETS_WIDTH, 
            tracker_assets + 8 * IMG_TRACKER_ASSETS_WIDTH + 7 * 8);

    /* draw scroll bar dividers */
    spigot_draw_hline(gfx, &t->foreground, 32, 22 * 8, 16);
    spigot_draw_hline(gfx, &t->foreground, 19 * 8, 22 * 8, 16);

    draw_page(gfx, t);
}

static void init_tracker_gfx(spigot_graphics *gfx, void *ud)
{
    spigot_tracker *t;

    t = ud;

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

static int rproc_page(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    runt_int page;

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
    page = s->f;
    t->page = page;

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
    if(*t->isplaying) {
        *t->isrolling = 1;
        if(t->isstarted == 1) {
            t->isstarted = 0;
        } else {
            t->row++;
            if(t->row >= PATSIZE) {
                t->row = 0;
                if(t->playmode == PLAYMODE_SONG) {
                    t->seqpos++;
                    t->seqpos %= t->nseq;
                    t->page = t->seq[t->seqpos];
                }
            }
            calculate_offsets(t);
        }

        pg = get_current_page(t);
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

static int rproc_snotes(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    const char *str;
    plumber_data *pd;
    sp_ftbl *ft;
    plumber_stream *stream;

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
    stream = runt_to_cptr(s->p);

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    str = runt_to_string(s->p);

    sp_ftbl_bind(pd->sp, &ft, t->slice, NCHAN);

    plumber_stream_append_data(pd, stream, str, strlen(str), ft, 1, PTYPE_TABLE);

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

static int rproc_sgates(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    const char *str;
    plumber_data *pd;
    sp_ftbl *ft;
    plumber_stream *stream;

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
    stream = runt_to_cptr(s->p);

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    str = runt_to_string(s->p);

    sp_ftbl_bind(pd->sp, &ft, t->gates, NCHAN);
    plumber_stream_append_data(pd, stream, str, strlen(str), ft, 1, PTYPE_TABLE);

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
    SPFLOAT *var;

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

    rc = plumber_ftmap_search_userdata(pd, str, (void **)&var);

    if(rc != PLUMBER_OK) {
        runt_print(vm, "Could not find variable %s\n", str);
        return RUNT_NOT_OK;
    }

    t->isplaying = var;

    return RUNT_OK;
}

static void return_to_zero(spigot_tracker *t)
{
    t->row = 0;
    t->chan = 0;
    t->offset = 0;
    t->page = 0;
    t->seqpos = 0;
}

static void load_tracker_file(spigot_tracker *t)
{
    if(!t->loaded) {
        runt_print(t->vm, "No filename loaded!\n");
        return;
    }
    if(access(t->filename, F_OK) != -1) {
        runt_print(t->vm, "Loading file %s\n", t->filename);
        init_sequence_data(t);
        runt_parse_file(t->vm, t->filename);
        return_to_zero(t);
    }
}

static void save_tracker_file(spigot_tracker *t)
{
    FILE *fp;
    int row, chan;
    int page;
    int seq;
    tracker_page *pg;
    tracker_note *nt;

    if(!t->loaded) {
        runt_print(t->vm, "No filename loaded!\n");
        return;
    }
    fp = fopen(t->filename, "w");

    if(fp == NULL) {
        runt_print(t->vm, "There was a problem writing to %s.\n", t->filename);
        return;
    }
   
    /* mark active pages in sequence */

    for(seq = 0; seq < t->nseq; seq++) {
        t->pages[t->seq[seq]].active = 1;
    }

    for(page = 0; page < MAX_PAGES; page++) {
        pg = &t->pages[page];
        if(pg->active == 0) continue; 
        fprintf(fp, "%d tracker_page\n", page);
        for(chan = 0; chan < NCHAN; chan++) {
            fprintf(fp, "%d tracker_chan\n", chan);
            for(row = 0; row < PATSIZE; row++) {
                nt = get_note_from_page(pg, row, chan);
                if(nt->note >= 0) {
                    fprintf(fp, "%d %d tracker_note\n", nt->note, row);
                } else if(nt->note == -2) {
                    fprintf(fp, "%d tracker_noteoff\n", row);
                }
            }
            fprintf(fp, "\n");
        }
    }
    
    /* save sequence */
    fprintf(fp, "tracker_seq\n");
    for(seq = 0; seq < t->nseq; seq++) {
        t->pages[t->seq[seq]].active = 0;
        fprintf(fp, "%d tracker_insert\n", t->seq[seq]);
    }
    
    runt_print(t->vm, "File saved to %s.\n", t->filename);

    fclose(fp);
}

static int rproc_load(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    const char *str;

    rsd = runt_to_cptr(p);

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
    /* save string onto stack... careful */
    runt_mark_set(vm);
    runt_cell_undo(vm);

    t->loaded = 1;
    t->filename = str;

    load_tracker_file(t);

    return RUNT_OK;
}

static int rproc_seq(runt_vm *vm, runt_ptr p)
{
    runt_spigot_data *rsd;
    spigot_tracker *t;

    rsd = runt_to_cptr(p);

    if(rsd->loaded == 0) {
        runt_print(vm, "Please load a state first.\n");
        return RUNT_NOT_OK;
    }

    if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "State type is not a tracker.\n");
        return RUNT_NOT_OK;
    }
    
    t = rsd->state->ud;

    t->seqpos = 0;
    t->nseq = 0;

    return RUNT_OK;
}

static int rproc_insert(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    runt_stacklet *s;
    runt_spigot_data *rsd;
    spigot_tracker *t;
    runt_int page;

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
    page = s->f;

    t->seq[t->seqpos] = page;
    t->seqpos++;
    t->nseq++;
    return RUNT_OK;
}

static void toggle_playmode(spigot_tracker *t, int playmode)
{
    int i;
    if(*t->isplaying) {
        *t->isplaying = 0;
        *t->isrolling = 0;
    } else {
        for(i = 0; i < NCHAN; i++) t->gates[i] = 0;
        *t->isplaying = 1;
        t->isstarted = 1;
        switch(playmode) {
            case PLAYMODE_SONG:
                t->seqpos = 0;
                t->page = t->seq[t->seqpos];
            case PLAYMODE_PAGE:
                t->row = 0;
                t->offset = 0;
                break;
            case PLAYMODE_INPLACE: 
                break;
            default: 
                break;
        }

        t->playmode = playmode;
    }
}

static int get_tracker_data(runt_vm *vm, runt_ptr p, spigot_tracker **t)
{
    runt_spigot_data *rsd;

    rsd = runt_to_cptr(p);

    if(rsd->loaded == 0) {
        runt_print(vm, "tracker_note: state not set yet!\n");
        return RUNT_NOT_OK;
    } else if(rsd->state->type != SPIGOT_TRACKER) {
        runt_print(vm, "tracker_note: this is not a tracker!\n");
        return RUNT_NOT_OK;
    }

    *t = rsd->state->ud;
    return RUNT_OK;
}

static int set_color(runt_vm *vm, runt_ptr p, spigot_color *c)
{
    runt_int rc;
    runt_stacklet *s;
    runt_int r, g, b;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    b = s->f;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    g = s->f;
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    r = s->f;

    c->r = r;
    c->g = g;
    c->b = b;
    return RUNT_OK;
}

static int rproc_bgcolor(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    spigot_tracker *t;

    rc = get_tracker_data(vm, p, &t);
    RUNT_ERROR_CHECK(rc);
   
    return set_color(vm, p, &t->background);
}

static int rproc_fgcolor(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    spigot_tracker *t;

    rc = get_tracker_data(vm, p, &t);
    RUNT_ERROR_CHECK(rc);
   
    return set_color(vm, p, &t->foreground);
}

static int rproc_shade(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    spigot_tracker *t;

    rc = get_tracker_data(vm, p, &t);
    RUNT_ERROR_CHECK(rc);
   
    return set_color(vm, p, &t->shade);
}

static int rproc_row(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    spigot_tracker *t;

    rc = get_tracker_data(vm, p, &t);
    RUNT_ERROR_CHECK(rc);
   
    return set_color(vm, p, &t->row_selected);
}

static int rproc_text(runt_vm *vm, runt_ptr p)
{
    runt_int rc;
    spigot_tracker *t;

    rc = get_tracker_data(vm, p, &t);
    RUNT_ERROR_CHECK(rc);
   
    return set_color(vm, p, &t->text_selected);
}

static void toggle(void *ud)
{
    spigot_tracker *t;
    t = ud;
    toggle_playmode(t, PLAYMODE_PAGE);
}

static void left(void *ud)
{
    spigot_tracker *t;
    t = ud;
    t->chan = t->chan - 1;

    if(t->chan < 0) t->chan = NCHAN - 1;
}

static void right(void *ud)
{
    spigot_tracker *t;
    t = ud;
    t->chan = (t->chan + 1) % NCHAN;
}

static void up(void *ud)
{
    spigot_tracker *t;
    t = ud;
    t->row -= t->step;

    if(t->row < 0) t->row = 0;
}

static void down(void *ud)
{
    spigot_tracker *t;
    t = ud;
    t->row = (t->row + t->step);

    if(t->row >= PATSIZE) t->row = PATSIZE - 1;
}

static void delete_sequence(spigot_tracker *t)
{
    int i;

    if(t->nseq == 1) return ;
    for(i = t->seqpos; i < t->nseq; i++) {
        t->seq[i] = t->seq[i + 1];
    }
    t->nseq--;
    if(t->seqpos >= t->nseq - 1) {
        t->seqpos = t->nseq - 1;
    }
    t->page = t->seq[t->seqpos];
}

static void insert_sequence(spigot_tracker *t)
{
    int i;
    for(i = t->nseq; i > t->seqpos + 1; i--) {
        t->seq[i] = t->seq[i - 1];
    }
    t->nseq++;
    t->seqpos++;
    t->page = t->seq[t->seqpos] = t->seq[t->seqpos - 1];
}

static void keyhandler(spigot_graphics *gfx, void *ud, 
        int key, int scancode, int action, int mods)
{
    spigot_tracker *t;
    t = ud;
    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        if(mods == GLFW_MOD_SHIFT) {
            switch(key) {
                case GLFW_KEY_1:
                    insert_note(t, t->row, -2);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_EQUAL:
                    t->step += 1;
                    break;
                case GLFW_KEY_L:
                    t->seqpos += 1;
                    if(t->seqpos >= t->nseq) t->seqpos = t->nseq - 1;
                    t->page = t->seq[t->seqpos];
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_H:
                    t->seqpos -= 1;
                    if(t->seqpos < 0) t->seqpos = 0;
                    t->page = t->seq[t->seqpos];
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_K:
                    t->page++;
                    if(t->page >= MAX_PAGES) t->page = 0;
                    t->seq[t->seqpos] = t->page;
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_J:
                    t->page--;
                    if(t->page < 0) t->page = MAX_PAGES - 1;
                    t->seq[t->seqpos] = t->page;
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_X:
                    delete_sequence(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_SPACE:
                    toggle_playmode(t, PLAYMODE_INPLACE);
                    break;
                case GLFW_KEY_N:
                    insert_sequence(t);
                    spigot_gfx_step(gfx);
                    break;
            }
        } else if(mods == GLFW_MOD_ALT) {

        }else {
            switch(key) {
                case GLFW_KEY_Q:
                    insert_note(t, t->row, 12 * (t->oct + 1));
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_2:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 1);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_W:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 2);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_3:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 3);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_E:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 4);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_R:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 5);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_5:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 6);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_T:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 7);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_6:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 8);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_Y:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 9);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_7:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 10);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_U:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 11);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_I:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 12);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_9:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 13);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_O:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 14);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_0:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 15);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_P:
                    insert_note(t, t->row, 12 * (t->oct + 1) + 16);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_X:
                    insert_note(t, t->row, -1);
                    down(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_F:
                    load_tracker_file(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_S:
                    save_tracker_file(t);
                    spigot_gfx_step(gfx);
                    break;
                case GLFW_KEY_COMMA:
                    t->oct -= 1;
                    if(t->oct < 1) t->oct = 1;
                    break;
                case GLFW_KEY_PERIOD:
                    t->oct += 1;
                    if(t->oct > 7) t->oct = 7;
                    break;
                case GLFW_KEY_MINUS:
                    t->step -= 1;
                    if(t->step <= 0) t->step = 0;
                    break;
                case GLFW_KEY_ENTER:
                    toggle_playmode(t, PLAYMODE_SONG);
                    break;
                default:
                    break;
            }
        }
    }
}

int spigot_tracker_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "tracker_note", 12, rproc_note);
    spigot_word_define(vm, p, "tracker_chan", 12, rproc_chan);
    spigot_word_define(vm, p, "tracker_page", 12, rproc_page);
    spigot_word_define(vm, p, "tracker_notes", 13, rproc_notes);
    spigot_word_define(vm, p, "tracker_noteoff", 15, rproc_noteoff);
    spigot_word_define(vm, p, "tracker_gates", 13, rproc_gates);
    spigot_word_define(vm, p, "tracker_snotes", 14, rproc_snotes);
    spigot_word_define(vm, p, "tracker_sgates", 14, rproc_sgates);
    spigot_word_define(vm, p, "tracker_play", 12, rproc_play);
    spigot_word_define(vm, p, "tracker_open", 12, rproc_load);
    spigot_word_define(vm, p, "tracker_seq", 11, rproc_seq);
    spigot_word_define(vm, p, "tracker_insert", 14, rproc_insert);
    spigot_word_define(vm, p, "tracker_bgcolor", 15, rproc_bgcolor);
    spigot_word_define(vm, p, "tracker_fgcolor", 15, rproc_fgcolor);
    spigot_word_define(vm, p, "tracker_shade", 13, rproc_shade);
    spigot_word_define(vm, p, "tracker_row", 11, rproc_row);
    spigot_word_define(vm, p, "tracker_text", 12, rproc_text);
    return runt_is_alive(vm);
}


void spigot_tracker_state(plumber_data *pd, runt_vm *vm, spigot_state *state)
{
    spigot_tracker *t;
    state->gfx_init = init_tracker_gfx;
    state->free = spigot_tracker_free;
    state->init = init_tracker;
    state->ud = malloc(sizeof(spigot_tracker));
    state->draw = redraw;
    state->step = tracker_step;
    state->toggle = toggle;
    state->right = right;
    state->left = left;
    state->up = up;
    state->down = down;
    state->key = keyhandler;

    state->type = SPIGOT_TRACKER;
    t = state->ud;
    init_sequence_data(t);
    t->vm = vm;
    t->nseq = 1;
    spigot_color_rgb(&t->background, 130, 195, 255);
    spigot_color_rgb(&t->foreground, 0, 65, 130);
    spigot_color_rgb(&t->shade, 52, 158, 255);
    spigot_color_rgb(&t->row_selected, 255, 119, 171);
    spigot_color_rgb(&t->text_selected, 255, 255, 255);
    t->isplaying = &t->_isplaying;
    t->isrolling = &t->_isrolling;
}
