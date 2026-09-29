#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define MAX_READ_LEN 65536

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char *int_to_str(int x, char *s) {
  if (x < 0) *s++ = '-', x = -x;
  if (x > 9) s = int_to_str(x / 10, s);
  *s++ = (x % 10) + '0';
  *s = '\0';
  return s;
}

int printf(const char *fmt, ...) {
  char tmp[MAX_READ_LEN];
  tmp[0] = '\0';
  va_list args;
  va_start(args, fmt);
  int ret = sprintf(tmp, fmt, args);
  va_end(args);

  if (ret > 0) {
    for (const char *p = tmp; *p; p ++) putch(*p);
  }

  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  // panic("Not implemented");
  size_t i = 0, j, fmt_len = strlen(fmt);
  for (j = 0; j < fmt_len; j ++) {
    if (fmt[j] == '%') continue;
    if (j == 0 || fmt[j - 1] != '%') out[i ++] = fmt[j];
    else if (fmt[j - 1] == '%') {
      switch (fmt[j]) {
      case 'd': { // int
        int tmp = va_arg(ap, int);
        char str[32] = "";
        int_to_str(tmp, str);
        strcat(out, str);
        i += strlen(str);
        break;
      }
      case 's': { // string
        char *s = va_arg(ap, char *);
        strcat(out, s);
        i += strlen(s);
        break;
      }
      case 'c': { // character
        out[i ++] = va_arg(ap, int);
        break;
      }
      default:
        break;
      }
    }
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
