#include <common.h>

#ifdef CONFIG_FTRACE

void init_ftrace(const char *elf_file) {
    Assert(elf_file == NULL, "ELF file cannot be NULL");

    printf("[ftrace] elf = %s\n", elf_file);
}


#endif