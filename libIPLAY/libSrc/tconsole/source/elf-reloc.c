/*
*******************************************************************************
*                           Magic Pixel Inc. Inc.                             *
*                  Copyright (c) 2009 -, All Rights Reserved                  *
*                                                                             *
* Elf relocation                                                              *
*                                                                             *
* File : Elf-reloc.c                                                          *
* By : Kevin Huang                                                            *
*                                                                             *
*                                                                             *
* Description : relocation                                                    *
*                                                                             *
* History : 2009/04/01 File created.                                          *
*                                                                             *
*******************************************************************************
*/
/*************************************************************************/
/***                        Include Files                              ***/
/*************************************************************************/
#include <stdio.h>
#include <string.h>
#include "global612.h"
#define LOCAL_DEBUG_ENABLE 0
#include "mpTrace.h"
#include "mpapi.h"
#include "os.h"
#include "taskid.h"
#include "elf.h"

/*************************************************************************/
/***                              Macro                                ***/
/*************************************************************************/
/* FATAL ERROR */
#define KMOD_ERROR(s) \
                    do { \
                        mpDebugPrint(s); \
                        goto error; \
                    } while(1);
#define KMOD_ERROR_RET(s) \
                    do { \
                        mpDebugPrint(s); \
                        return -1; \
                    } while(1);

#define ELF_MAGIC "\x7f""ELF"
#define ENTRY_FUNC "testmain"

/* SYMBOL-STUFF */
typedef struct sym_ent {
    BYTE *name;
    DWORD addr;
} sym_t;

/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/
/*************************************************************************/
/***               Internally Visible Constants and Static Data        ***/
/*************************************************************************/
extern DWORD kernel_module_sarea;
extern DWORD kernel_module_sarea_end;
extern DWORD _gp;

#define KERNEL_MODULE_SAREA (DWORD)&kernel_module_sarea
#define KERNEL_MODULE_SAREA_END (DWORD)&kernel_module_sarea_end
#define KERNEL_M_SAREA_SIZE (KERNEL_MODULE_SAREA_END-KERNEL_MODULE_SAREA+1)

extern ST_KMODULEAPI * KmodGetkmodapiStart(void);
extern ST_KMODULEAPI * KmodGetkmodapiEnd(void);

#define MAX_NONFREE_PTR 10
static void * nonfreeptr[MAX_NONFREE_PTR];
static SBYTE freeIndex = 0;
/****************************************************************************
 **
 ** NAME:           KmodapiNextGet
 **
 ** PARAMETERS:     name, current
 **
 ** RETURN VALUES:  Address of function
 **
 ** DESCRIPTION:    Get the kmodule api section, this section records the API
 **                 function address.
 **
 ****************************************************************************/
static DWORD KmodapiNextGet(BYTE *name, ST_KMODULEAPI *current)
{    
    register DWORD end;

    if(!current)/* Start from first element */
        current = KmodGetkmodapiStart();
    else
        current++;

    end = (DWORD)KmodGetkmodapiEnd();/* Get the address of the end of kmodapi section */
    
    while((DWORD)current != end)
    {
        if(!strcmp(name, current->ptrName))/* Compare the string of name */
            return current->u32Function;
        else
            current ++;
    }

    return 0;/* not found */
}

/****************************************************************************
 **
 ** NAME:           sym_lookup
 **
 ** PARAMETERS:     sym, sym_val
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    get addr for a symbol
 **
 ****************************************************************************/
static SDWORD sym_lookup(BYTE *sym, DWORD *sym_val)
{
    DWORD tmp;

    if ( (tmp = KmodapiNextGet(sym, 0))!= 0 )
    {
        *sym_val = tmp;
        return PASS;
    }
    else
        return FAIL;
}

/****************************************************************************
 **
 ** NAME:           sym_valid_name
 **
 ** PARAMETERS:     symt
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    only local/global functions is valid.
 **
 ****************************************************************************/
static SDWORD sym_valid_name(Elf32_Sym *symt)
{
	if ((symt->st_name != 0) &&
	((ELF32_ST_BIND(symt->st_info) == STB_LOCAL) || (ELF32_ST_BIND(symt->st_info) == STB_GLOBAL))
	&& (ELF32_ST_TYPE(symt->st_info) == STT_FUNC))
	//&& (symt->st_size > 0) && (ELF32_ST_TYPE(symt->st_info) == STT_FUNC))
		return PASS;
	return FAIL;
}

