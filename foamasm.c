/* assembler for foamvm */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLABELS 4500
#define MAXSTRING 65536
#define MEMORYSZ 128*1024
#define MAXUNRES 4500
#define MAXIFS 128
#define MAXBACK 128
#define MAXAHEAD 1024

#define DUP 0x04000000
#define POP 0x02000000
#define IND 0x01000000

struct label {
    char *name;
    int addr;
};

struct unres {
    char *name;
    int addr;
    char lit;
};

enum { IMM, LIT, MEM };

const char *insStrs[] = {
    "call", "#", "jmp", "next",
    "jz", "jnz", "ret", "i",
    ">r", "r>", "drop", "pick",
    "+", "-", "and", "or",
    "xor", "*", "/", "mod",
    ">", "<", "!", "@",
    "swap", "#", "#", "#",
    "#", "#", "#", "halt",
};

char stringBuf[MAXSTRING];
char *nextString = stringBuf;
struct label labels[MAXLABELS];
int nlabels = 0;
struct unres unres[MAXUNRES];
int nunres = 0;

int memory[MEMORYSZ];
int here=0, org=0;

int ifs[MAXIFS];
int nifs = 0;
int back[MAXBACK];
int nback = 0;
int ahead[MAXAHEAD];
int nahead = 0;

void getNext(char *buf, int *line, FILE *fp) {
    char c;
    do {
        if((c = fgetc(fp)) == 10) ++*line;
    } while(c <= ' ' && !feof(fp));
    if(c <= ' ') { *buf = 0; return; }
    do { *(buf++) = c; c = fgetc(fp); } while(c > ' ');
    *buf = 0;
    if(c == '\n') ++*line;
}

int labelAddr(char *name, int *a) {
    int i;
    for(i = 0; i < nlabels; i++)
        if(!strcmp(labels[i].name, name)) { *a = labels[i].addr; return 1; }
    return 0;
}

void err(const char *filename, int line, const char *e) {
    printf("error at %s:%d: %s\n", filename, line, e);
    exit(1);
}

char *addName(char *name) {
    char *s;
    for(s = stringBuf; s < nextString; s += strlen(s)+1)
        if(!strcmp(name, s)) return s;
    s = nextString;
    strcpy(nextString, name);
    nextString += strlen(nextString)+1;
    return s;
}

int hex(char *s, int *n, char neg) {
    do {
        *n <<= 4;
        if(*s >= '0' && *s <= '9') *n += *s-'0';
        else if(*s >= 'a' && *s <= 'f') *n += *s-'a'+10;
        else if(*s >= 'A' && *s <= 'F') *n += *s-'A'+10;
    } while(*++s);
    if(neg) *n = -*n;
    return 1;
}

int bin(char *s, int *n, char neg) {
    do {
        *n <<= 1;
        if(*s == '1') ++*n;
        else if(*s != '0') return 0;
    } while(*++s);
    if(neg) *n = -*n;
    return 1;
}

int number(char *s, int *n) {
    char neg;
    *n = 0;
    if(*s == '-') { s++; neg = 1; } else neg = 0;
    if(*s == '$') return hex(s+1, n, neg);
    if(*s == '%') return bin(s+1, n, neg);
    do {
        *n *= 10;
        if(*s >= '0' && *s <= '9') *n += *s-'0';
        else return 0;
    } while(*++s);
    if(neg) *n = -*n;
    return 1;
}

int ins(char *s) {
    int i;
    for(i = 0; i < 32; i++)
        if(!strcmp(insStrs[i], s)) return (i<<27)|POP;
    return -1;
}

int eval(const char *filename, int *line, FILE *fp, char *buf, char type) {
    int a, b;
    char o;
    if(!strcmp(buf, "[")) {
        getNext(buf, line, fp);
        a = eval(filename, line, fp, buf, IMM);
        for(;;) {
            getNext(buf, line, fp);
            if(!strcmp(buf, "]")) break;
            o = *buf;
            getNext(buf, line, fp);
            b = eval(filename, line, fp, buf, IMM);
            switch(o) {
            case '+': a += b; break;
            case '-': a -= b; break;
            case '*': a *= b; break;
            case '/': a /= b; break;
            case '%': a %= b; break;
            case '&': a &= b; break;
            case '|': a |= b; break;
            case '^': a ^= b; break;
            default: err(filename, *line, "expected operator or ]");
            }
        }
        return a;
    }
    if(!strcmp(buf, "$")) return org;
    if(*buf == '"') return buf[1];
    if(number(buf, &a)) return a;
    if(labelAddr(buf, &a)) return a;
    if((a = ins(buf)) != -1) return a;
    if(!strcmp(buf, "<-")) {
        if(!nback) err(filename, *line, "expected { before <-");
        return back[nback-1];
    }
    if(!strcmp(buf, "->")) {
        if(type == IMM)
            err(filename, *line, "cannot use -> as an immediate value");
        if(!nback) err(filename, *line, "expected { before ->");
        ahead[nahead++] = here-(type==LIT);
        return 0;
    }
    if(type == MEM) unres[nunres].lit = 0;
    else if(type == LIT) unres[nunres].lit = 1;
    else err(filename, *line, "failed to evaluate label");
    unres[nunres].name = addName(buf);
    unres[nunres++].addr = here-(type==LIT);
    return 0;
}

