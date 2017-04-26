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

#include "spigot.h"

#define SIZE 65536
#define CA(x) case x: fprintf(stderr, "Error: " 

const char *f;

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
};

static void e(spigot_pbrain *spb, int i){
    switch(i){
        CA(2) "call to undefined procedure (%hu)", spb->a[spb->p]); break;
        CA(3) "pointer too far %s", spb->p>0?"right":"left"); break;
        CA(4) "unmatched '[' at byte %d of %s", spb->s[spb->sp], f); 
            break;
        CA(5) "unmatched ']' at byte %d of %s", spb->q, f); 
            break;
        CA(6) "unmatched '(' at byte %d of %s", spb->s[spb->sp], f); 
            break;
        CA(7) "unmatched ')' at byte %d of %s", spb->q, f); 
            break;
        CA(8) "can't open %s", f); 
            break;
    }
    printf(".\n");
}

static void init(spigot_pbrain *spb, const char *str, int len)
{
    memset(spb->a, 0, sizeof(short) * SIZE);
    memset(spb->s, 0, sizeof(int) * SIZE);
    memset(spb->t, 0, sizeof(int) * SIZE);
    memset(spb->code, 0, sizeof(char) * SIZE);
    memset(spb->ptable, 0, sizeof(int) * (USHRT_MAX + 1));
    /*
    if(!(input = fopen(f=filename, "r"))) e(8);
    length = fread(code, 1, SIZE, input);
    fclose(input);
    */
    strncpy(spb->code, str, len);
    spb->length = len;
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
    spb->curpos = 0;
    spb->q = 0;
}

static int step(spigot_pbrain *spb)
{
        switch(spb->code[spb->q]){
            case '+': spb->a[spb->p]++; return 2;
            case '-': spb->a[spb->p]--; return 3;
            case '<': if(--spb->p<0) e(spb, 3); return 4;
            case '>': if(++spb->p>=SIZE) e(spb, 3); return 5;
            case ',': spb->a[spb->p]=spb->in; return 0;
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

int spigot_init(spigot_pbrain *spb, const char *str)
{
    init(spb, str, strlen(str));
    spb->q = 0;
    return 0;
}

int spigot_step(spigot_pbrain *spb)
{
    int s = -1;

    while(s == -1) {
        if(spb->q >= spb->length) return 0;
        spb->curpos = spb->q;
        s = step(spb);
        spb->q++;
    }
    return s;
}

int spigot_constant(spigot_pbrain *spb, unsigned short val)
{
    spb->in = val;
    return 0;
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

void spigot_pbrain_free(spigot_pbrain *spb)
{
    free(spb);
}
