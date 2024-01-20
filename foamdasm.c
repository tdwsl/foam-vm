#include <stdio.h>
#include <stdlib.h>

#define DUP 0x04000000
#define POP 0x02000000
#define IND 0x01000000

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

void dasm(const char *filename) {
    FILE *fp;
    int ins0;
    char ins;
    int pc;
    fp = fopen(filename, "rb");
    if(!fp) { printf("failed to open %s\n", filename); }
    pc = 0;
    for(;;) {
        fread(&ins0, 4, 1, fp);
        if(feof(fp)) break;
        printf("%.6X ", pc++);
        ins = (ins0>>27)&0x1f;
        if(ins0&DUP) printf("DUP ");
        printf("%s", insStrs[ins]);
        if(!(ins0&POP)) printf(" %.6X", (unsigned)ins0&0xffffff);
        if(ins0&IND) printf("^ ");
        printf("\n");
    }
    fclose(fp);
}

int main(int argc, char **args) {
    int i;
    for(i = 1; i < argc; i++) dasm(args[i]);
}