void asmFile(const char *filename);

void asmNext(const char *filename, int *line, char *buf, FILE *fp) {
    int i;
    getNext(buf, line, fp);
    if(*buf == 0) return;
    if(*buf == '"') {
        for(i = 1; buf[i]; i++) { memory[here++] = buf[i]; org++; }
    } else if(*buf == ':') {
        if(!buf[1]) err(filename, *line, "empty name for label");
        if(labelAddr(buf+1, &i)) err(filename, *line, "duplicate label");
        labels[nlabels].name = addName(buf+1);
        labels[nlabels++].addr = org;
    } else if(*buf == '=') {
        org = eval(filename, &*line, fp, buf+1, IMM);
    } else if(*buf == '#') {
        here++;
        memory[here-1] = 0x08000000
                | eval(filename, line, fp, buf+1, LIT) & 0xffffff;
        org++;
    } else if(*buf == '|') {
        asmFile(buf+1);
    } else if(*buf == '+' && buf[1]) {
        here++;
        org++;
        memory[here-1] = 0x0c<<27
                | eval(filename, line, fp, buf+1, LIT) & 0xffffff;
    } else if(*buf == '!' && buf[1]) {
        here++;
        org++;
        memory[here-1] = 0x16<<27
                | eval(filename, line, fp, buf+1, LIT) & 0xffffff;
    } else if(*buf == '@' && buf[1]) {
        here++;
        org++;
        memory[here-1] = 0x17<<27
                | eval(filename, line, fp, buf+1, LIT) & 0xffffff;
    } else if(!strcmp(buf, "(")) {
        do getNext(buf, line, fp);
        while(strcmp(buf, ")") && !feof(fp));
    } else if(!strcmp(buf, "{")) {
        back[nback++] = org;
        ahead[nahead++] = 0;
    } else if(!strcmp(buf, "}")) {
        if(!nback) err(filename, *line, "expected { before }");
        nback--;
        while(ahead[--nahead]) memory[ahead[nahead]] |= org;
    } else if(!strcmp(buf, "^")) {
        memory[here-1] |= IND;
    } else if(!strcmp(buf, "if")) {
        ifs[nifs++] = here;
        memory[here++] = 4<<27;
        org++;
    } else if(!strcmp(buf, "0if")) {
        ifs[nifs++] = here;
        memory[here++] = 5<<27;
        org++;
    } else if(!strcmp(buf, "else")) {
        if(!nifs) err(filename, *line, "expected if before else");
        memory[here++] = 2<<27;
        org++;
        memory[ifs[nifs-1]] |= org;
        ifs[nifs-1] = here-1;
    } else if(!strcmp(buf, "then")) {
        if(!nifs) err(filename, *line, "expected if before then");
        memory[ifs[--nifs]] |= org;
    } else if(!strcmp(buf, "dup")) {
        i = here;
        asmNext(filename, line, buf, fp);
        if(i == here) err(filename, *line, "expected instruction after dup");
        memory[here-1] |= DUP;
    } else if(buf[i=(strlen(buf)-1)] == ':') {
        here++;
        org++;
        buf[i] = 0;
        memory[here-1] = eval(filename, line, fp, buf, MEM);
        getNext(buf, line, fp);
        memory[here-1] &= ~POP;
        memory[here-1] |= eval(filename, line, fp, buf, LIT)&0xffffff;
    } else if(buf[i] == '.') {
        here++;
        org++;
        buf[i] = 0;
        memory[here-1] = eval(filename, line, fp, buf, MEM);
        memory[here-1] &= ~POP;
    } else {
        memory[here] = eval(filename, line, fp, buf, MEM);
        here++;
        org++;
    }
}

void asmFile(const char *filename) {
    FILE *fp;
    int line = 1;
    char buf[256];
    fp = fopen(filename, "r");
    if(!fp) {
        printf("failed to open %s for assembling\n", filename);
        exit(1);
    }
    while(!feof(fp)) asmNext(filename, &line, buf, fp);
    fclose(fp);
}

void saveFile(const char *filename) {
    FILE *fp;
    int i, a;
    for(i = 0; i < nunres; i++) {
        if(!labelAddr(unres[i].name, &a)) {
            printf("failed to resolve %s\n", unres[i].name);
            exit(1);
        }
        if(unres[i].lit) a &= 0xffffff;
        memory[unres[i].addr] |= a;
    }
    fp = fopen(filename, "wb");
    if(!fp) {
        printf("failed to open %s for output\n", filename);
        exit(1);
    }
    fwrite(memory, 4, here, fp);
    fclose(fp);
}

int main(int argc, char **args) {
    int i;
    if(argc < 3) {
        printf("usage: %s <file1,file2,...> <out>\n", args[0]);
        return 1;
    }
    for(i = 1; i < argc-1; i++)
        asmFile(args[i]);
    saveFile(args[argc-1]);
    for(i = 0; i < nlabels; i++)
        printf("\t%.6X %s\n", labels[i].addr, labels[i].name);
    printf("%d words (%d bytes) total\n", here, here*4);
}
