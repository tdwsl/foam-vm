#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    CALL=0, LIT, JMP, NEXT,
    JZ, JNZ, RET, I,
    RPH, RPL, DROP, PICK,
    ADD, SUB, AND, OR,
    XOR, MUL, DIV, MOD,
    GTN, LTN, STW, LDW,
    SWAP,
    HALT=0x1f,
};

#define DUP  0x04000000
#define POP  0x02000000
#define IND  0x01000000

const char *insStrs[] = {
    "CALL", "LIT", "JMP", "NEXT",
    "JZ", "JNZ", "RET", "I",
    "RPH", "RPL", "DROP", "PICK",
    "ADD", "SUB", "AND", "OR",
    "XOR", "MUL", "DIV", "MOD",
    "GTN", "LTN", "STW", "LDW",
    "SWAP", "?", "?", "?",
    "?", "?", "?", "HALT",
};

#define BASE 0x00010000
#define CHOUT BASE

#define MEMORYSZ 0x00020000

int memory[MEMORYSZ];
unsigned pc = 0;

unsigned rs[256], ds[256];
unsigned char dp, rp;

char debug = 0;

unsigned ins0;
unsigned a, b;
char ins;

void printDebug() {
    int ins0;
    char ins;
    unsigned char p;
    ins0 = memory[pc];
    ins = (ins0>>27)&0x1f;
    printf("\t%.6x ", pc);
    if(ins0&DUP) printf("DUP ");
    printf("%s", insStrs[ins]);
    if(!(ins0&POP)) printf(" %.6x", ins0&0xffffff);
    if(ins0&IND) printf("^ ");
    p = dp;
    printf(" \t( ");
    for(ins0 = 0; ins0 < 4; ins0++) printf("%x ", ds[p--]);
    printf(")\n");
}

void step() {
    if(debug) printDebug();
    ins0 = memory[pc++];
    ins = (ins0>>27)&0x1f;
    if(ins0&DUP) { ds[(dp+1)&0xff] = ds[dp]; dp++; }
    if(ins0&POP) a = ds[dp--];
    else {
        a = ins0&0xffffff;
        if(a >= 0x800000) a |= 0xff000000;
    }
    if(ins0&IND) a = memory[a];
    switch(ins) {
    case CALL: rp++; rs[rp] = pc; pc = a; break;
    case LIT: dp++; ds[dp] = a; break;
    case JMP: pc = a; break;
    case NEXT: if((int)--rs[rp] >= 0) pc = a; else rp--; break;
    case JZ: if(!ds[dp--]) pc = a; break;
    case JNZ: if(ds[dp--]) pc = a; break;
    case RET: pc = rs[rp--]; break;
    case RPH: rp++; rs[rp] = a; break;
    case RPL: dp++; ds[dp] = rs[rp--]; break;
    case I: dp++; ds[dp] = rs[(rp-a)&0xff]; break;
    case DROP: dp -= a; break;
    case PICK: a = (dp-a)&0xff; dp++; ds[dp] = ds[a]; break;
    case ADD: ds[dp] += a; break;
    case SUB: ds[dp] -= a; break;
    case AND: ds[dp] &= a; break;
    case OR: ds[dp] |= a; break;
    case XOR: ds[dp] ^= a; break;
    case MUL: ds[dp] *= a; break;
    case DIV: ds[dp] = (int)ds[dp] / (int)a; break;
    case MOD: ds[dp] %= a; break;
    case GTN: ds[dp] = ((int)ds[dp]>(int)a)?-1:0; break;
    case LTN: ds[dp] = ((int)ds[dp]<(int)a)?-1:0; break;
    case STW: memory[a] = ds[dp--];
        if(a < BASE) break;
        if(a == CHOUT) printf("%c", memory[a]);
        break;
    case LDW: dp++; ds[dp] = memory[a]; break;
    case SWAP: b=ds[dp]; ds[dp]=ds[(dp-a)&0xff]; ds[(dp-a)&0xff]=b; break;
    case HALT: exit(0);
    }
}

void run();

void loadFile(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "rb");
    if(!fp) {
        printf("failed to open %s\n", filename);
        exit(1);
    }
    fread(memory, 4, MEMORYSZ, fp);
    fclose(fp);
}

int foam_main(int argc, char **args) {
    int i;
    while(args[argc-1][0] == '-') {
        switch(args[--argc][1]) {
        case 'd': debug = 1; break;
        default: printf("unknown option -%c\n", args[argc][1]); break;
        }
    }
    if(argc < 2) { printf("specify file(s) to run\n"); return 1; }
    loadFile(args[1]);
    run();
    return 0;
}
