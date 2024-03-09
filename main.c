#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "libs/light_ws2812.h"
#include <stdint.h>

#include "libs/millis.h"

#define ABS(a) (a>0)?a:(-(a))
#define WIDTH 16
#define HEIGHT 16
#define MAXPIX (WIDTH*HEIGHT)
#define COL 0x10
#define CENTER 8
#define RADIUS 7

typedef struct {
    int8_t x, y;
} Vector2;

struct cRGB led[MAXPIX];
int8_t display[MAXPIX];
int8_t hours=0, minutes=-5;

Vector2 hours_pos, minutes_pos;
void my_memset(void *p, int8_t c, uint16_t n){
    while(n--){
        ((int8_t*)p)[n]=c;
    }
}

void setPX(int8_t x, int8_t y, uint32_t col){
    if(x%2!=0){
        y=WIDTH-1-y;
    }
    led[x*WIDTH+y].r=(col>>16)&0xff;
    led[x*WIDTH+y].g=(col>>8)&0xff;
    led[x*WIDTH+y].b=(col>>0)&0xff;
}

uint32_t statecolor[]={
    [0] = 0x00000000,
    [1] = 0x00101010,
    [2] = 0x00000020,
    [3] = 0x00200000,
};

int8_t lookup[][2]  ={
    [0] = {8,2},
    [1] = {11,2},
    [2] = {13,5},
    [3] = {14,8},
    [4] = {13,11},
    [5] = {11,13},
    [6] = {8,14},
    [7] = {4,13},
    [8] = {2,11},
    [9] = {2,8},
    [10] = {2,5},
    [11] = {5,2}
};

int8_t lookup_h[][2] = {
    [0] = {8,4},
    [1] = {10,4},
    [2] = {11,6},
    [3] = {12,8},
    [4] = {11,10},
    [5] = {9,11},
    [6] = {7,12},
    [7] = {5,11},
    [8] = {4,10},
    [9] = {4,8},
    [10] = {4,5},
    [11] = {6,4}
};

int8_t tocenter(int8_t a)
{
    if(a>8){return a-1;}
    return a+1;
}

void drawClock(void)
{
    for(int8_t x=0; x<16; x++){
        for(int8_t y=0; y<16; y++){
            setPX(x, y, statecolor[display[x+y*16]]);
        }
    }
    ws2812_sendarray((uint8_t *)led,MAXPIX*3);
}
void plotLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, int8_t c)
{
    int8_t dx = ABS(x1 - x0);
    int8_t sx = x0 < x1 ? 1 : -1;
    int8_t dy = -(ABS(y1 - y0));
    int8_t sy = y0 < y1 ? 1 : -1;
    int8_t error = dx + dy;
    while(1){
        display[x0 + y0*16] = c;
        if(x0 == x1 && y0 == y1){break;}
        int8_t e2 = 2 * error;
        if(e2 >= dy){
            if(x0 == x1){break;}
            error += dy;
            x0 += sx;
        }
        if(e2 <= dx){
            if(y0 == y1){break;}
            error += dx;
            y0 += sy;
        }
    }
}

void plotArrow(int8_t x1, int8_t y1, int8_t c)
{
    plotLine(CENTER,CENTER,x1,y1, c);
}

void updateClock(void)
{
    my_memset(display, 0, 16*16*sizeof(*display));
    for(int8_t x=0; x<16; x++){
        for(int8_t y=0; y<16; y++){
            int8_t xx = x*2/2;
            int8_t yy = y*2/2;
            display[xx+yy*16] = (xx-CENTER)*(xx-CENTER)+(yy-CENTER)*(yy-CENTER)<=RADIUS*RADIUS;
            if((xx-CENTER)*(xx-CENTER)+(yy-CENTER)*(yy-CENTER)<=(RADIUS-1)*(RADIUS-1)){
                display[xx+yy*16] = 0;
            }
        }
    }
    minutes += 5;
    hours   += minutes==60;
    hours   %= 12;
    minutes %= 60;
    plotArrow(lookup[minutes/5][0], lookup[minutes/5][1], 3);
    plotArrow(lookup_h[hours][0], lookup_h[hours][1], 2);
}
int main(void){
 	DDRB = (1<<PB5);
    init_millis();
    unsigned long prev=0;
    updateClock();
    drawClock();
	while(1){
        if(millis()-prev>1){
            prev=millis();
            updateClock();
            drawClock();
        }
    }
}
