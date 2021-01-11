#include "elfreader.h"

#include <bfd.h>


bool ELFSection::contains(uint64_t addr)
{
    if (vma <= addr && vma + size > addr)
        return true;
    
    return false;
}

static bfd* _open_bin(const char* fname)
{
    // initialization
    static bool bfd_inited = false;
    if (!bfd_inited)
    {
        bfd_init();
        bfd_inited = true;
    }

    // Open bin file
    bfd* bfd_file = bfd_openr(fname, nullptr);
    if (!bfd_file)
    {
        fprintf(stderr, "Error openning binary file (%s)\n", bfd_errmsg(bfd_get_error()));
        return nullptr;
    }

    // Format checking
    if (!bfd_check_format(bfd_file, bfd_format::bfd_object))
    {
        fprintf(stderr, "Error: Unsupported format! (%s)\n", bfd_errmsg(bfd_get_error()));
        bfd_close(bfd_file);
        return nullptr;
    }

    bfd_set_error(bfd_error_type::bfd_error_no_error);

    // Check if it is ELF
    if (bfd_get_flavour(bfd_file) != bfd_flavour::bfd_target_elf_flavour)
    {
        fprintf(stderr, "Error: Not an ELF file!\n");
        bfd_close(bfd_file);
        return nullptr;
    }

    return bfd_file;
}

static int _load_static_symbols(bfd *bfd_file, ELFBinary* bin)
{
    auto ub = bfd_get_symtab_upper_bound(bfd_file);

    if (ub < 0)
    {
        fprintf(stderr, "Error reading symbols. (%s)\n", bfd_errmsg(bfd_get_error()));
        return -1;
    }
    else if (ub == 0)
    {
        fprintf(stderr, "No static symbol found in the file.\n");
        return 0;
    }
    
    
    bfd_symbol** sym_tab = (bfd_symbol**) malloc(ub);   //malloc
    if (!sym_tab)
    {
        fprintf(stderr, "Error: Out of memory.\n");
        return -1;
    }

    long sym_n = bfd_canonicalize_symtab(bfd_file, sym_tab);
    for (long i=0; i < sym_n; i++)
    {
        ELFSymbol symbol;
        if(sym_tab[i]->flags & BSF_FUNCTION) // is a function symbol
            symbol.type = ELFSymbol::SymbolType::ELF_SYM_TYPE_FUNC;
        else
            symbol.type = ELFSymbol::SymbolType::ELF_SYM_TYPE_OTHER;

        symbol.name = sym_tab[i]->name;
        symbol.addr = bfd_asymbol_value(sym_tab[i]);

        bin->symbols.push_back(std::move(symbol));
    }

    free(sym_tab);  //free
    
    return 0;
}

static int _load_dyn_symbols(bfd *bfd_file, ELFBinary* bin)
{
    auto ub = bfd_get_dynamic_symtab_upper_bound(bfd_file);

    if (ub < 0)
    {
        fprintf(stderr, "Error reading symbols. (%s)\n", bfd_errmsg(bfd_get_error()));
        return -1;
    }
    else if (ub == 0)
    {
        fprintf(stderr, "No dynamic symbol found in the file.\n");
        return 0;
    }
    
    
    bfd_symbol** sym_tab = (bfd_symbol**) malloc(ub);   //malloc
    if (!sym_tab)
    {
        fprintf(stderr, "Error: Out of memory.\n");
        return -1;
    }

    long sym_n = bfd_canonicalize_dynamic_symtab(bfd_file, sym_tab);
    for (long i=0; i < sym_n; i++)
    {
        ELFSymbol symbol;
        if(sym_tab[i]->flags & BSF_FUNCTION) // is a function symbol
            symbol.type = ELFSymbol::SymbolType::ELF_SYM_TYPE_FUNC;
        else
            symbol.type = ELFSymbol::SymbolType::ELF_SYM_TYPE_OTHER;

        symbol.name = sym_tab[i]->name;
        symbol.addr = bfd_asymbol_value(sym_tab[i]);

        bin->symbols.push_back(std::move(symbol));
    }

    free(sym_tab);  //free
    
    return 0;
}



static int _load_sections(bfd* bfd_file, ELFBinary* bin)
{
    bfd_section* sec = bfd_file->sections;
    while (sec)
    {
        ELFSection elf_section;
        if (sec->flags & SEC_CODE)  // Code section
            elf_section.type = ELFSection::ELF_SEC_TYPE_CODE;
        else if (sec->flags & SEC_DATA)
            elf_section.type = ELFSection::ELF_SEC_TYPE_DATA;
        else
            elf_section.type = ELFSection::ELF_SEC_TYPE_OTHER;

        elf_section.bin = bin;
        elf_section.vma = sec->vma;
        elf_section.name = sec->name ? sec->name : "<noname>";
        elf_section.size = sec->size;

        void* location = malloc(sec->size);    //malloc

        if (!location)
        {
            fprintf(stderr, "Error: Out of memory.\n");
            return -1;
        }

        if (!bfd_get_section_contents(bfd_file, sec, location, 0, sec->size))
        {
            fprintf(stderr, "Error on reading section content. (Section %s)\n", sec->name);
            return -1;
        }

        elf_section.bytes = (uint8_t*) location;

        bin->sections.push_back(std::move(elf_section));

        sec = sec->next;
    }
    
}

static int _load_binary(const char* fname, ELFBinary* bin)
{
    bfd* bfd_file = _open_bin(fname);
    if (!bfd_file)
        return -1;

    bin->fileName = fname;
    bin->entry = bfd_file->start_address;
    bin->arch = bfd_file->arch_info->printable_name;

    if (bfd_file->arch_info->mach & bfd_mach_i386_i386)
        bin->bits = 32;
    else if (bfd_file->arch_info->mach & bfd_mach_x86_64)
        bin->bits = 64;
    else
    {
        fprintf(stderr, "Machine not supported.\n");
        goto fail;
    }
    
    if (_load_static_symbols(bfd_file, bin) < 0)
        goto fail;
    if (_load_dyn_symbols(bfd_file, bin) < 0)
        goto fail;
    if (_load_sections(bfd_file, bin) < 0)
        goto fail;

    bfd_close(bfd_file);
    return 0;


fail:
    bfd_close(bfd_file);
    return -1;  
}


int load_elf(const char* fname, ELFBinary* bin)
{
    return _load_binary(fname, bin);
}

void free_elf(ELFBinary* bin)
{
    for (auto& sec : bin->sections)
    {
        if (sec.bytes)
            free(sec.bytes);
    }
}
