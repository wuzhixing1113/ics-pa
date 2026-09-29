#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define MAX_READ_LEN 8192

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char *uint_to_str(int x, char *s) {
  if (x > 9) s = uint_to_str(x / 10, s);
  *s++ = (x % 10) + '0';
  *s = '\0';
  return s;
}

int printf(const char *fmt, ...) {
  char tmp[MAX_READ_LEN];
  tmp[0] = '\0';
  va_list args;
  va_start(args, fmt);
  int ret = vsprintf(tmp, fmt, args);
  va_end(args);

  if (ret > 0) {
    for (const char *p = tmp; *p; p ++) putch(*p);
  }

  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  // panic("Not implemented");
  size_t i = 0, j, fmt_len = strlen(fmt);
  for (j = 0; j < fmt_len; ) {
    if (fmt[j] == '%') {
      j ++; int len = 1, width = 0;
      while (fmt[j] >= '0' && fmt[j] <= '9') {
        width = width * 10 + (fmt[j] - '0');
        len++, j++;
      }
      switch (fmt[j]) {
      case 'd': { // int
        int t = va_arg(ap, int);
        char str[64] = "";
        uint_to_str(t > 0 ? t : -t, str);
        if(t < 0) out[i ++] = '-';
        while (width > strlen(str) + (t < 0 ? 1 : 0)) out[i ++] = '0', width--;
        out[i] = '\0';
        strcat(out, str);
        i += strlen(str);
        break;
      }
      case 's': { // string
        char *s = va_arg(ap, char *);
        while (width > strlen(s)) out[i ++] = ' ', width--;
        out[i] = '\0';
        strcat(out, s);
        i += strlen(s);
        break;
      }
      case 'c': { // character
        while (--width) out[i ++] = ' ';
        out[i ++] = (unsigned char)va_arg(ap, int);
        break;
      }
      default: {
        for (int k = 1; k <= len; k ++) out[i ++] = fmt[j - len + k];
        break;
      }
      }
      j ++;
    }
    else out[i ++] = fmt[j ++];
    out[i] = '\0';
  }
  return i;
}

int sprintf(char *out, const char *fmt, ...) {
  // panic("Not implemented");
  out[0] = '\0';
  va_list args;
  va_start(args, fmt);
  int ret = vsprintf(out, fmt, args);
  va_end(args);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
