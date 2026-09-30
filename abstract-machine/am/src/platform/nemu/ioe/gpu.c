#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // int i;
  // AM_GPU_CONFIG_T cfg;
  // ioe_read(AM_GPU_CONFIG, &cfg);
  // int w = cfg.width;  // TODO: get the correct width
  // int h = cfg.height;  // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 0, .height = 0,
    .vmemsz = 0
  };
  int32_t hw  = inl(VGACTL_ADDR);
  cfg->height = hw & 0xffff;
  cfg->width  = hw >> 16;
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y;
  int h = ctl->h, w = ctl->w;

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *buf = ctl->pixels;
  AM_GPU_CONFIG_T cfg;
  ioe_read(AM_GPU_CONFIG, &cfg);

  for (int i = 0; i < h; i ++) {
    for (int j = 0; j < w; j ++) {
      fb[(y + i) * cfg.width + (x + j)] = buf[i * w + j];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
