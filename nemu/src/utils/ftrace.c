#include <common.h>

#ifdef CONFIG_FTRACE

#define NR_FUNCT 256
#define BUF_SIZE 256

#define SHT_SYMTAB 2
#define STT_FUNC 2

struct {
    const char *name;
    word_t start;
    word_t size;
} funct[NR_FUNCT];

static int depth;
static bool enabled;
static bool little_endian;
static int nr_funct;
static char ftrace_buf[BUF_SIZE];
static unsigned char *elf_data;
static size_t elf_size;

static const char *find_funct(vaddr_t addr) {
    int i;
    for (i = 0; i < nr_funct; i ++) {
        if (addr >= funct[i].start && addr < funct[i].start + funct[i].size) {
            return funct[i].name;
        }
    }
    return "???";
}

static uint32_t rd32(const unsigned char *p) {                                                                                                                       
    if (little_endian)                                                                                                                                               
        return (uint32_t)p[0] | ((uint32_t)p[1] << 8)                                                                                                                                 
            | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);                                                                                                                               
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)                                                                                                                                      
        | ((uint32_t)p[2] << 8) | (uint32_t)p[3];                                                                                                                                           
}

static uint16_t rd16(const unsigned char *p) {                                                                                                                       
    if (little_endian) return (uint16_t)(p[0] | (p[1] << 8));                                                                                                                       
    return (uint16_t)((p[0] << 8) | p[1]);                                                                                                                           
} 

static bool parse_elf(const char *elf_file) {
    FILE *fp = fopen(elf_file, "rb");
    
    if (fseek(fp, 0, SEEK_END) != 0) return fclose(fp), false;
    long sz = ftell(fp);
    if (sz <= 0) return fclose(fp), false;
    if (fseek(fp, 0, SEEK_SET) != 0) return fclose(fp), false;

    elf_data = malloc((size_t)sz);
    if (elf_data == NULL) return fclose(fp), false;
    if (fread(elf_data, 1, (size_t)sz, fp) != (size_t)sz) {                                                                                                          
        fclose(fp);                                                                                                                                                  
        free(elf_data);                                                                                                                                              
        elf_data = NULL;                                                                                                                                             
        return false;                                                                                                                                                
    }                                                                                                                                                                
    fclose(fp);

    elf_size = (size_t)sz;

    if (memcmp(elf_data, "\177ELF", 4) != 0) return false;

    if (elf_data[5] != 1 && elf_data[5] != 2) return false;
    little_endian = elf_data[5] == 1;

    uint32_t shoff = rd32(elf_data + 32);
    uint16_t shentsize = rd16(elf_data + 46);
    uint16_t shnum = rd16(elf_data + 48);

    for (int i = 0; i < shnum; i ++) {
        unsigned char *sh = elf_data + shoff + i * shentsize;

        if (rd32(sh + 4) != SHT_SYMTAB) continue;

        uint32_t sym_off = rd32(sh + 16);
        uint32_t sym_size = rd32(sh + 20);
        uint32_t str_idx = rd32(sh + 24);
        uint32_t sym_ent = rd32(sh + 36);
        if (sym_ent == 0) continue; 

        unsigned char *strsh = elf_data + shoff + str_idx * shentsize;                                                                                               
        uint32_t str_off = rd32(strsh + 16);                                                                                                                         
                                                                                                                                                                          
        int n = sym_size / sym_ent;                                                                                                                                  
                                                                                                                                                                          
        for (int j = 0; j < n && nr_funct < NR_FUNCT; j++) {                                                                                                         
            unsigned char *sym = elf_data + sym_off + j * sym_ent;                                                                                                   
                                                                                                                                                                          
            uint32_t nameoff = rd32(sym + 0);                                                                                                                        
            uint32_t value   = rd32(sym + 4);                                                                                                                        
            uint32_t size    = rd32(sym + 8);                                                                                                                        
            unsigned char info = sym[12];                                                                                                                            
                                                                                                                                                                          
            if ((info & 0xf) != STT_FUNC) continue;                                                                                                                  
            if (size == 0) continue;                                                                                                                                 
                                                                                                                                                                          
            funct[nr_funct].name  = (char *)elf_data + str_off + nameoff;                                                                                            
            funct[nr_funct].start = value;                                                                                                                           
            funct[nr_funct].size  = size;                                                                                                                            
            nr_funct++;                                                                                                                                              
        }                                                                                                                                                            
                                                                                                                                                                          
        break;        
    }

    return nr_funct > 0;
}

void init_ftrace(const char *elf_file) {
    Assert(elf_file != NULL, "[ftrace] ELF file cannot be NULL\n");

    Assert(parse_elf(elf_file), "[ftrace] Fail to load %s\n", elf_file);

    enabled = true;

    printf("[ftrace] Initialized successfully!\n");
    printf("[ftrace] loaded %d functions\n", nr_funct);
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
    if (depth > 0) depth --;
    char *p = ftrace_buf;
    p += snprintf(p, sizeof(ftrace_buf), FMT_WORD ": %*sret  [%s]",
         pc, depth * 2, "", find_funct(pc));
    printf("%s\n", ftrace_buf);
}

#endif