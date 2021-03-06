#pragma once

#include "multiboot.h"

extern int screen_width,screen_height;

#define MAX_SHEETS		256
#define SHEET_USE		1
class SHEETCTRL;
class SHEET {
public:
	class GRAPHICS *graphics;
	int bxsize,bysize,vx0,vy0,height,flags;//height越大越上层
	SHEET(SHEETCTRL *ctl);
	void updown(int height);
 	void refresh(int bx0,int by0,int bx1,int by1);
	void slide(int vx0,int vy0);
	void putstring(int x,int y,int scale,uint32_t f,uint32_t b,char *s);
	void putstring(int x,int y,int scale,uint32_t f,uint32_t b,bool withalpha,char *s);
	void free();
	void window_inactive();
	void window_active();
};
class SHEETCTRL {
private:
	uint8_t bpp;
	void refreshpixel(int x, int y, int height);
public:
	uint32_t *vram;
	uint16_t fb_stride32;
	int top;
	SHEET *sheets[MAX_SHEETS];
	SHEET sheets0[MAX_SHEETS];
	SHEET *allocsheet(int xsize,int ysize);
	void refreshall(int vx0, int vy0, int vx1, int vy1);
};
void shtctl_init(vbe_mode_info_t *vbeinfo);
SHEET *shtctl_alloc(int xsize,int ysize);
void shtctl_refresh(int vx0,int vy0,int vx1,int vy1);
