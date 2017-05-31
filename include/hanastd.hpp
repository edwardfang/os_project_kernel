#ifndef _HANASTD_HPP
#define _HANASTD_HPP

#include <stdint.h>

namespace hanastd {
	template <typename T>
	auto strlen(T str){
		auto i=str;
		for(;*i!=0;i++);
		return i-str+1;
	}
	int sprintf(char *s, const char *format, ...);
	void *memset(void *s, int c, uint32_t n);
	void strncpy(const char *src, char *dest, int n);
}

#endif
