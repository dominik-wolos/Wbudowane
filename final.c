#include <targets/AT91SAM7.h>
#include "lcd.h"
#include <stdio.h>
 
#define LCD_BACKLIGHT PIOB_SODR_P20
#define SW_1 PIOB_SODR_P24
#define SW_2 PIOB_SODR_P25
 
#define JOY_UP PIOA_SODR_P9
#define JOY_DOWN PIOA_SODR_P8
#define JOY_RIGHT PIOA_SODR_P14
#define JOY_LEFT PIOA_SODR_P7
#define JOY_ENTER PIOA_SODR_P15

#define PADDING_START 2
#define PADDING_END 126

#define OFFSET 50
#define WIDTH 132
#define SCALE 0.33
#define RECT_START 125
#define POINTER_Y 95
#define HEADER_DEF 10
 
typedef struct menu_struct menu_t;
struct menu_struct {
  const char * name;
  menu_t * next;
  menu_t * prev;
  menu_t * child;
  menu_t * parent;
  int position;
};

__attribute__ ((section(".fast")))

void delay(int n);
void LCDClearScreenV2(int bColor);
void drawMenu(menu_t *menu_p);
void drawPointer(menu_t *menu_p);
void handleClick(menu_t *menu_pointer);

void initLCD() {
    PMC_PCER=PMC_PCER_PIOB;
    PMC_PCER=PMC_PCER_PIOA;
    PIOB_OER=LCD_BACKLIGHT;
    PIOB_PER=LCD_BACKLIGHT;

    InitLCD();
    LCDSettings();
}

void initMenuStructs(
    menu_t *menu_1,
    menu_t *menu_2,
    menu_t *menu_3,
    menu_t *sub_1_1,
    menu_t *sub_1_2,
    menu_t *sub_2_1,
    menu_t *sub_2_2,
    menu_t *sub_3_1,
    menu_t *sub_3_2
);

int main()
{
    initLCD();

    menu_t menu_1,menu_2,menu_3, sub_1_1, sub_1_2, sub_2_1, sub_2_2, sub_3_1, sub_3_2;
    menu_t *menu_pointer;

    initMenuStructs(
        &menu_1,
        &menu_2,
        &menu_3,
        &sub_1_1,
        &sub_1_2,
        &sub_2_1,
        &sub_2_2,
        &sub_3_1,
        &sub_3_2
    );
 
    menu_pointer = & menu_1;
    
    drawMenu(menu_pointer);
    drawPointer(menu_pointer);
    while(1) {
      handleClick(menu_pointer);
      delay(200);
    }
  
    return 0;
}

void delay(int n) {
    volatile int i;
    
    for(i=3000*n;i>0;i--) {
        __asm__("nop");
    }
}

void initMenuStructs(
    menu_t *menu_1,
    menu_t *menu_2,
    menu_t *menu_3,
    menu_t *sub_1_1,
    menu_t *sub_1_2,
    menu_t *sub_2_1,
    menu_t *sub_2_2,
    menu_t *sub_3_1,
    menu_t *sub_3_2
) {
    sub_1_1.name = "PROGRAM 1 ";
    sub_1_1.prev = NULL;
    sub_1_1.next = &sub_1_2;
    sub_1_1.parent = &menu_1;
    sub_1_1.child = NULL;
    sub_1_1.position = 0;

    sub_1_2.name = "PROGRAM 2 ";
    sub_1_2.prev = &sub_1_1;
    sub_1_2.next = NULL;
    sub_1_2.parent = &menu_1;
    sub_1_2.child = NULL;
    sub_1_2.position = 1;

    sub_2_1.name = "PROGRAM 1 ";
    sub_2_1.prev = NULL;
    sub_2_1.next = &sub_2_2;
    sub_2_1.parent = &menu_2;
    sub_2_1.child = NULL;
    sub_2_1.position = 0;

    sub_2_2.name = "PROGRAM 2 ";
    sub_2_2.prev = &sub_2_1;
    sub_2_2.next = NULL;
    sub_2_2.parent = &menu_2;
    sub_2_2.child = NULL;
    sub_2_2.position = 1;

    sub_3_1.name = "PROGRAM 1 ";
    sub_3_1.prev = NULL;
    sub_3_1.next = &sub_3_2;
    sub_3_1.parent = &menu_3;
    sub_3_1.child = NULL;
    sub_3_1.position = 0;

    sub_3_2.name = "MENU 1 ";
    sub_3_2.prev = &sub_3_1;
    sub_3_2.next = NULL;
    sub_3_2.parent = &menu_3;
    sub_3_2.child = &sub_1_1;
    sub_3_2.position = 1;

    menu_1.name = "MENU 1 ";
    menu_1.prev = NULL;
    menu_1.next = &menu_2;
    menu_1.parent = NULL;
    menu_1.child = &sub_1_1;
    menu_1.position = 0;

    menu_2.name = "MENU 2 ";
    menu_2.prev = &menu_1;
    menu_2.next = &menu_3;
    menu_2.parent = NULL;
    menu_2.child = &sub_2_1;
    menu_2.position = 1;

    menu_3.name = "MENU 3 ";
    menu_3.prev = &menu_2;
    menu_3.next = NULL;
    menu_3.parent = NULL;
    menu_3.child = &sub_3_1;
    menu_3.position = 2;
}

