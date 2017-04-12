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
#define SIZE 65536
#define CA(x) case x: fprintf(stderr, "Error: " 

unsigned short a[SIZE];
int s[SIZE], sp, ptable[USHRT_MAX+1], t[SIZE], p, q, length, c;
char code[SIZE];
const char *f;
FILE *input;

void e(int i){
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
    exit(i);
}

int main(int argc, char **argv){
    const char *filename;

    filename = "test.bf";
    if(!(input = fopen(f=filename, "r"))) e(8);
    length = fread(code, 1, SIZE, input);
    fclose(input);
    for(q=0;q<length;q++){
        switch(code[q]){
            case '(': case '[': s[sp++]=q ; break;
            case ')': if(!sp--||code[s[sp]]!='(') e(7); t[s[sp]]=q; break;
            case ']': if(!sp--||code[t[t[s[sp]]=q]=s[sp]]!='[') e(5); break;
        }
    }
    if(sp) e(code[s[--sp]]=='['?4:6);
    for(q=0;q<=USHRT_MAX;q++) ptable[q]=-1;
    for(q=0;q<length;q++){
        switch(code[q]){
            case '+': a[p]++; break;
            case '-': a[p]--; break;
            case '<': if(--p<0) e(3); break;
            case '>': if(++p>=SIZE) e(3); break;
            case ',': if((c=getchar())!=EOF) a[p]=c=='\n'?10:c; break;
            case '.': putchar(a[p]==10?'\n':a[p]); break;
            case '[': if(!a[p]) q=t[q]; break;
            case ']': if(a[p]) q=t[q]; break;
            case '(': ptable[a[p]]=q; q=t[q]; break;
            case ')': q=s[--sp]; break;
            case ':': s[sp++]=q; if((q=ptable[a[p]])<0) e(2); break;
        }
    }
    exit(0);
}
