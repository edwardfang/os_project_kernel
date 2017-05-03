#include <stddef.h>
#include <stdint.h>
#include "asmfunc.hpp"
#include "multiboot.h"
#include "graphics.hpp"
#include "gdt.hpp"
#include "idt.hpp"
#include "isr.hpp"
#include "paging.hpp"
#include "heap.hpp"
#include "sheet.hpp"
#include "hanastd.hpp"
using namespace hanastd;

SHEET *sht_back,*sht_win;

static void key_handler(registers_t regs){
	(void)regs;
	sht_back->putstring(50,200,2,0x00ff00,0x66ccff,"Keyboard!");
}
uint32_t tick=0;
char buff[32];
static void timer_handler(registers_t regs){
	(void)regs;
	if(tick==4294967295)tick=0;
	else tick++;
	sprintf(buff,"Tick: %d",tick);
	sht_win->putstring(10,50,2,0x000000,sht_win->graphics->bgcolor,buff);
}
void timer_init(uint32_t frequency){
	register_interrupt_handler(IRQ0,&timer_handler);
	uint32_t divisor=1193180/frequency;
	io_out8(0x43,0x36);
	uint8_t l=(uint8_t)(divisor&0xff);
	uint8_t h=(uint8_t)((divisor>>8)&0xff);
	io_out8(0x40,l);
	io_out8(0x40,h);
}
#ifdef __cplusplus
extern "C"{
#endif
extern uint32_t kmalloc_addr;
void kernel_main(multiboot_info_t *hdr,uint32_t magic)
{
	//Memory Test & MEMMAN init
	unsigned int memtotal;
	memtotal=memtest(kmalloc_addr,0xbfffffff);
	auto *memman=(MEMMAN*)kmalloc_a(sizeof(MEMMAN));
	memman->init();
	memman->free(kmalloc_addr,memtotal-kmalloc_addr);
	memset((void*)kmalloc_addr,0,memman->total());

	//Init Sheetctrl
	auto shtctl=sheetctrl_init(memman,(vbe_mode_info_t*)hdr->vbe_mode_info);

	//Init background
	sht_back=shtctl->allocsheet(shtctl->xsize,shtctl->ysize);
	sht_back->graphics->show_bgimg();
	sht_back->slide(0,0);
	sht_back->updown(0);
	sht_back->graphics->setcolor(0x66ccff);
	sht_back->graphics->boxfill(0,100,500,300);
	sht_back->refresh(0,0,1024,768);

	char str[32];
	sprintf(str,"mem %dMB free:%dKB",memtotal/0x400000*4,memman->total()/1024);
	sht_back->putstring(50,110,2,0xff0000,0x66ccff,str);
	sht_back->putstring(50,150,2,0x2a6927,0x66ccff,"Hello HanaOS!");
	
	auto test_win=shtctl->allocsheet(320,100);
	test_win->graphics->init_window("test");
	test_win->slide(250,250);
	test_win->updown(1);
	test_win->putstring(10,50,2,0xff0000,test_win->graphics->bgcolor,"I am a window");

	//Init window
	sht_win=shtctl->allocsheet(320,100);
	sht_win->graphics->init_window("timer");
	sht_win->slide(320,300);
	sht_win->updown(2);

	gdt_init();
	idt_init();
//	paging_init();
	register_interrupt_handler(IRQ1,&key_handler);
	timer_init(10);
	io_sti();
	for(;;)
		io_hlt();
}
#ifdef __cplusplus
}
#endif
