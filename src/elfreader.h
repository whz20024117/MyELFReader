#include <string>
#include <vector>


//forward declaration
class ELFSection;
class ELFSymbol;
class ELFBinary;



class ELFSection
{
public:
    enum SectionType    // Code or data section
    {
        ELF_SEC_TYPE_CODE,
        ELF_SEC_TYPE_DATA,
        ELF_SEC_TYPE_OTHER
    };

    std::string name;
    SectionType type;
    size_t vma;
    size_t size;
    // allows byte level operation. pointer arithmatics not allowed on void* by standard
    uint8_t* bytes;
    ELFBinary* bin;


    bool contains(uint64_t addr);
    
    ELFSection()
        : bytes(nullptr), size(0), vma(0), name(), bin(nullptr) {}
};


class ELFSymbol
{
public:
    enum SymbolType
    {
        ELF_SYM_TYPE_FUNC,
        ELF_SYM_TYPE_OTHER
    };

    SymbolType type;
    std::string name;
    size_t addr;

    ELFSymbol()
        : type(ELF_SYM_TYPE_OTHER), name(), addr(0) {}

};


class ELFBinary
{
public:
    std::string fileName;
    uint8_t bits;    //32 or 64 elf program
    size_t entry;
    std::string arch;
    std::vector<ELFSection> sections;
    std::vector<ELFSymbol> symbols;

    ELFBinary()
        :bits(0), entry(0), fileName() {}
};

// loading function
int load_elf(const char* fname, ELFBinary* bin);

// Clean up function
void free_elf(ELFBinary* bin);

