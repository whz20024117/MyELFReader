#include "src/elfreader.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: example binary_name.\n");
        return 0;
    }


    ELFBinary bin;

    load_elf(argv[1], &bin);

    printf("Arch: %s\n", bin.arch.c_str());
    printf("Bits: %d\n", bin.bits);
    printf("Entrance: 0x%x\n", bin.entry);
    printf("Number of Sections: %l\n", bin.sections.size());
    printf("Number of Symbols: %l\n", bin.symbols.size());

    printf("List of sections: \n");
    for (auto& s : bin.sections)
    {
        printf("    %s\n", s.name.c_str());
    }

    printf("List of symbols: \n");
    for (auto& s : bin.symbols)
    {
        printf("    %s\n", s.name.c_str());
    }

    free_elf(&bin);

    return 0;
}