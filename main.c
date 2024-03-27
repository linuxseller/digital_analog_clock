#include <avr/interrupt.h>
#include <avr/io.h>
#include "libs/light_ws2812.h"
#include <stdint.h>
#include <avr/cpufunc.h>

#include "data.h"
#include "libs/millis.h"

#define ABS(a) (a>0)?a:(-(a))
#define WIDTH 16
#define HEIGHT 16
#define MAXPIX (WIDTH*HEIGHT)
#define COL 0x10
#define CENTER 8
#define RADIUS 7
#define CLOCK_UPDATE_MS 59583 // every 1 minute. clock generator is broken, so 1 minute does not pass every 60_000 miliseconds
#define UPDATE_ACCORDING_TO_MOD() { \
    switch(cur_mode) { \
        case DIGITAL_CLOCK: \
            updateDigitalClock(); \
            break; \
        case ANALOG_CLOCK: \
            updateAnalogClock(); \
            break; \
        default: break; \
    } \
}

typedef struct {
    int8_t x, y;
} Vector2;

int8_t hours=0, minutes=0;
struct cRGB led[MAXPIX];
int8_t display[MAXPIX];
uint16_t matrix_random=0xa5e6;
enum CurrentMode {ANALOG_CLOCK, DIGITAL_CLOCK, MODE_DVD, MODE_MATRIX, MODES_COUNT};
Vector2 hours_pos, minutes_pos;

struct {
    Vector2 pos;
    Vector2 speed;
    Vector2 size;
} dvd_logo = {.pos=(Vector2){5,4}, .speed=(Vector2){2,1}, .size=(Vector2){9,3}};

uint16_t my_random(){
    unsigned long rndseed = millis();
    matrix_random = (rndseed&0x3f)|((rndseed&0x3f)<<5)|((rndseed&0x3f)<<10);
    return matrix_random;
}

void my_memset(void *p, int8_t c, uint16_t n)
{
    while(n--){
        ((int8_t*)p)[n]=c;
    }
}

void setPX(int8_t x, int8_t y, uint32_t col)
{
    x=WIDTH-x;
    y=HEIGHT-y;
    if(x%2!=0){
        y=WIDTH-1-y;
    }
    led[x*WIDTH+y].r=(col>>16)&0xff;
    led[x*WIDTH+y].g=(col>>8)&0xff;
    led[x*WIDTH+y].b=(col>>0)&0xff;
}

void drawScreen(void)
{
    for(int8_t x=0; x<16; x++){
        for(int8_t y=0; y<16; y++){
            setPX(x, y, statecolor[display[x+y*16]]);
        }
    }
    ws2812_sendarray((uint8_t *)led,MAXPIX*3);
}

void drawNumber(uint8_t id, uint8_t pos)
{
    uint8_t dx = 7*(pos%2==0);
    uint8_t dy = 8*(pos>2);
    if(pos==1 || pos==2){
        dy+=1;
    }
    for(uint8_t y=0; y<7; y++){
        for(int8_t bit=7; bit>=0; bit--){
            if((letters[id][y]>>bit)&1){
                display[(8-bit+dx)+(y+1+dy)*WIDTH] = YELLOW;
            } else {
                display[(8-bit+dx)+(y+1+dy)*WIDTH] = BLACK;
            }
        }
    }
}

void updateDigitalClock(void)
{
    my_memset(display, 0, 16*16*sizeof(*display));
    drawNumber(hours/10,1);
    drawNumber(hours%10,2);
    drawNumber(minutes/10,3);
    drawNumber(minutes%10,4);
}

void plotLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, enum ColorEnum c)
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

void drawRect(Vector2 lu, Vector2 sz, enum ColorEnum c)
{
    sz.x--;
    sz.y--;
    plotLine(lu.x,      lu.y,      lu.x+sz.x, lu.y, c);
    plotLine(lu.x+sz.x, lu.y,      lu.x+sz.x, lu.y+sz.y, c);
    plotLine(lu.x,      lu.y,      lu.x,      lu.y+sz.y, c);
    plotLine(lu.x,      lu.y+sz.y, lu.x+sz.x, lu.y+sz.y, c);
}

