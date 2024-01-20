#include <SDL2/SDL.h>
#include <stdlib.h>
#include <assert.h>

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *screen;

#define W 384
#define H 224

extern int memory[];
extern int rs[], ds[];
extern unsigned char rp, dp;
extern unsigned int pc;
int *pal = &memory[0x10110];
int *scr = &memory[0x10200];
int *xo = &memory[0x10122];
int *yo = &memory[0x10123];
int *vupdate = &memory[0x10100];
int *vkeydown = &memory[0x10101];

int foam_main(int argc, char **args);
void step();

void draw() {
    int i, j, x, y, c;
    int *t;
    SDL_Rect r;
    c = pal[0];
    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, c>>16, c>>8, c, 0xff);
    SDL_RenderClear(renderer);
    for(y = 0; y < 29; y++) {
        for(x = 0; x < 49; x++) {
            t = &memory[scr[y*49+x]];
            for(i = 0; i < 8; i++) {
                for(j = 0; j < 8; j++) {
                    c = pal[(t[i]>>((7-j)*4))&0xf];
                    SDL_SetRenderDrawColor(renderer, c>>16, c>>8, c, 0xff);
                    SDL_RenderDrawPoint(renderer, x*8+j-*xo, y*8+i-*yo);
                }
            }
        }
    }
    SDL_SetRenderTarget(renderer, 0);
    SDL_GetWindowSize(window, &i, &r.h);
    r.w = W*r.h/H;
    r.x = i/2-r.w/2;
    r.y = 0;
    SDL_RenderCopy(renderer, screen, 0, &r);
    SDL_RenderPresent(renderer);
}

void interrupt(int a) {
    //int p;
    rp++;
    rs[rp] = pc;
    //p = pc;
    pc = a;
    //do step(); while(pc != p);
}

void run() {
    SDL_Event ev;
    int i, current, last;
    last = SDL_GetTicks();
    for(;;) {
        while(SDL_PollEvent(&ev)) {
            switch(ev.type) {
            case SDL_QUIT: exit(0);
            case SDL_KEYDOWN:
                if(*vkeydown) {
                    dp++;
                    ds[dp] = ev.key.keysym.sym;
                    interrupt(*vkeydown);
                }
                break;
            }
        }
        for(i = 0; i < 32768; i++) step();
        if((current = SDL_GetTicks())-last > 20 || current < last) {
            if(*vupdate) interrupt(*vupdate);
            last = current;
            draw();
        }
    }
}

int main(int argc, char **args) {
    assert(window = SDL_CreateWindow("Foam-VM",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            W, H, SDL_WINDOW_RESIZABLE));
    assert(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE));
    assert(screen = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            W, H));
    return foam_main(argc, args);
}
