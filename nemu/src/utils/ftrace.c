#include <common.h>

#ifdef CONFIG_FTRACE

#define NR_FUNCT 256
#define BUF_SIZE 256

struct {
    const char *name;
    word_t start;
    word_t size;
} funct[NR_FUNCT];

static int depth;
static bool enabled;
static int nr_funct;
static char ftrace_buf[BUF_SIZE];

static const char *find_funct(vaddr_t addr) {
    int i;
    for (i = 0; i < nr_funct; i ++) {
        if (addr >= funct[i].start && addr < funct[i].start + funct[i].size) {
            return funct[i].name;
        }
    }
    return "???";
}

static bool parse_elf(const char *elf_file) {
    //FILE *fp = fopen(elf_file, "rb");

    return true;
}

void init_ftrace(const char *elf_file) {
    Assert(elf_file != NULL, "[ftrace] ELF file cannot be NULL\n");

    Assert(parse_elf(elf_file), "[ftrace] Fail to load %s\n", elf_file);

    enabled = true;

    printf("[ftrace] Initialized successfully!\n");
}

void ftrace_call(vaddr_t pc, vaddr_t target) {
    if (!enabled) return;
    char *p = ftrace_buf;
    p += snprintf(p, sizeof(ftrace_buf), FMT_WORD ": %*scall [%s@" FMT_WORD "]",
         pc, depth * 2, "", find_funct(target), target);
    printf("%s\n", ftrace_buf);
    depth ++;
}

void ftrace_ret(vaddr_t pc) {
    if (!enabled) return;
    depth --;
    char *p = ftrace_buf;
    p += snprintf(p, sizeof(ftrace_buf), FMT_WORD ": %*sret  [%s]",
         pc, depth * 2, "", find_funct(pc));
    printf("%s\n", ftrace_buf);
}

#endif