/*
 * The MEMORY command describes the location and size of blocks of memory
 * in the target. You can use it to describe which memory regions may be
 * used by the linker, and which memory regions it must avoid.
 */
MEMORY
{
  /*
   * Memory with CPU cache.
   *6M CPU SRAM
   */
  ram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = (6 * 1024 * 1024)
  /*
   * Memory without CPU cache
   * 6M CPU SRAM
  */
  ram_nocache (wxa!ri) : ORIGIN = 0x40000000, LENGTH = (6 * 1024 * 1024)
}

PROVIDE( _ram_end    = ORIGIN(ram) + LENGTH(ram) );
PROVIDE( _stack_size = 1 << 17 );

/*
 * The OUTPUT_ARCH command specifies the machine architecture where the
 * argument is one of the names used in the Kendryte library.
 */
OUTPUT_ARCH( "riscv" )

/*
 * The ENTRY command specifies the entry point (ie. first instruction to
 * execute). The symbol _start is defined in crt0.S
 */
ENTRY(_start)

/*
 * The linker only pays attention to the PHDRS command when generating
 * an ELF output file. In other cases, the linker will simply ignore PHDRS.
 */
PHDRS
{
  DATA      PT_LOAD;
  DYN_DATA  PT_NULL;
}

/*
 * This is where we specify how the input sections map to output
 * sections.
 */
SECTIONS
{
  .text.start :
  {
    KEEP( *(.text.start) )
  } > ram : DATA

  .init :
  {
    KEEP (*(SORT_NONE(.init)))
  } > ram : DATA

  .text :
  {
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(.text .stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } > ram : DATA

  .fini :
  {
    KEEP (*(SORT_NONE(.fini)))
  } > ram : DATA

  /* Read-only data segment */
  .rodata :
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  } > ram : DATA

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > ram : DATA
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > ram : DATA
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > ram : DATA

  .ctors :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } > ram : DATA
  .dtors :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  } > ram : DATA

  .data :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  } > ram : DATA

  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata :
  {
    __global_pointer$ = . + 0x800;
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata .srodata.*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  } > ram : DATA
  _edata = .; PROVIDE (edata = .);
  /*. = ALIGN(64);*/
  __bss_start = .;
  .sbss :
  {
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  } > ram : DYN_DATA
  .bss :
  {
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
  } > ram : DYN_DATA
  __bss_end = .;

  PROVIDE( _tls_data = ABSOLUTE(.) );
  /*
   * Thread Local Storage (TLS) are per-thread global variables.
   * Compilers such as GCC provide a __thread keyword to mark global
   * variables as per-thread. Support is required in the program loader
   * and thread creator.
   */

  /* Thread-local data segment, .tdata (initialized tls). */
  .tdata :
  {
    KEEP( *(.tdata.begin) )
    *(.tdata .tdata.*)
    *(.gnu.linkonce.td.*)
    KEEP( *(.tdata.end) )
  } > ram : DATA

  /* Thread-local bss segment, .tbss (zero-initialized tls). */
  .tbss :
  {
    *(.tbss .tbss.*)
    *(.gnu.linkonce.tb.*)
    KEEP( *(.tbss.end) )
  } > ram : DYN_DATA

  /*
   * End of uninitalized data segement
   *
   * Actually the stack needs 16B alignment, and it won't hurt to also slightly
   * increase the alignment to 32 or even 64 (cache line size).
   *
   * Align _heap_start to cache line size
   */
  . = ALIGN(64);
  PROVIDE( _end = ABSOLUTE(.) );
  /* Leave 2 holes for stack & TLS, the size can set in kconfig */
  PROVIDE( _heap_start = ABSOLUTE(.) + _stack_size * 5 );
  PROVIDE( _tp0 = (_end + 63) & (-64) );
  PROVIDE( _tp1 = _tp0 + _stack_size );
  PROVIDE( _sp0 = _tp1 + _stack_size );
  PROVIDE( _sp1 = _sp0 + _stack_size );
  /* Heap end is at the end of memory, the memory size can set in kconfig */
  PROVIDE( _heap_end = _ram_end );


  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to the beginning
   * of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}