void plotArrow(int8_t x1, int8_t y1, enum ColorEnum c)
{
    plotLine(CENTER,CENTER,x1,y1, c);
}
Vector2 vec2Add(Vector2 a, Vector2 b){
    return (Vector2){a.x+b.x, a.y+b.y};
}
void updateMatrix(void)
{
    uint16_t rnd = my_random();
    for (int y = 16; y > 0; y--) {
        for (int x = 0; x <WIDTH; x++) {
            display[x+(y+1)*WIDTH] = display[x+y*WIDTH];
        }
    }
    for(int bit=0; bit<16; bit++){
        display[bit+WIDTH]=((rnd >> bit)&1)?GREEN:BLACK;
    }
}
void updateDvd(void)
{
    my_memset(display, 0, 16*16*sizeof(*display));
    if(dvd_logo.pos.x+dvd_logo.size.x+dvd_logo.size.x>WIDTH || dvd_logo.pos.x+dvd_logo.speed.x<0){
        dvd_logo.speed.x*=-1;
    }
    if(dvd_logo.pos.y+dvd_logo.speed.y+dvd_logo.size.y>HEIGHT || dvd_logo.pos.y+dvd_logo.speed.y<1){
        dvd_logo.speed.y*=-1;
    }
    dvd_logo.pos = vec2Add(dvd_logo.pos, dvd_logo.speed);
    //## # ###
    //# ## ## #
    //##  # ##
    uint8_t y = dvd_logo.pos.y;
    uint8_t x = dvd_logo.pos.x;
    display[(0+x)+(0+y)*WIDTH]=BLUE;
    display[(0+x)+(1+y)*WIDTH]=BLUE;
    display[(0+x)+(2+y)*WIDTH]=BLUE;
    display[(1+x)+(0+y)*WIDTH]=BLUE;
    display[(1+x)+(2+y)*WIDTH]=BLUE;
    display[(2+x)+(1+y)*WIDTH]=BLUE;
    display[(3+x)+(0+y)*WIDTH]=BLUE;
    display[(3+x)+(1+y)*WIDTH]=BLUE;
    display[(4+x)+(2+y)*WIDTH]=BLUE;
    display[(5+x)+(0+y)*WIDTH]=BLUE;
    display[(5+x)+(1+y)*WIDTH]=BLUE;
    display[(6+x)+(0+y)*WIDTH]=BLUE;
    display[(6+x)+(1+y)*WIDTH]=BLUE;
    display[(6+x)+(2+y)*WIDTH]=BLUE;
    display[(7+x)+(0+y)*WIDTH]=BLUE;
    display[(7+x)+(2+y)*WIDTH]=BLUE;
    display[(8+x)+(1+y)*WIDTH]=BLUE;
}
void updateAnalogClock(void)
{
    my_memset(display, 0, 16*16*sizeof(*display));
    for(int8_t x=0; x<16; x++){
        for(int8_t y=0; y<16; y++){
            int8_t xx = x*2/2;
            int8_t yy = y*2/2;
            display[xx+yy*16] = (xx-CENTER)*(xx-CENTER)+(yy-CENTER)*(yy-CENTER)<=RADIUS*RADIUS;
            if((xx-CENTER)*(xx-CENTER)+(yy-CENTER)*(yy-CENTER)<=(RADIUS-1)*(RADIUS-1)){
                display[xx+yy*16] = BLACK;
            }
        }
    }
    plotArrow(lookup[minutes/5][0], lookup[minutes/5][1], RED);
    plotArrow(lookup_h[hours][0], lookup_h[hours][1], BLUE);
}

void updateTime(void)
{
    minutes += 1;
    hours   += minutes==60;
    hours   %= 12;
    minutes %= 60;
}
enum CurrentMode cur_mode = DIGITAL_CLOCK;
int main(void){
    DDRB = (1<<DDB5);
    DDRB &= ~(1<<PINB4);
    DDRB &= ~(1<<PINB2);
    DDRB &= ~(1<<PINB3);
    PORTB |= (1<<PINB4) | (1<<PINB3) | (1<<PINB2);
    init_millis();
    _NOP();
    unsigned long clock_prev_timer=0;
    unsigned long draw_clock_prev_timer=0;
    unsigned long hours_btn_timer=0;
    unsigned long minutes_btn_timer=0;
    unsigned long modes_change_btn_timer=0;
    unsigned long game_timer=0;
    unsigned long matrix_timer=0;
    switch(cur_mode){
        case ANALOG_CLOCK:
            updateAnalogClock();
            break;
        case DIGITAL_CLOCK:
            updateDigitalClock();
            break;
        default:break;
    };
    drawScreen();
	while(1){
        if(millis()-clock_prev_timer>(long unsigned int)CLOCK_UPDATE_MS){
            clock_prev_timer=millis();
            updateTime();
        }
        _NOP();
        uint8_t pinb = PINB;
        if(!((pinb>>PINB3)&1)){
            if(millis()-modes_change_btn_timer>250){
                modes_change_btn_timer = millis();
                cur_mode += 1;
                cur_mode %= MODES_COUNT;
                UPDATE_ACCORDING_TO_MOD();
                drawScreen();
            }
        }
        if(!((pinb>>PINB4)&1)){
            if(millis()-hours_btn_timer>250){
                hours_btn_timer = millis();
                hours++;
                hours%=12;
            }
            UPDATE_ACCORDING_TO_MOD();
            drawScreen();
            clock_prev_timer=millis();
            continue;
        }
        if(!((pinb>>PINB2)&1)){
            if(millis()-minutes_btn_timer>250){
                minutes_btn_timer = millis();
                if(cur_mode==ANALOG_CLOCK){
                    minutes+=5;
                } else if(cur_mode==DIGITAL_CLOCK) {
                    minutes+=1;
                }
                minutes%=60;
            }
            UPDATE_ACCORDING_TO_MOD();
            drawScreen();
            clock_prev_timer=millis();
            continue;
        }
        if(cur_mode==MODE_DVD){
            if(millis()-game_timer>100){
                updateDvd();
                drawScreen();
                game_timer = millis();
            }
        }
        if(cur_mode==MODE_MATRIX){
            if(millis()-matrix_timer>250){
                updateMatrix();
                drawScreen();
                matrix_timer = millis();
            }
        }
        if(millis()-draw_clock_prev_timer>(long unsigned int)CLOCK_UPDATE_MS){
            draw_clock_prev_timer = millis();
            UPDATE_ACCORDING_TO_MOD();
            drawScreen();
        }
    }
}
