#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include "hanastd.hpp"
using namespace hanastd;

unsigned long strtoul0(const unsigned char **ps, int base, unsigned char *errflag);

static int prefix(int c)
{
	signed char base = 0;
	if ('a' <= c && c <= 'z')
		c += 'A' - 'a';
	if (c == 'B')
		base =  2;
	if (c == 'D')
		base = 10;
	if (c == 'O')
		base =  8;
	if (c == 'X')
		base = 16;
	return base;
}

unsigned long strtoul0(const unsigned char **ps, int base, unsigned char *errflag)
{
	const unsigned char *s = *ps;
	unsigned long val = 0, max;
	int digit;
	if (base == 0) {
		base += 10;
		if (*s == '0') {
			base = prefix(*(s + 1));
			if (base == 0)
				base += 8; /* base = 8; */
		}
	}
	if (*s == '0') {
		if (base == prefix(*(s + 1)))
			s += 2;
	}
	max = ULONG_MAX / base;
	*errflag = 0;
	for (;;) {
		digit = 99;
		if ('0' <= *s && *s <= '9')
			digit = *s - '0';
		if ('A' <= *s && *s <= 'Z')
			digit = *s - ('A' - 10);
		if ('a' <= *s && *s <= 'z')
			digit = *s - ('a' - 10);
		if (digit >= base)
			break;
		if (val > max)
			goto err;
		val *= base;
		if (ULONG_MAX - val < (unsigned long) digit) {
err:
			*errflag = 1;
			val = ULONG_MAX;
		} else
			val += digit;
		s++;
	}
	*ps = s;
	return val;
}

typedef unsigned char UCHAR;
typedef unsigned int UINT;

static UCHAR *setdec(UCHAR *s, UINT ui)
{
	do {
		*--s = (ui % 10) + '0';
	} while (ui /= 10);
	return s;
}

int vsprintf(char *s, const char *format, va_list arg)
{
	UCHAR c, *t = s, *p, flag_left, flag_zero /* , flag_sign, flag_space */;
	UCHAR temp[32] /* ”Žšp */, *q;
	temp[31] = '\0';
	int field_min, field_max, i;
	long l;
	static char hextable_X[16] = "0123456789ABCDEF";
	static char hextable_x[16] = "0123456789abcdef";
	for (;;) {
		c = *format++;
		if (c != '%') {
put1char:
			*t++ = c;
			if (c)
				continue;
			return t - (UCHAR *) s - 1;
		}
		flag_left = flag_zero = /* flag_sign = flag_space = flag_another = */ 0;
		c = *format++;
		for (;;) {
			if (c == '-')
				flag_left = 1;
			else if (c == '0')
				flag_zero = 1;
			else
				break;
			c = *format++;
		}
		field_min = 0;
		if ('1' <= c && c <= '9') {
			format--;
			field_min = (int) strtoul0(&format, 10, &c);
			c = *format++;
		} else if (c == '*') {
			field_min = va_arg(arg, int);
			c = *format++;
		}
		field_max = INT_MAX;
		if (c == '.') {
			c = *format++;
			if ('1' <= c && c <= '9') {
				format--;
				field_min = (int) strtoul0(&format, 10, &c);
				c = *format++;
			} else if (c == '*') {
				field_max = va_arg(arg, int);
				c = *format++;
			}
		}
		if (c == 's') {
			if (field_max != INT_MAX)
				goto mikan;
			p = va_arg(arg, unsigned char *);
			l = strlen(p);
			if (*p) {
				c = ' ';
copy_p2t:
				if (flag_left == 0) {
					while (l < field_min) {
						*t++ = c;
						field_min--;
					}
				}
				do {
					*t++ = *p++;
				} while (*p);
			}
			while (l < field_min) {
				*t++ = ' ';
				field_min--;
			}
			continue;
		}
		if (c == 'l') {
			c = *format++;
			if (c != 'd' && c != 'x' && c != 'u') {
				format--;
				goto mikan;
			}
		}
		if (c == 'u') {
			l = va_arg(arg, UINT);
			goto printf_u;
		}
		if (c == 'd') {
printf_d:
			l = va_arg(arg, long);
			if (l < 0) {
				*t++ = '-';
				l = - l;
				field_min--;
			}
printf_u:
			if (field_max != INT_MAX)
				goto mikan;
			if (field_min <= 0)
				field_min = 1;
			p = setdec(&temp[31], l);
printf_x2:
			c = ' ';
			l = &temp[31] - p;
			if (flag_zero)
				c = '0';
			goto copy_p2t;
		}
		if (c == 'i')
			goto printf_d;
		if (c == '%')
			goto put1char;
		if (c == 'x') {
			q = hextable_x;
printf_x:
			l = va_arg(arg, UINT);
			p = &temp[31];
			do {
				*--p = q[l & 0x0f];
			} while ((*(UINT *) &l) >>= 4);
			goto printf_x2;
		}
		if (c == 'X') {
			q = hextable_X;
			goto printf_x;
		}
		if (c == 'p') {
			i = (int) va_arg(arg, void *);
			p = &temp[31];
			for (l = 0; l < 8; l++) {
				*--p = hextable_X[i & 0x0f];
				i >>= 4;
			}
			goto copy_p2t;
		}
		if (c == 'o') {
			l = va_arg(arg, UINT);
			p = &temp[31];
			do {
				*--p = hextable_x[l & 0x07];
			} while ((*(UINT *) &l) >>= 3);
			goto printf_x2;
		}
		if (c == 'f') {
			if (field_max == INT_MAX)
				field_max = 6;
			/* for ese */
			if (field_min < field_max + 2)
				field_min = field_max + 2;
			do {
				*t++ = '#';
			} while (--field_min);
			continue;
		}
mikan:
		for(;;);
	}
}

int hanastd::sprintf(char *s, const char *format, ...){
	int i;
	va_list ap;

	va_start(ap, format);
	i = vsprintf(s, format, ap);
	va_end(ap);
	return i;
}

void *hanastd::memset(void *s, int c, uint32_t n){
    unsigned char *pos = s;

    while (n--)
        *pos++ = (unsigned char)c;

    return s;
}