/****************************************************************************
 **
 ** NAME:           kmod_allocateSection
 **
 ** PARAMETERS:     file, retBuf, size, offset
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Allocate memory for section.
 **
 ****************************************************************************/
static SDWORD kmod_allocateSection(STREAM * file, BYTE **retBuf, DWORD size, DWORD offset)
{
    DWORD readSize;

    if (Seek(file, offset) < 0)
    {   
        KMOD_ERROR_RET("File seek failed");
    }
    if ((*retBuf = (BYTE *)ext_mem_malloc(size)) == NULL)
    {
        KMOD_ERROR_RET("ext_mem_malloc() failed");
    }
    if ( !(readSize = FileRead(file, (BYTE *)*retBuf, size)) )
    {
        KMOD_ERROR_RET("File read error");
    }
    else if ( size != readSize )
    {
        MP_DEBUG2("Loader: need %d, return %d", size, readSize);
        KMOD_ERROR_RET("Loader: err, read not enough");
    }
    return PASS;
}

/****************************************************************************
 **
 ** NAME:           Loader_init
 **
 ** PARAMETERS:     fname
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Start to relocate elf and execute it.
 **
 ****************************************************************************/
SDWORD Loader_init(BYTE *fname, BOOL free, SDWORD argc, SBYTE *argv[])
{
    DRIVE *sDrv=NULL;
    STREAM *file=NULL;
    Elf32_Sym *symtab=NULL;
    BYTE *strtab=NULL;
    BYTE *shstrtab=NULL;
    BYTE *textSection=NULL;
    Elf32_Ehdr *elfhdr=NULL;
    Elf32_Shdr *shdr, *off_shdr=NULL;
    DWORD rel_sz;
    Elf32_Rel *rel=NULL;
    DWORD elf_sz, entry, size, addrOff, addrGpOff, readSize, symtabSize, totalsize, totalGPsize, StartAddress=0;
    SDWORD (*kmodEntry)(SDWORD, SBYTE **);
    SDWORD nr_entry, i, j;
    WORD *tmp;
    DWORD sym_count;
    // symbol table from module
    sym_t *sym_tab_mod=NULL;

    DWORD *rel_addr;
    Elf32_Shdr *rel_sect;//section for relocation
    DWORD sym_val;
    DWORD Ahl, tmp26;
    LONG tmpLongLong;
    SWORD signA;
    BOOL locationFlag;

    BYTE *sym;
    DWORD symidx;
    SDWORD result;

    /* open file */
    sDrv = DriveChange(SD_MMC);
    DirReset(sDrv);  /* go back to the beginning of root directory */

    if (FileSearch(sDrv, fname, "ELF", E_FILE_TYPE))
    {
        KMOD_ERROR("not found the file");
    }
    else
    {
        file = FileOpen(sDrv);
        if (file == NULL)
            KMOD_ERROR("open failed");
    }

    /* getting file length */
    if ((elf_sz = FileSizeGet(file)) == -1)
    {   
        KMOD_ERROR("stat failed");
    }
    mpDebugPrint("Module: %s", fname);

    /* Get Ehdr */
    if ((elfhdr = (Elf32_Ehdr *)ext_mem_malloc(sizeof(Elf32_Ehdr))) == NULL)
    {   
        KMOD_ERROR("elfhdr ext_mem_malloc() failed");
    }
    if ( !(readSize = FileRead(file, (BYTE *)elfhdr, sizeof(Elf32_Ehdr))) )
    {
        KMOD_ERROR("read elf Header err");
    }
    else if ( sizeof(Elf32_Ehdr) != readSize )
    {
        MP_DEBUG2("Loader: need %d, return %d", sizeof(Elf32_Ehdr), readSize);
        KMOD_ERROR("Loader: err, read not enough");
    }
    /* Check file format */
    if (strncmp(&elfhdr->e_ident[EI_MAG0], ELF_MAGIC, 4) ||
        (elfhdr->e_ident[EI_CLASS] != ELFCLASS32) ||
        (elfhdr->e_ident[EI_DATA] != ELFDATA2MSB) ||
        (elfhdr->e_type != ET_REL) ||
        (elfhdr->e_machine != EM_MIPS) ||
        (elfhdr->e_version != 1))
        KMOD_ERROR("Loader: Elf file format err");
    /* Read Shdr */
    if ( kmod_allocateSection(file, (BYTE **)&off_shdr, elfhdr->e_shnum*sizeof(Elf32_Shdr), (DWORD)elfhdr->e_shoff))
        KMOD_ERROR("Loader: Read shdr err");
    /* Read Symbol table and Shstrtab */
    //read sh strtab
    if ( kmod_allocateSection(file, &shstrtab, off_shdr[elfhdr->e_shstrndx].sh_size, (DWORD)off_shdr[elfhdr->e_shstrndx].sh_offset))
        KMOD_ERROR("Loader: Read shstrtab err");
    //search Symbol-Table
    for (i=0; i < elfhdr->e_shnum; i++)
    {
        if (off_shdr[i].sh_type == SHT_SYMTAB)
            shdr = &off_shdr[i];
    }
    MP_DEBUG1("Loader: symtab is in: 0x%x", shdr->sh_offset);

    if ( kmod_allocateSection(file, (BYTE **)&symtab, shdr->sh_size, (DWORD)shdr->sh_offset))
        KMOD_ERROR("Loader: Read symtab err");
    symtabSize = (DWORD)shdr->sh_size;
    
    /* Account the size of all non-Zero section, but exclude the .debug section */
    totalsize = 0;
    totalGPsize = 0;
    for (i=0; i < elfhdr->e_shnum; i++)
    {
        if ((off_shdr[i].sh_size > 0) &&
            (elfhdr->e_shstrndx != i) && /* not sh strtab */
            (off_shdr[i].sh_type != SHT_SYMTAB) && /* not symbol table */
            !strstr(&shstrtab[off_shdr[i].sh_name], "debug") && /* not debug section */
            !strstr(&shstrtab[off_shdr[i].sh_name], "stab") && /* not stab section */
            (off_shdr[i].sh_type != SHT_RELA) &&
            (off_shdr[i].sh_type != SHT_REL)) /* not relocation table */
        {
            //add size
            if (off_shdr[i].sh_flags & SHF_MIPS_GPREL)//in GP area
            {
                if (off_shdr[i].sh_size & 0x3)
                    totalGPsize += (off_shdr[i].sh_size & 0xFFFFFFFC) + 4;/* align to 4 bytes */
                else
                    totalGPsize += off_shdr[i].sh_size;
            }
            else
            {
                if (off_shdr[i].sh_size & 0x3)
                    totalsize += (off_shdr[i].sh_size & 0xFFFFFFFC) + 4;/* align to 4 bytes */
                else
                    totalsize += off_shdr[i].sh_size;
            }
        }
    }
    if (totalGPsize > KERNEL_M_SAREA_SIZE)
    {
        MP_DEBUG2("Loader: need GP size : %d, max is %d", totalGPsize, KERNEL_M_SAREA_SIZE);
        KMOD_ERROR("Loader: GP area is not enough");
    }
    
    mpDebugPrint("Loader: program size needs: %d", totalsize);
    /* Allocate program memory */
    if ((StartAddress = (DWORD)ext_mem_malloc(totalsize)) == NULL)
    {
        KMOD_ERROR("Loader: Program addr ext_mem_malloc() failed");
    }
    /* Read all section with non-Zero size and update section offset */
    addrOff = StartAddress;
    addrGpOff = KERNEL_MODULE_SAREA;
    for (i=0; i < elfhdr->e_shnum; i++)
    {
        //Get sectionname
        if ((off_shdr[i].sh_size > 0) &&
            (elfhdr->e_shstrndx != i) && /* not sh strtab */
            (off_shdr[i].sh_type != SHT_SYMTAB) && /* not symbol table */
            !strstr(&shstrtab[off_shdr[i].sh_name], "debug") && /* not debug section */
            !strstr(&shstrtab[off_shdr[i].sh_name], "stab") && /* not stab section */
            (off_shdr[i].sh_type != SHT_RELA) &&
            (off_shdr[i].sh_type != SHT_REL)) /* not relocation table */
        {
            if (strstr(&shstrtab[off_shdr[i].sh_name], "sbss"))
            {
                off_shdr[i].sh_offset = addrGpOff-KERNEL_MODULE_SAREA;/* update offset */
                if (off_shdr[i].sh_size & 0x3)
                    addrGpOff += (off_shdr[i].sh_size & 0xFFFFFFFC) + 4;/* align to 4 bytes */
                else
                    addrGpOff += off_shdr[i].sh_size;
            }
            else if (strstr(&shstrtab[off_shdr[i].sh_name], "bss"))
            {
                off_shdr[i].sh_offset = addrOff-StartAddress;/* update offset */
                if (off_shdr[i].sh_size & 0x3)
                    addrOff += (off_shdr[i].sh_size & 0xFFFFFFFC) + 4;/* align to 4 bytes */
                else
                    addrOff += off_shdr[i].sh_size;
            }
            else
            {
                if (Seek(file, off_shdr[i].sh_offset) < 0)
                {   
                    KMOD_ERROR_RET("File seek failed");
                }
                //read
                if (off_shdr[i].sh_flags & SHF_MIPS_GPREL)//in GP area
                {
                    if ( !(readSize = FileRead(file, (BYTE *)addrGpOff, off_shdr[i].sh_size)) )
                    {
                        KMOD_ERROR("Loader: File read error");
                    }
                    else if (off_shdr[i].sh_size != readSize)
                    {
                        MP_DEBUG2("Loader: need %d, return %d", off_shdr[i].sh_size, readSize);
                        KMOD_ERROR("Loader: err, read not enough");
                    }
                    off_shdr[i].sh_offset = addrGpOff-KERNEL_MODULE_SAREA;/* update offset */
                    if (off_shdr[i].sh_size & 0x3)
                        addrGpOff += (off_shdr[i].sh_size & 0xFFFFFFFC) + 4;/* align to 4 bytes */
                    else
                        addrGpOff += off_shdr[i].sh_size;
                }
                else
                {
                    if ( !(readSize = FileRead(file, (BYTE *)addrOff, off_shdr[i].sh_size)) )
                    {
                        KMOD_ERROR("Loader: File read error");
                    }
                    else if (off_shdr[i].sh_size != readSize)
                    {
                        MP_DEBUG2("Loader: need %d, return %d", off_shdr[i].sh_size, readSize);
                        KMOD_ERROR("Loader: err, read not enough");
                    }
                    /* find string table this moment */
                    if ((off_shdr[i].sh_type == SHT_STRTAB) && !(off_shdr[i].sh_flags) && (elfhdr->e_shstrndx != i))
                        strtab = (BYTE *)addrOff;
                    /* Find text section */
                    if (off_shdr[i].sh_flags & 0x0004)
                        textSection =  (BYTE *)addrOff;
                    off_shdr[i].sh_offset = addrOff-StartAddress;/* update offset */
                    if (off_shdr[i].sh_size & 0x3)
                        addrOff += (off_shdr[i].sh_size & 0xFFFFFFFC) + 4;/* align to 4 bytes */
                    else
                        addrOff += off_shdr[i].sh_size;
                }
            }
        }
    }
    /* Create symbol table */
    sym_count = 0;
    mpDebugPrint("Loader: [Create SymTab]");
    nr_entry = symtabSize/sizeof(Elf32_Sym);
    /* account size of the table */
    for (i=0; i<nr_entry; i++)
    {
        if (sym_valid_name(&symtab[i]) == PASS)
            sym_count++;
    }
    if ((sym_tab_mod = (sym_t *)ext_mem_malloc(sym_count*sizeof(sym_t))) == NULL)
    {   
        KMOD_ERROR("sym_tab_mod ext_mem_malloc() failed");
    }
    sym_count = 0;
    // create symbol-table of module
    for (i=0; i<nr_entry; i++)
    {
        if (sym_valid_name(&symtab[i]) == PASS)
        {
            sym_tab_mod[sym_count].name = &strtab[symtab[i].st_name];
            sym_tab_mod[sym_count].addr = (DWORD)(symtab[i].st_value + textSection);
            MP_DEBUG2("Loader: Symbol: %s (0x%x)", sym_tab_mod[sym_count], sym_tab_mod[sym_count].addr);
            sym_count++;
        }
    }

    /* relocation */
    /* search relocation-sections */
    for(i=0; i<elfhdr->e_shnum; i++, rel_sz=0)
    {
        if ((off_shdr[i].sh_type != SHT_RELA) && (off_shdr[i].sh_type != SHT_REL))
            continue;
        mpDebugPrint("Loader: section name: %s", &shstrtab[off_shdr[i].sh_name]);
        if ( strstr(&shstrtab[off_shdr[i].sh_name], "debug") )
            continue;
        if ( strstr(&shstrtab[off_shdr[i].sh_name], "stab") )
            continue;
        rel_sz = off_shdr[i].sh_entsize;
        //read relocation table
        if ( kmod_allocateSection(file, (BYTE **)&rel, (DWORD)off_shdr[i].sh_size, (DWORD)off_shdr[i].sh_offset))
            KMOD_ERROR("Loader: Read relocation err");

        MP_DEBUG5("Loader: rel_section: %d(0x%x), rel_ent_size: %d -> rel_type: %d, rel_ent_nr: %d",
        i, off_shdr[i].sh_offset, rel_sz, off_shdr[i].sh_type, off_shdr[i].sh_size/rel_sz);

        for(j=0;j<off_shdr[i].sh_size/rel_sz;j++)
        {
            MP_DEBUG2("Loader: Relocation-Nr: %d/%d", j+1, off_shdr[i].sh_size/rel_sz);
            
            // do relocation
            //section for relocation
            rel_sect = (Elf32_Shdr*)((DWORD)off_shdr + elfhdr->e_shentsize * off_shdr[i].sh_info);

            //relocation addr
            if (rel_sect->sh_flags & SHF_MIPS_GPREL)//in GP area
                rel_addr = (DWORD*)(KERNEL_MODULE_SAREA + rel_sect->sh_offset + rel[j].r_offset);
            else
                rel_addr = (DWORD*)(StartAddress + rel_sect->sh_offset + rel[j].r_offset);
            MP_DEBUG3("Loader: rel_offset: 0x%x, SYM: 0x%x, TYPE: 0x%x",
            rel[j].r_offset, ELF32_R_SYM(rel[j].r_info), ELF32_R_TYPE(rel[j].r_info));

            MP_DEBUG2("Loader: sect_for_rel_ofs: 0x%x, SymTab-Sect-IDX: %d", rel_sect->sh_offset, off_shdr[i].sh_link);

            // get addr of symbol
            symidx = ELF32_R_SYM(rel[j].r_info);
            MP_DEBUG1("Loader: SymTab-IDX: %d", symidx);
            MP_DEBUG1("Loader: symtab[symidx].st_shndx: 0x%x", symtab[symidx].st_shndx);
            if (!symtab[symidx].st_shndx)
            {
                //external symbol
                locationFlag = FALSE;
                MP_DEBUG("Loader: External Symbol");
                sym = &strtab[symtab[symidx].st_name];
                MP_DEBUG1("Loader: Symbol: \"%s\"", sym);
                if (sym_lookup(sym, &sym_val) == FAIL)
                {
                    mpDebugPrint("Error: Symbol: \"%s\"", sym);
                    KMOD_ERROR("Error: Unknown Symbol!");
                }
                MP_DEBUG1("Loader: ext. Symbol-Addr: 0x%x", sym_val);
            }
            else if(symtab[symidx].st_shndx == SHN_COMMON || symtab[symidx].st_shndx == SHN_MIPS_SCOMMON)
            {
                KMOD_ERROR("Error: not support the COMMON SYMBOL, please add -fno-common parameter for gcc");
            }
            else
            {
                //internal symbol
                locationFlag = TRUE;
                MP_DEBUG("Loader: Internal Symbol");
                shdr = (Elf32_Shdr*)((DWORD)off_shdr + elfhdr->e_shentsize * symtab[symidx].st_shndx);
                if (shdr->sh_flags & SHF_MIPS_GPREL)//in GP area
                    sym_val = symtab[symidx].st_value + (DWORD)(KERNEL_MODULE_SAREA + shdr->sh_offset);
                else
                    sym_val = symtab[symidx].st_value + (DWORD)(StartAddress + shdr->sh_offset);
                MP_DEBUG2("Loader: int. Symbol-Addr: 0x%x, Symtab-Value: %d", sym_val, symtab[symidx].st_value);	
            }
            MP_DEBUG1("Loader: *rel_addr = 0x%x", *rel_addr);
            switch (ELF32_R_TYPE(rel[j].r_info))
            {
                case R_MIPS_16:	    /* S + sign-extend(A) */
                /*
                * There shouldn't be R_MIPS_16 relocs in kernel objects.
                */
                KMOD_ERROR("kldload: unexpected R_MIPS_16 relocation");
                case R_MIPS_32: /* S + A */
                tmpLongLong = (LONG)sym_val + *rel_addr;
                *rel_addr = (DWORD)tmpLongLong;
                MP_DEBUG2("Loader: R_MIPS_32: rel_addr: 0x%x(0x%x)", rel_addr, *rel_addr);
                break;
                case R_MIPS_REL32:		/* A - EA + S */
                /*
                * There shouldn't be R_MIPS_REL32 relocs in kernel objects?
                */
                KMOD_ERROR("kldload: unexpected R_MIPS_REL32 relocation");
                case R_MIPS_26: /* internal (((A << 2) | (P & 0xf0000000)) + S) >> 2 */
                /* external (sign-extend(A << 2) + S) >> 2 */
                tmp26 = *rel_addr&0xFC000000;//store OP code
                *rel_addr &= *rel_addr&0x3FFFFFF;
                if (locationFlag)
                {
                    *rel_addr = (((*rel_addr << 2) | ((DWORD)rel_addr & 0xF0000000)) + (sym_val&0x3FFFFFF)) >> 2;
                    *rel_addr = *rel_addr & 0x3FFFFFF | tmp26;//restore OP code
                }
                else
                {
                    *rel_addr = (DWORD)((SDWORD)(*rel_addr << 2) + (sym_val&0x3FFFFFF)) >> 2;
                    *rel_addr = *rel_addr & 0x3FFFFFF | tmp26;//restore OP code
                }
                MP_DEBUG3("Loader: R_MIPS_26: rel_addr: 0x%x(0x%x), locationFlag=%d", rel_addr, *rel_addr, locationFlag);
                break;
                case R_MIPS_HI16:
                /* extern/local: ((AHL + S) - ((short)(AHL + S)) >> 16 */
                /* _gp_disp: ((AHL + GP - P) - (short)(AHL + GP - P)) >> 16 */
                tmp = (WORD *)rel_addr + 1;//Save the address
                break;
                case R_MIPS_LO16:
                /* extern/local: AHL + S */
                if (tmp)
                {
                    Ahl = (((DWORD)*tmp)<<16) | (*rel_addr&0xFFFF);
                    *tmp = (WORD)((Ahl + sym_val)>>16);//modify %hi
                    if (((WORD)(Ahl + sym_val))&0x8000)//Minus
                        *tmp += 1;
                    MP_DEBUG1("Loader: R_MIPS_LO16: hi:(0x%x)", *tmp);
                    tmp = (WORD *)rel_addr + 1;
                    *tmp = (WORD)(Ahl + sym_val);//modify %lo
                }
                else
                {
                    Ahl = *rel_addr&0xFFFF;
                    tmp = (WORD *)rel_addr + 1;
                    *tmp = (WORD)(Ahl + sym_val);//modify %lo
                }
                tmp = NULL;//reset
                MP_DEBUG("Loader: R_MIPS_LO16: rel_addr: 0x%x", rel_addr);
                MP_DEBUG("Loader: R_MIPS_LO16: *rel_addr 0x%x", *rel_addr);
                /* _gp_disp: AHL + GP - P + 4 */
                break;
                case R_MIPS_GPREL16:
                /* extern: sign-extend(A) + S + GP */
                /* local: sign-extend(A) + S + GP0 - GP */
                MP_DEBUG2("Loader: &_gp = 0x%x, sym_val = 0x%x", &_gp, sym_val);
                tmp = (WORD *)rel_addr + 1;
                if ( sym_val > &_gp )
                    *tmp = (WORD)((SWORD)*tmp + (sym_val - (DWORD)&_gp));
                else
                    *tmp = (WORD)((SWORD)*tmp - (SWORD)((DWORD)&_gp - sym_val));
                #if 0
                if (locationFlag)
                {
                    tmp = (WORD *)*rel_addr;
                    *tmp = (WORD)((SWORD)*tmp + sym_val);//GP0 = GP
                }
                else
                {
                    tmp = (WORD *)*rel_addr;
                    *tmp = (WORD)((SWORD)*tmp + (WORD)&_gp + (WORD)sym_val);
                }
                #endif
                MP_DEBUG2("Loader: R_MIPS_GPREL16: rel_addr: 0x%x(0x%x)", rel_addr, *rel_addr);
                //KMOD_ERROR("kldload: unexpected R_MIPS_GPREL16 relocation, please recompile your code with -G 0 argument");
                case R_MIPS_LITERAL: /* sign-extend(A) + L */
                break;
                case R_MIPS_GOT16: /* external: G */
                /* local: tbd */
                case R_MIPS_PC16: /* sign-extend(A) + S - P */
                case R_MIPS_CALL16: /* G */
                case R_MIPS_GPREL32: /* A + S + GP0 - GP */
                case R_MIPS_GOTHI16: /* (G - (short)G) >> 16 + A */
                case R_MIPS_GOTLO16: /* G & 0xffff */
                case R_MIPS_CALLHI16: /* (G - (short)G) >> 16 + A */
                case R_MIPS_CALLLO16: /* G & 0xffff */
                /*
                * There shouldn't be this relocs in kernel objects.
                */
                KMOD_ERROR("kldload: unexpected this relocation, please recompile your code with -G 0 argument");
                default:
                KMOD_ERROR("Error: wrong Reloc-Type");
            }
            MP_DEBUG("Loader: -------------------------------------");
        }
        ext_mem_free(rel);
        rel = NULL;
    }
    //free useless merory->symbol table, sh strtab, elfhdr, shdr
    ext_mem_free(elfhdr);
    ext_mem_free(shstrtab);
    ext_mem_free(symtab);
    FileClose(file);
    file = NULL;
    kmodEntry = NULL;
    //exec
    // search for ENTRY_FUNC, set main-entry-addr
    for (i=0; i<sym_count; i++)
    {
        if (!strcmp(sym_tab_mod[i].name, ENTRY_FUNC))
        {
            kmodEntry = (void *)sym_tab_mod[i].addr;
            break;
        }
    }
    ext_mem_free(sym_tab_mod);
    sym_tab_mod = NULL;
    // ENTRY_FUNC not found, set main-entry-addr to beginning of .text
    if (kmodEntry == NULL)
    {
        MP_DEBUG("Loader: No Main-Entry! Set entry to beginning of .text!");
        kmodEntry = (void *)textSection;
        MP_DEBUG1("Loader: Entry: 0x%x", kmodEntry);
    }	

    mpDebugPrint("Loader: [Start]");
    mpDebugPrint("Loader: kmodEntry: 0x%x", kmodEntry);
    mpDebugPrint("Loader: <output>");
    result = kmodEntry(argc, argv);
    mpDebugPrint("Loader: </output");
    mpDebugPrint("Loader: > Done!");
    if (free)
        ext_mem_free(StartAddress);//free
    else
    {
        mpDebugPrint("No free, the memory ptr was saved in %d slot", freeIndex);
        nonfreeptr[freeIndex++] = (void *)StartAddress;
    }
    //return
    return result;
error:
    if (elfhdr)
        ext_mem_free(elfhdr);
    if (shstrtab)
        ext_mem_free(shstrtab);
    if (symtab)
        ext_mem_free(symtab);
    if (StartAddress)
        ext_mem_free(StartAddress);
    if (rel)
        ext_mem_free(rel);
    if (sym_tab_mod)
        ext_mem_free(sym_tab_mod);
    if (file)
        FileClose(file);
    return FAIL;
}

