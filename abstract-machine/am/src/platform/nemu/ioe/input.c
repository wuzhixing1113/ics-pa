#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  int32_t cur_keycode = inl(KBD_ADDR);
  kbd->keydown = KEYDOWN_MASK & cur_keycode;
  kbd->keycode = ~KEYDOWN_MASK & cur_keycode;
}
