#define _GNU_SOURCE
#include <link.h>
#include <stdio.h>
#include <stdlib.h>

// Assuming AUDIT_PROGRAM is defined as a string literal
#ifndef AUDIT_PROGRAM
#define AUDIT_PROGRAM "default_program"
#endif

FILE *open_file(const char *base_name, const char *suffix) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s-%s.txt", base_name, suffix);
    return fopen(filename, "w");
}

static FILE *search_file = NULL;
static FILE *found_file = NULL;
static FILE *symbol_file = NULL;

void open_files(const char *audit_program) {
    if (!search_file)
        search_file = open_file(audit_program, "search");
    if (!found_file)
        found_file = open_file(audit_program, "found");
    if (!symbol_file)
        symbol_file = open_file(audit_program, "symbol");
}

void close_files() {
    if (search_file) {
        fclose(search_file);
        search_file = NULL;
    }
    if (found_file) {
        fclose(found_file);
        found_file = NULL;
    }
    if (symbol_file) {
        fclose(symbol_file);
        symbol_file = NULL;
    }
}

unsigned int la_version(unsigned int version) {
    const char *audit_program = getenv("AUDIT_PROGRAM");
    if (!audit_program) {
        audit_program = "default_program";
    }
    open_files(audit_program);
    return LAV_CURRENT;
}

char *la_objsearch(const char *name, uintptr_t *cookie, unsigned int flag) {
    if (search_file) {
        if(flag == LA_SER_ORIG)
        {
            fprintf(search_file, "\n");
        }
        fprintf(search_file, "%s --> %s\n", name, (flag == LA_SER_ORIG) ? "original" :
                                                (flag == LA_SER_LIBPATH) ? "libpath" :
                                                (flag == LA_SER_RUNPATH) ? "runpath" :
                                                (flag == LA_SER_CONFIG) ? "config" :
                                                (flag == LA_SER_DEFAULT) ? "default" : "unknown");
    }
    return (char *)name;
}

unsigned int la_objopen(struct link_map *map, Lmid_t lmid, uintptr_t *cookie) {
    if (found_file && map) {
        fprintf(found_file, "%s\n", map->l_name);
    }
    *cookie = (uintptr_t)map;
    return LA_FLG_BINDTO | LA_FLG_BINDFROM;
}

uintptr_t la_symbind64(Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook, uintptr_t *defcook, unsigned int *flags, const char *symname) {
    struct link_map *map = (struct link_map *)*defcook;
    if (symbol_file && symname && map) {
        fprintf(symbol_file, "%s <-- %s\n", symname, map->l_name);
    }
    return sym->st_value;
}

unsigned int la_objclose(uintptr_t *cookie) {
    close_files();
    return 0;
}

void __attribute__((destructor)) finalize() {
    close_files();
}
