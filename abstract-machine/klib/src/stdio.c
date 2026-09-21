#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

char *int_to_str(int x, char *s) {
  if (x < 0) *s++ = '-', x = -x;
  if (x > 9) s = int_to_str(x / 10, s);
  *s++ = (x % 10) + '0';
  *s = '\0';
  return s;
}

int sprintf(char *out, const char *fmt, ...) {
  // panic("Not implemented");
  out[0] = '\0';
  size_t i = 0, j, fmt_len = strlen(fmt);
  va_list args;
  va_start(args, fmt);
  for (j = 0; j < fmt_len; j ++) {
    if (fmt[j] == '%') continue;
    if (j == 0 || fmt[j - 1] != '%') out[i ++] = fmt[j];
    else if (fmt[j - 1] == '%') {
      if (fmt[j] == 'd') {
        int tmp = va_arg(args, int);
        char str[32] = "";
        int_to_str(tmp, str);
        strcat(out, str);
        i += strlen(str);
      }
      else if (fmt[j] == 's') {
        char *s = va_arg(args, char *);
        strcat(out, s);
        i += strlen(s);
      }
      else return -1;
    }
    out[i] = '\0';
  }
  va_end(args);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
