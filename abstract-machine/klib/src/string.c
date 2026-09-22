#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  // panic("Not implemented");
  size_t len = 0;
  while(s[len] != '\0') len ++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  // panic("Not implemented");
  size_t i, len = strlen(src);
  for (i = 0; i < len; i ++) {
    dst[i] = src[i];
  }
  dst[len] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  // panic("Not implemented");
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i ++) {
    dst[i] = src[i];
  }
  for (; i < n; i ++) {
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  // panic("Not implemented");
  size_t i, len_dst = strlen(dst), len_src = strlen(src);
  for (i = 0; i < len_src; i ++) {
    dst[len_dst + i] = src[i];
  }
  dst[len_dst + len_src] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  panic("Not implemented");
  size_t i, len1 = strlen(s1), len2 = strlen(s2);
  size_t max_len = len1 <= len2 ? len2 : len1;
  for (i = 0; i < max_len; i ++) {
    if (s1[i] != s2[i]) return s1[i] - s2[i];
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  // panic("Not implemented");
  size_t i;
  for (i = 0; i < n; i ++) {
    if (s1[i] != s2[i]) return s1[i] - s2[i];
    if (s1[i] == '\0' || s2[i] == '\0') return 0; //Avoid overflow
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  // panic("Not implemented");
  size_t i = 0;
  unsigned char *p = (unsigned char *)s;
  for (i = 0; i < n; i ++) {
    p[i] = c & 0xff;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  // panic("Not implemented");
  size_t i;
  unsigned char *p_src = (unsigned char *)src, *p_dst = (unsigned char *)dst;
  if (p_src > p_dst) {
    for (i = 0; i < n; i ++) {
      p_dst[i] = p_src[i];
    }
  }
  else {
    for (i = n; i > 0; i --) {
      p_dst[i - 1] = p_src[i - 1];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  // panic("Not implemented");
  size_t i;
  unsigned char *p_out = (unsigned char *)out, *p_in = (unsigned char *)in;
  for (i = 0; i < n; i ++) {
    p_out[i] = p_in[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  // panic("Not implemented");
  if (n == 0) return 0;

  size_t i;
  unsigned char *p_s1 = (unsigned char *)s1, *p_s2 = (unsigned char *)s2;
  for (i = 0; i < n; i ++) {
    if (p_s1[i] != p_s2[i]) return p_s1[i] - p_s2[i];
  }
  return 0;
}

#endif
