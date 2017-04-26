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

unsigned short a[SIZE];
int s[SIZE], sp, ptable[USHRT_MAX+1], t[SIZE], p, q, length, c;
int curpos;
char code[SIZE];
const char *f;
unsigned short in;

struct spigot_pbrain {

};

static void e(int i){
    switch(i){
        CA(2) "call to undefined procedure (%hu)", a[p]); break;
        CA(3) "pointer too far %s", p>0?"right":"left"); break;
        CA(4) "unmatched '[' at byte %d of %s", s[sp], f); break;
        CA(5) "unmatched ']' at byte %d of %s", q, f); break;
        CA(6) "unmatched '(' at byte %d of %s", s[sp], f); break;
        CA(7) "unmatched ')' at byte %d of %s", q, f); break;
        CA(8) "can't open %s", f); break;
    }
    printf(".\n");
}

static void init(spigot_pbrain *spb, const char *str, int len)
{
    memset(a, 0, sizeof(short) * SIZE);
    memset(s, 0, sizeof(int) * SIZE);
    memset(t, 0, sizeof(int) * SIZE);
    memset(code, 0, sizeof(char) * SIZE);
    memset(ptable, 0, sizeof(int) * (USHRT_MAX + 1));
    /*
    if(!(input = fopen(f=filename, "r"))) e(8);
    length = fread(code, 1, SIZE, input);
    fclose(input);
    */
    strncpy(code, str, len);
    length = len;
    for(q=0;q<length;q++){
        switch(code[q]){
            case '(': case '[': s[sp++]=q ; break;
            case ')': if(!sp--||code[s[sp]]!='(') e(7); t[s[sp]]=q; break;
            case ']': if(!sp--||code[t[t[s[sp]]=q]=s[sp]]!='[') e(5); break;
        }
    }
    if(sp) e(code[s[--sp]]=='['?4:6);
    for(q=0;q<=USHRT_MAX;q++) ptable[q]=-1;
    curpos = 0;
    q = 0;
}

static int step()
{
        switch(code[q]){
            case '+': a[p]++; return 2;
            case '-': a[p]--; return 3;
            case '<': if(--p<0) e(3); return 4;
            case '>': if(++p>=SIZE) e(3); return 5;
            case ',': a[p]=in; return 0;
            case '.': return 1;
            case '[': if(!a[p]) q=t[q]; return 6;
            case ']': if(a[p]) q=t[q]; return 7;
            case '(': ptable[a[p]]=q; q=t[q]; break;
            case ')': q=s[--sp]; break;
            case ':': s[sp++]=q; if((q=ptable[a[p]])<0) e(2); break;
        }
        return -1;
}

int spigot_init(spigot_pbrain *spb, const char *str)
{
    init(spb, str, strlen(str));
    q = 0;
    return 0;
}

int spigot_step(spigot_pbrain *spb)
{
    int s = -1;

        
    while(s == -1) {
        if(q >= length) return 0;
        curpos = q;
        s = step();
        q++;
    }
    return s;
}

int spigot_constant(spigot_pbrain *spb, unsigned short val)
{
    in = val;
    return 0;
}

char * spigot_get_code(spigot_pbrain *spb)
{
    return code;
}

int spigot_get_length(spigot_pbrain *spb)
{
    return length;
}

int spigot_get_pos(spigot_pbrain *spb)
{
    return curpos;
}

spigot_pbrain * spigot_pbrain_new()
{
    return malloc(sizeof(spigot_pbrain));
}

void spigot_pbrain_free(spigot_pbrain *spb)
{
    free(spb);
}