void LCDClearScreenV2(int bColor) {
    long i; 
    WriteSpiCommand(PASET);
    WriteSpiData(0);
    WriteSpiData(131);
    
    WriteSpiCommand(CASET);
    WriteSpiData(0);
    WriteSpiData(131);
    
    WriteSpiCommand(RAMWR);
    for(i = 0; i < ((131 * 131) / 2); i++) {
        WriteSpiData((bColor >> 4) & 0xFF);
        WriteSpiData(((bColor & 0xF) << 4) | ((bColor >> 8) & 0xF));
        WriteSpiData(bColor & 0xFF);
    }
    WriteSpiCommand(NOP);
}

void DrawBorder(int x0, int y0, int x1, int y1, int color){
    LCDSetLine(x0, y0, x0, y1,color);
    LCDSetLine(x0+1, y0, x1-1, y0, color);
    LCDSetLine(x0+1, y1, x1-1, y1, color);
    LCDSetLine(x1, y0, x1, y1, color);
}

void writeHeader(menu_t *menu_p) {
    if (menu_p -> parent){
        LCDPutStr(menu_p->parent->name, HEADER_DEF, HEADER_DEF, LARGE, BLACK, YELLOW);
    } else {
        LCDPutStr("DOMINIK WOLOS", HEADER_DEF, HEADER_DEF, SMALL, BLACK, GREEN);
    }
}

double getScaledX(menu_t *menu) {
    return OFFSET + menu->position * (WIDTH - OFFSET) * SCALE;
}

void drawPointer(menu_t *menu_p){
    DrawRect(OFFSET-1, POINTER_Y-1, RECT_START, RECT_START-3, GREEN);
    LCDPutStr(" < " , getScaledX(menu_p), POINTER_Y ,LARGE,BLACK,YELLOW);
}

void drawMenu(menu_t *menu_p){
    LCDClearScreenV2(GREEN);
    
    DrawBorder(PADDING_START, PADDING_START, PADDING_END, PADDING_END, BLACK);
    writeHeader(menu_p);
    
    while (menu_p -> prev){
        menu_p=menu_p -> prev;
    }
    
    while (menu_p){
        LCDPutStr(menu_p->name , getScaledX(menu_p), 10 ,LARGE,BLACK,YELLOW);
        menu_p=menu_p -> next;
    }
}

void handleClick(menu_t *menu_pointer) {
    if (((PIOA_PDSR & JOY_UP) == 0) && menu_p->prev) {
        menu_pointer = menu_pointer->prev;
        drawPointer(menu_pointer);

        return;
    }

    if (((PIOA_PDSR & JOY_DOWN) == 0) && menu_pointer->next) {
        menu_pointer = menu_pointer->next;
        drawPointer(menu_pointer);

        return;
    }

    if (((PIOA_PDSR & JOY_LEFT) == 0)  && menu_pointer->parent) {
        menu_pointer = menu_pointer->parent;
        handleClick(menu_pointer);

        return;
    }

    if (((PIOA_PDSR & JOY_RIGHT) == 0)  && menu_pointer->child) {
        menu_pointer = menu_pointer->child;
        handleClick(menu_pointer);

        return;
    }

    if (((PIOA_PDSR & JOY_ENTER) == 0)  && menu_pointer->child) {
        menu_pointer->child->parent = menu_pointer;
        menu_pointer = menu_pointer->child;
        handleClick(menu_pointer);

        return;
    }

    if (((PIOB_PDSR & SW_1) == 0) && menu_pointer->child) {
        menu_pointer = menu_pointer -> child;
        handleClick(menu_pointer);

        return;
    }

    if ((PIOB_PDSR & SW_2) == 0) {
        menu_pointer = menu_pointer->parent;
        handleClick(menu_pointer);

        return;
    }
}
 
