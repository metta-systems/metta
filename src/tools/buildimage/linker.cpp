//---------------------------------------------------------------------------------------
//
// ELF object files linker for Metta.
//
// Copyright (C) 2001, 2009, Stanislav Karchebny <berkus@exquance.com>
//
// Code portions copyright Greg Law <glaw@nexwave-solutions.fr>
// Code portions copyright Dave Poirier <instinc@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Produces a component file in comp format from a series of ELF
// object files. Some special features for Odin:
// + .text and .data are linked at zero
//
//---------------------------------------------------------------------------------------

#define __sjofn_VERSION "1.0.4"

#include <unistd.h>   // getcwd()/access()/chdir()
#include <string.h>   // str*()
#include <stdarg.h>   // va_*
#include <stdlib.h>   // atoi()
#include <stdio.h>
#include "mt.h"
#include "ELF.h"
#include "comp.h"     // comp_desc
#include "version"
#include "options.h"


#ifdef __DEBUG__
#define DEBUG(x) fprintf ## x ;
#else
#define DEBUG(x)
#endif

#define DEF_TEXT_ALIGN 16
#define DEF_DATA_ALIGN 16

#define SEARCH_DIRS_MAX 10


//------------------------------------------------------------------------
// Types Definition
//------------------------------------------------------------------------

typedef unsigned long  dword;
typedef unsigned short word;
typedef unsigned char  byte;

struct local;
struct section;

typedef struct object    // ELF object
{
   object     *next;     // next object in the list
   char       *filename; // name of the object file
   FILE       *fp;       // file pointer
   local      *locals;   // list of object's locals
   section    *sections; // list of object's sections

   int open( char *filename );
   int add_section( char *name, Elf32_Shdr *header, Elf32_Word secndx, char *data );
   int add_local( char *name, Elf32_Sym *sym );

   section *get_section_by_type( Elf32_Word type );
};

typedef struct section   // object section
{
   section    *next;     // next section in the object's sections list
   section    *ord_next; // next section in the ordered sections list
   object     *parent;   // object owning this section
   char       *name;     // name of the section (pointer into object's strtab)
   Elf32_Shdr *header;   // section header      (pointer into object's shtab)
   char       *contents; // section contents
   Elf32_Word  offset;   // new section offset  (after rearrangement)
};

typedef struct global
{
   global    *next;      // next global in the list
   object    *parent;    // object owning this global
   char      *name;      // symbol's name (pointer into object's strtab)
   Elf32_Sym *sym;       // the global symbol itself
};

typedef struct local
{
   local     *next;      // next local in the list
   char      *name;      // symbol's name (pointer into object's strtab)
   Elf32_Sym *sym;       // the local itself
};

template<class T>
class linked_list_t
{
	T* head;
public:
	linked_list_t<T>()
	{ 
		head = NULL; 
	}

	void append(T* item)
	{
		assert(item);
		if(!head)
		 	head = item;
		else
		{
			T* p = head;
			while (p->next)
				p = p->next;
			p->next = item;
		}
	}

	void prepend(T* item)
	{
		assert(item);
		item->next = head;
		head = item;
	}
};


//------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------

int add_global( char *name, Elf32_Sym *sym, object *parent );
int add_file( char *filename );
int arrange_sections();
int adjust_symbols_values();
int register_symbols();
int resolve_references();
int get_global_value( char *name, Elf32_Word *value );
int apply_relocations();
int generate_mt();
// output
int output_sections( char *filename );
int output_metadata( char *filename );
int produce_xrefs( char *filename );
int produce_bochs_sym( char *filename );
// command line
int opt_filename_option( char *cmd, char *param, int extra );
int opt_linker_options( char *cmd, char *param, int extra );
int opt_mt_option( char *cmd, char *param, int extra );
int opt_mt_dir_option( char *cmd, char *param, int extra );
int opt_text_align_option( char *cmd, char *param, int extra );
int opt_data_align_option( char *cmd, char *param, int extra );
// support
void error( const char *mess, ... );
void warning( const char *mess, ... );


//------------------------------------------------------------------------
// Initialized Data
//------------------------------------------------------------------------

char header[]       = "sjofn - version " __sjofn_VERSION " - compiled " __DATE__ " " __TIME__ "\n";
char help_head[]    = "Copyright (c) 2001 Stanislav Karchebny <berk@madfire.net>\n"
                      "Distributed under BSD License.\n"
                      "usage: sjofn [options|files]+\n"
                      "where: options are\n";
char help_tail[]    = "\n"
                      "   files are ELF objects to be linked\n"
                      "\n"
                      "sample invocation:\n"
                      "   sjofn --make-orb -o orb.img jmp_tbl.o impl.o mem.o\n"
                      "\n";



#define MAX_ALIGNMENT   512

char default_output[] = "odin.bin";
char *output_filename = default_output;

Elf32_Word text_alignment = DEF_TEXT_ALIGN;
Elf32_Word data_alignment = DEF_DATA_ALIGN;

char *mt_file = NULL;
char *search_dirs[SEARCH_DIRS_MAX];
int   nsearch_dirs = 0;

FILE *fp_output = NULL;

int linker_options = 0;
#define OPT_SHOW_HELP      0x01
#define OPT_MAKE_ORB       0x02
#define OPT_ABORT_ON_WARN  0x04
#define OPT_VERBOSE        0x08
#define OPT_XREF           0x10
#define OPT_XREF_NO_ABS    0x20
#define OPT_BOCHS_SYM      0x40

int errors = 0,
    warnings = 0;

#define countof(array) (sizeof(array)/sizeof(array[0]))

opt_option options[] =
{
   { 'o', "output",             1, opt_filename_option,    0,                 "set output file name",                        "file" },
   { 'i', "mt-dir",             1, opt_mt_dir_option,      SEARCH_DIRS_MAX,   "list search dirs for mt .inf files (%d max)", "dir"  },  // \   order
   {  0 , "mt",                 1, opt_mt_option,          0,                 "method table file name",                      "file" },  // / dependency
   { 'h', "help",               0, opt_linker_options,     OPT_SHOW_HELP,     "show usage information"                              },
   {  0 , "make-orb",           0, opt_linker_options,     OPT_MAKE_ORB,      "generate orb format file"                            },
   { 'w', "abort-on-warning",   0, opt_linker_options,     OPT_ABORT_ON_WARN, "error exit even on warnings"                         },
   { 'v', "verbose",            0, opt_linker_options,     OPT_VERBOSE,       "be verbose"                                          },
   { 'x', "xref",               0, opt_linker_options,     OPT_XREF,          "output symbols with their values"                    },
   { 'b', "bochs-sym",          0, opt_linker_options,     OPT_BOCHS_SYM,     "generate symbols for bochs"                          },
	{  0 , "no-abs",             0, opt_linker_options,     OPT_XREF_NO_ABS,   "do not output abs data in xrefs"                     },
   {  0 , "text-align",         1, opt_text_align_option,  0,                 "specify .text sections alignment"                    },
   {  0 , "data-align",         1, opt_data_align_option,  0,                 "specify .data sections alignment"                    }
};

// TODO: parse a simplistic linker script instead of hardcoded section names

// arrange sections of code in this order
char *text_sections_names[] =
{
   // linked at 0
   ".text",
   ".gnu.linkonce.",
   "..text.end" // a special section that marks end of text section
                 // (forces align and text_end symbol defined)
};

// arrange sections of data in this order
char *data_sections_names[] =
{
   // linked at 0
   ".data",
   ".rodata",
   ".ctors",
   "..data.end", // a special section that marks end of data section
                 // (forces align and data_end symbol defined)
   "COMMON",
   ".bss",
   "..bss.end"   // a special section that marks end of bss section
                 // (forces align and bss_end symbol defined)
};

linked_list_t<object>  root_obj;
global  *root_global   = NULL;  // global symbols

section *text_sections = NULL,
        *data_sections = NULL;

Elf32_Off text_offset = 0,      // virtual sections offsets (that is, in memory)
          data_offset = 0;
Elf32_Off data_file_offset = 0; // physical offset of data section (in file)

comp_desc comp;                 // a component meta-data

Elf32_Sym  text_end_sym = { 0, 0, 0, ELF32_ST_INFO(STB_GLOBAL,STT_NOTYPE), 0, SHN_ABS },
           data_end_sym = { 0, 0, 0, ELF32_ST_INFO(STB_GLOBAL,STT_NOTYPE), 0, SHN_ABS },
           bss_end_sym  = { 0, 0, 0, ELF32_ST_INFO(STB_GLOBAL,STT_NOTYPE), 0, SHN_ABS },
           img_end_sym  = { 0, 0, 0, ELF32_ST_INFO(STB_GLOBAL,STT_NOTYPE), 0, SHN_ABS };


//------------------------------------------------------------------------
// Uninitialized Data
//------------------------------------------------------------------------

// TODO: use new/delete, like the real thing!
mt   mtbl;                   // MUST be adjacent with the next array
char trash_space[ 20000 ];   // this space gets trashed by the method table


//------------------------------------------------------------------------
// Main Function
//------------------------------------------------------------------------

int main(int argc, char **argv)
{
   fprintf( stdout, header );

   DEBUG(( stderr, "main: parsing command line\n" ));
   if( parse_cmdline( argc, argv, options, countof(options) ) ) goto bail_out;
   if( warnings && (linker_options & OPT_ABORT_ON_WARN) ) goto bail_out;

   if( linker_options & OPT_SHOW_HELP )
   {
      help_msg( help_head, help_tail, options, countof(options) );
      goto bail_out;
   }

   if( !root_obj ) { error( "no input files" ); goto bail_out; }

   if( arrange_sections() ) goto bail_out;
   if( warnings && (linker_options & OPT_ABORT_ON_WARN) ) goto bail_out;

   adjust_symbols_values();
   register_symbols();
   resolve_references();

   if( warnings && (linker_options & OPT_ABORT_ON_WARN) ) goto bail_out;

   apply_relocations();

   if( warnings && (linker_options & OPT_ABORT_ON_WARN) ) goto bail_out;

   if( !(linker_options & OPT_MAKE_ORB) ) generate_mt();

   if( errors || warnings && (linker_options & OPT_ABORT_ON_WARN) ) goto bail_out;

   output_sections( output_filename );
   if( !(linker_options & OPT_MAKE_ORB) ) output_metadata( output_filename );
   if( warnings && (linker_options & OPT_ABORT_ON_WARN) ) goto bail_out;

   if( linker_options & OPT_XREF ) produce_xrefs( output_filename );
   if( linker_options & OPT_BOCHS_SYM ) produce_bochs_sym( output_filename );

   if( errors || (warnings && (linker_options & OPT_ABORT_ON_WARN)))
      remove(output_filename);
   else if( linker_options & OPT_VERBOSE )
   {
      fprintf(stdout, "\n"
                      "******************************************************\n"
                      "** %s\n"
                      "**\n",
                      output_filename );

      if( linker_options & OPT_MAKE_ORB )
         fprintf( stdout, "** ORB generated as:\n" );
      else
         fprintf( stdout, "** Component generated as:\n" );

      fprintf(stdout, "** Text at 0x%08x, size 0x%08x\n"
                      "** Data at 0x%08x, size 0x%08x\n"
                      "** BSS  at 0x%08x, size 0x%08x\n",
                      comp.text.start, comp.text.size,
                      comp.data.start, comp.data.size,
                      comp.bss.start,  comp.bss.size
             );

      if( !(linker_options & OPT_MAKE_ORB) )
         fprintf( stdout, "** MTbl at 0x%08x, size 0x%08x\n", comp.mt.start, comp.mt.size );

      fprintf(stdout, "******************************************************\n"
                      "\n" );
   }

bail_out:
   //global_destruction(); // who cares? we exit anyway...

   if( errors || warnings )
      fprintf( stderr, "*** %d error(s), %d warning(s)\n", errors, warnings );

   if( warnings && (linker_options & OPT_ABORT_ON_WARN) ) return errors + warnings;

   return errors;
}


int arrange_sections()
{
   object  *p;
   section *s, *last;

   DEBUG(( stderr, "arrange_sections: arranging text sections\n" ));

   comp.text.start = text_offset = 0;

   // arrange text sections
   for( unsigned int i = 0; i < countof(text_sections_names); i++ )
   {
      if( strcmp( text_sections_names[i], "..text.end" ) == 0 )
      {
         text_offset = text_end_sym.st_value = (text_offset + text_alignment - 1) & -text_alignment;
         add_global( "text_end", &text_end_sym, NULL );

         DEBUG(( stderr, "text ends at %d\n", text_offset ));
         continue;
      }

      // traverse all sections in all objects
      p = root_obj;
      while( p )
      {
         s = p->sections;
         while( s )
         {
            if( strcmp( s->name, text_sections_names[i] ) == 0 )
            {
               // append section to text_sections list
               DEBUG(( stderr, "adding text_section %s at section offset %d, size %d\n", s->name, text_offset, s->header->sh_size ));

               s->ord_next = NULL;
               s->offset = text_offset;
               text_offset += s->header->sh_size;
               text_offset = (text_offset + text_alignment - 1) & -text_alignment;

               if( !text_sections )
               {
                  text_sections = s;
               }
               else
               {
                  last = text_sections;
                  while(last->ord_next) last = last->ord_next;
                  last->ord_next = s;
               }
               break;
            }
            s = s->next;
         }
         p = p->next;
      }
   }

   DEBUG(( stderr, "arrange_sections: arranging data sections\n" ));

   comp.text.size = comp.data.start = data_file_offset = text_offset;

   // arrange data sections
   for( unsigned int i = 0; i < countof(data_sections_names); i++ )
   {
      if( strcmp( data_sections_names[i], "..data.end" ) == 0 )
      {
         data_offset = data_end_sym.st_value = (data_offset + data_alignment - 1) & -data_alignment;
         add_global( "data_end", &data_end_sym, NULL );

         comp.data.size = data_offset;
         comp.bss.start = comp.mt.start = comp.data.start + comp.data.size;

         DEBUG(( stderr, "data ends at %d\n", data_offset ));
         continue;
      }

      if( strcmp( data_sections_names[i], "..bss.end" ) == 0 )
      {
         data_offset = bss_end_sym.st_value = (data_offset + data_alignment - 1) & -data_alignment;
         add_global( "bss_end", &bss_end_sym, NULL );

         comp.bss.size = data_offset - comp.data.size;
         comp.mt.size = 0;

         DEBUG(( stderr, "bss ends at %d\n", data_offset ));
         continue;
      }

      // traverse all sections in all objects
      p = root_obj;
      while( p )
      {
         s = p->sections;
         while( s )
         {
            if( strcmp( s->name, data_sections_names[i] ) == 0 )
            {
               // append section to data_sections list
               DEBUG(( stderr, "adding data_section %s at section offset %d, size %d\n", s->name, data_offset, s->header->sh_size ));

               s->ord_next = NULL;
               s->offset = data_offset;
               data_offset += s->header->sh_size;
               data_offset = (data_offset + data_alignment - 1) & -data_alignment;

               if( !data_sections )
               {
                  data_sections = s;
               }
               else
               {
                  last = data_sections;
                  while(last->ord_next)last = last->ord_next;
                  last->ord_next = s;
               }
               break;
            }
            s = s->next;
         }
         p = p->next;
      }
   }

   img_end_sym.st_value = text_offset + data_offset;
   add_global( "img_end", &img_end_sym, NULL );
   DEBUG(( stderr, "the whole image ends at %d\n", img_end_sym.st_value ));

   DEBUG(( stderr, "arrange_sections: finished\n" ));
   return 0;
}


int output_sections( char *filename )
{
   DEBUG(( stderr, "output_sections(%s):\n", filename ));
   int zero = 0;
   int pad;

   FILE *fp = fopen( filename, "wb" );
   if( !fp ) { error( "could not output to '%s', check permissions", filename ); return 1; }

   // output text sections first
   section *s = text_sections;
   while( s )
   {
      if( s->header->sh_size != 0 ) // don't write empty sections, no matter how important they are
      {
         pad = s->offset - ftell(fp);
         if( pad < 0 )
         {
            error( "text section offset is insane! (virtual offset %d, real offset %d)", s->offset, ftell(fp) );
            return 1;
         }

         if( pad > 0 )
         {
            DEBUG(( stderr, "output_sections: padding text section %s with %d bytes to offset %d\n", s->name, pad, s->offset ));
            while(pad--) fwrite( &zero, 1, 1, fp );
         }

         if( fwrite( s->contents, s->header->sh_size, 1, fp ) != 1 ) { error("couldn't write to '%s'", filename); return 1; }
      }

      s = s->ord_next;
   }

   // output data sections
   s = data_sections;
   while( s )
   {
      if( s->header->sh_type != SHT_NOBITS )
      {
         if( s->header->sh_size != 0 ) // don't write empty sections
         {
            pad = s->offset - (ftell(fp) - data_file_offset);
            if( pad < 0 )
            {
               error( "data section offset is insane! (virtual offset %d, real offset %d, physical offset %d)", s->offset, ftell(fp), data_file_offset );
               return 1;
            }

            if( pad > 0 )
            {
               DEBUG(( stderr, "output_sections: padding data section %s with %d bytes to offset %ld\n", s->name, pad, ftell(fp)+pad ));
               while(pad--) fwrite( &zero, 1, 1, fp );
            }

            if( fwrite( s->contents, s->header->sh_size, 1, fp ) != 1 ) { error("couldn't write to '%s'", filename); return 1; }
         }
      }
      s = s->ord_next;
   }

   // final padding
   pad = comp.bss.start - ftell(fp);
   if( pad < 0 )
   {
      error( "final offset is insane! (virtual offset %d, real offset %d)", comp.bss.start, ftell(fp) );
      return 1;
   }

   if( pad > 0 )
   {
      DEBUG(( stderr, "output_sections: final padding with %d bytes to offset %ld\n", pad, ftell(fp)+pad ));
      while(pad--) fwrite( &zero, 1, 1, fp );
   }

   fclose( fp );
   DEBUG(( stderr, "output_sections finished\n" ));
   return 0;
}


int output_metadata( char *filename )
{
   DEBUG(( stderr, "output_metadata(%s):\n", filename ));

   int zero[4] = { 0, 0, 0, 0 };

   FILE *fp = fopen( filename, "ab" );
   if( !fp ) { error( "could not output to '%s', check permissions", filename ); return 1; }
   fseek( fp, 0, SEEK_END );

   // -- Append the MT

   comp.mt.start = ftell( fp );
   comp.mt.size  = mtbl.sz();

   DEBUG(( stderr, "output_metadata: mt @ %d, %d bytes\n", comp.mt.start, comp.mt.size ));

   fwrite( &mtbl, mtbl.sz(), 1, fp );

   // -- Write comp name (experimental)

   fwrite( filename, strlen(filename)+1, 1, fp );

   // -- Align comp on 16 bytes boundary

   Elf32_Off pos = ftell( fp );

   if( pos & 15 )
      fwrite( &zero, 16 - ( pos & 15 ), 1, fp );

   pos = ftell( fp );
//   assert( ( pos & 15 ) == 0 );

   // -- Finally, append the meta-data

   comp.version = __odin_VERSION;
   fwrite( &comp, sizeof( comp_desc ), 1, fp );

   fclose( fp );
   DEBUG(( stderr, "output_metadata finished\n" ));
   return 0;
}

// method table generator helper function
unsigned int count( char *str, char to_count )
{
   unsigned int count = 0;
   while( (str = strchr( str, to_count )) )
   {
      str++;
      count++;
   }
   return count;
}

// recursive helper function to generate method table
int fill_methods( char *filename )
{
   DEBUG(( stderr, "fill_methods(%s):\n", filename ));

   FILE *fp = fopen( filename, "rb" );
   if( !fp ) { error( "couldn't open inf file '%s'", filename ); return 1; }

   fseek( fp, 0, SEEK_END );
   Elf32_Word size = ftell(fp);
   fseek( fp, 0, SEEK_SET );

   char *table = new char [size];
   if( fread( table, size, 1, fp ) != 1 ) { error( "couldn't read from inf file '%s'", filename ); return 1; }

   DEBUG(( stderr, "fill_methods: file size %d (table = %p)\n", size, table ));

   int check_dirs = nsearch_dirs;

   if( table[1] != '-' ) // lets look for base files
   {
      DEBUG(( stderr, "fill_methods: looking up base file(s)\n" ));

      char *look_dir;
      char cwd[1000];
      getcwd( cwd, 999 );

      strtok( table, "]" );
      char fn[ 1000 ];
      strcat( strcpy( fn, table + 1 ), ".inf" );

      look_dir = "./";

      while( access(fn,0) && check_dirs > 0 )
      {
         look_dir = search_dirs[ --check_dirs ];
         DEBUG(( stderr, "fill_methods: looking up '%s' in '%s'\n", fn, look_dir ));
         chdir( look_dir );
      }

      if( access(fn,0) ) { error( "couldn't open base interface file '%s'", fn ); return 1; }

      chdir( cwd );

      char base[1000];
      strcat( strcpy( base, look_dir ), fn );
      DEBUG(( stderr, "fill_methods: base interface file '%s'\n", base ));

      fill_methods( base );
   }

   char *c = table;
   while( *(++c) ) ;
   *c = ']';
   char *mname;

   char *p = table;

   if( linker_options & OPT_VERBOSE )
      fprintf( stdout, "\n** Name ******************************************** Offs ***** Param **\n" );

   while( 1 )
   {
      Elf32_Word offs;

      unsigned short psize = count( strtok( p, ":" ), ';' ); // calc parameters size
      psize = psize*4;
      p = 0; // reset p to NULL so that next time strtok above will return next token

      strtok( 0, " " );

      mname = strtok( 0, "\n" );

      DEBUG(( stderr, "fill_methods: found %s (table = %p)\n", mname, table ));

      if( mname == 0 )
         break;

      char namecp[ 100 ];
      strcpy( namecp, "method_" );
      strncat( namecp, mname, 90 );

      if( linker_options & OPT_VERBOSE )
         fprintf(stdout, "* %-40s", mname);

      if( get_global_value(namecp, &offs) ) { error( "cannot find entry for function %s()", mname ); return 1; }

      if( strcmp( mname, "ctor" ) == 0 )
      {
         if( linker_options & OPT_VERBOSE )
            fprintf(stdout, "[ CTOR ]");

         mtbl.ctor_entry = (void *)offs;
      }
      else if( strcmp( mname, "dtor" ) == 0 )
      {
         if( linker_options & OPT_VERBOSE )
            fprintf(stdout, "[ DTOR ]");

         mtbl.dtor_entry = (void *)offs;
      }
      else
      {
         if( linker_options & OPT_VERBOSE )
            fprintf(stdout, "        ");

         mtbl[ mtbl.mcount ].start = (void *)offs;
         mtbl[ mtbl.mcount ].psize = psize;
         mtbl.mcount++;
      }

      if( linker_options & OPT_VERBOSE )
         fprintf(stdout, " 0x%08x 0x%08x\n", offs, psize);
   }

   delete table;

   DEBUG(( stderr, "fill_methods: finished\n" ));
   return 0;
}

// generate method table
int generate_mt()
{
   DEBUG(( stderr, "generate_mt():\n" ));

   if( !mt_file ) { error( "mt file name not specified for a comp" ); return 1; }

   mtbl.mcount = 0;
   if( fill_methods( mt_file ) ) return 1;

   DEBUG(( stderr, "generate_mt() finished\n" ));
   return 0;
}


//------------------------------------------------------------------------
//
// ELF objects manipulation functions
//
//------------------------------------------------------------------------

// Load progbits, symtab, strtab and rels from an ELF object file
int object::open( char *fname )
{
   Elf32_Ehdr ehdr;
   Elf32_Shdr *shdr;
   char       *shstrtab = 0;

   sections = NULL;
   locals   = NULL;
   next     = NULL;
   filename = strdup( fname );
   fp       = fopen( filename, "rb" );
   if( !fp ) { error( "could not open object file '%s'", filename ); return 1; }

   if( fread( &ehdr, sizeof ehdr, 1, fp ) != 1 ) { error( "cannot read elf header from '%s'", filename ); return 1; }
   if( ehdr.e_magic   != ELFMAGIC    ||
       ehdr.e_class   != ELFCLASS32  ||
       ehdr.e_data    != ELFDATA2LSB ||
       ehdr.e_machine != EM_386      ||
       ehdr.e_version != EV_CURRENT)             { error( "bad elf file header in '%s'", filename ); return 1; }
   if( ehdr.e_type    != ET_REL )                { warning( "file '%s' is not relocatable, skipped!", filename ); return 1; }
   if( ehdr.e_shoff   == 0 )                     { warning( "section header is not present in '%s', skipped!", filename ); return 1; }
   if( ehdr.e_shstrndx == SHN_UNDEF )            { warning( "section string table is absent in '%s', possibly broken file", filename ); }

   // read section headers
   char *shdrs = new char [ehdr.e_shentsize * ehdr.e_shnum];

   fseek( fp, ehdr.e_shoff, SEEK_SET );
   if( fread( shdrs, ehdr.e_shentsize, ehdr.e_shnum, fp ) != ehdr.e_shnum ) { error( "cannot read section header from '%s'", filename ); return 1; }

   // now find and load section string table
   shdr = (Elf32_Shdr *)(shdrs + ehdr.e_shstrndx * ehdr.e_shentsize);

   if( shdr->sh_size != 0 )
   {
      shstrtab = new char [shdr->sh_size];

      fseek( fp, shdr->sh_offset, SEEK_SET );
      if( fread( shstrtab, shdr->sh_size, 1, fp ) != 1 ) { error( "could not read section string table from '%s'", filename ); return 1; }
   }

   // now read sections and append to object's sections list
   for( int i = 0; i < ehdr.e_shnum; i++ )
   {
      shdr = (Elf32_Shdr *)(shdrs + i * ehdr.e_shentsize);

      char *data = NULL;

      if( ((shdr->sh_type == SHT_PROGBITS && (shdr->sh_flags & SHF_ALLOC)) || // progbits
           (shdr->sh_type == SHT_SYMTAB)                                   || // symtab
           (shdr->sh_type == SHT_STRTAB && (i != ehdr.e_shstrndx))         || // strtab
           (shdr->sh_type == SHT_REL))                                        // rel
        )
      {
         DEBUG(( stderr, "reading section %d of type %d [size %d at position %d]\n", i, shdr->sh_type, shdr->sh_size, shdr->sh_offset ));

         // empty .text sections must be loaded to avoid warnings
         // we load other important sections as well, even if they are empty
         if( shdr->sh_size == 0 )
         {
            add_section( shstrtab + shdr->sh_name, shdr, i, NULL );
         }
         else
         {
            data = new char [shdr->sh_size];
            fseek( fp, shdr->sh_offset, SEEK_SET );
            if( fread( data, shdr->sh_size, 1, fp ) != 1 ) { error( "could not read section from '%s'", filename ); return 1; }//memleak!

            add_section( shstrtab + shdr->sh_name, shdr, i, data );
         }
      }
      else if( (shdr->sh_type == SHT_NOBITS) && (shdr->sh_flags & SHF_ALLOC) ) // .bss
      {
         add_section( shstrtab + shdr->sh_name, shdr, i, data );//data==NULL, use NULL for clarity
      }
      else DEBUG(( stderr, "skipping section %s of type %d [size %08x, flags %08x]\n", shstrtab + shdr->sh_name, shdr->sh_type, shdr->sh_size, shdr->sh_flags ));
   }

   delete shstrtab;
   delete shdrs;

   return 0;
}

int object::add_section( char *name, Elf32_Shdr *header, Elf32_Word secndx, char *data )
{
   DEBUG(( stderr, "add_section(%s, %p, %p):\n", name, header, data ));

   if( strstr( name, ".stab" ) ) return 0; // we don't need that crap

   section *s  = new section;
   s->next     = NULL;
   s->parent   = this;
   s->offset   = 0;
   s->contents = data;
   s->name     = strdup(name);
   s->header   = new Elf32_Shdr;
   memcpy( s->header, header, sizeof(Elf32_Shdr) );
   s->header->sh_name = secndx;

	// next should be linkedlist->append()
   if( !sections ) sections = s;
   else
   {
      section *p = sections;
      while( p->next ) p = p->next;
      p->next = s;
   }

   DEBUG(( stderr, "add_section: finished\n" ));
   return 0;
}

section *object::get_section_by_type( Elf32_Word type )
{
   section *s = sections;
   while( s )
   {
      if( s->header->sh_type == type ) break;
      s = s->next;
   }
   return s;
}

int object::add_local( char *name, Elf32_Sym *sym )
{
   DEBUG(( stderr, "add_local(%s, %p):\n", name, sym ));

   local *l = locals;

   while( l )
   {
      if( strcmp( name, l->name ) == 0 )
      {
         if( sym->st_value == l->sym->st_value )
         {
			   // This is normal, no need to worry.
            //warning( "local symbol '%s' defined twice in '%s'", name, filename );
            return 0;
         }
         else
         {
            error( "local symbol '%s' defined twice in '%s'", name, filename );
            return 1;
         }
      }
      l = l->next;
   }

   l = new local;
   // next should be linkedlist->prepend()
//locals->prepend(l);
   l->next = locals;
   locals = l;

   l->name = name;
   l->sym = sym;

   return 0;
}


int add_global( char *name, Elf32_Sym *sym, object *parent )
{
   DEBUG(( stderr, "add_global(%s, %p, %p): ", name, sym, parent ));

   global *g = root_global;

   while( g )
   {
      if( strcmp( name, g->name ) == 0 ) break;
      g = g->next;
   }

   if( !g )
   {
      // such global does not yet exists, add to list
      g = new global;
      g->next = root_global;
      root_global = g;
      g->name = name;
      g->sym = sym;
      g->parent = parent;
      DEBUG(( stderr, "created global definition of '%s' at %p [weak: %d, value: %d]\n", name, g, ELF32_ST_BIND(sym->st_info) == STB_WEAK, sym->st_value ));
   }
   else
   {
      if( ELF32_ST_BIND(g->sym->st_info) == ELF32_ST_BIND(sym->st_info) ) // equal binding
      {
         if( ELF32_ST_BIND(sym->st_info) == STB_WEAK )
         {
            DEBUG(( stderr, "duplicate weak symbol, discarding\n" ));
            return 0;
         }

         if( g->sym->st_value == sym->st_value )
         {
            warning( "global symbol '%s' is redefined in '%s'", name, parent->filename );
            return 0;
         }

         error( "global symbol '%s' defined in '%s' conflicts with global symbol already defined in '%s'", name, parent->filename, g->parent->filename );
         return 1;
      }

      if( ELF32_ST_BIND(sym->st_info) == STB_GLOBAL )
      {
         g->sym = sym;
         g->parent = parent;
         DEBUG(( stderr, "replacing old weak definition of '%s' at %p\n", name, g ));
      }
   }

   return 0;
}

int not_an_option_handler( char *param )
{
   return add_file( param );
}

// Open file, read its ELF contents and add it to the list of open objects
int add_file( char *filename )
{
   DEBUG(( stderr, "add_file: reading object '%s'\n", filename ));

   object *file = new object;
   if( file->open( filename ) )
   {
      // don't error() here since file->open() will do it for us
      DEBUG(( stderr, "add_file: failed to add '%s'\n", filename ));
      delete file;
   }
   else
   {
		root_obj.append(file);
        DEBUG(( stderr, "add_file: added file '%s'\n", filename ));
   }

   return errors;
}

// Adjust all symbols values with regard to their current offsets
int adjust_symbols_values()
{
   DEBUG(( stderr, "adjust_symbols_values()\n" ));

   object *o = root_obj; // process all objects
// TODO: need iterator for linked_list

   while( o )
   {
      DEBUG(( stderr, "adjust_symbols_values: processing object '%s'\n", o->filename ));

      section *strtab = o->get_section_by_type( SHT_STRTAB ); // get object's strtab
      section *s      = o->get_section_by_type( SHT_SYMTAB ); // get object's symtab

      if( !s ) warning( "object '%s' got no symbol table!!! (skipped)", o->filename );
      else
      {
         int nsyms = s->header->sh_size / s->header->sh_entsize;

         while( nsyms-- )
         {
            Elf32_Sym *sym = (Elf32_Sym *)(s->contents + nsyms * s->header->sh_entsize);
            DEBUG(( stderr, "adjusting symbol %d (%s) in object '%s' [shndx %d, old value %x, ", nsyms, sym->st_name==0?"<null>":strtab==NULL?"<null>":strtab->contents+sym->st_name, o->filename, sym->st_shndx, sym->st_value ));

            if( sym->st_shndx > SHN_UNDEF && sym->st_shndx < SHN_LORESERVE )
            {
               // find relevant section
               section *sec = o->sections;
               while( sec )
               {
                  if( sec->header->sh_name == sym->st_shndx ) break;
                  sec = sec->next;
               }
               if( !sec ) warning("symbol %s relevant section %d is missing in object '%s', not adjusted!!!", sym->st_name==0?"<null>":strtab==NULL?"<null>":strtab->contents+sym->st_name, sym->st_shndx, o->filename);
               else
                  sym->st_value += sec->offset; // add its offset
            }
            DEBUG(( stderr, "new value %x]\n", sym->st_value ));
         }
      }

      o = o->next;
   }

   DEBUG(( stderr, "adjust_symbols_values finished\n" ));
   return errors;
}


// Register global and local symbols for resolving
int register_symbols()
{
   DEBUG(( stderr, "register_symbols started\n" ));

   object *o = root_obj; // process all objects

   while( o )
   {
      // get object's symtab
      section *s = o->get_section_by_type( SHT_SYMTAB );

      if( !s ) warning( "object '%s' has no symbol table!!! (skipped)", o->filename );
      else
      {
         // get object's strtab
         section *strtab = o->sections;
         while( strtab )
         {
            if( (strtab->header->sh_type == SHT_STRTAB) && (strtab->header->sh_name == s->header->sh_link) ) break;
            strtab = strtab->next;
         }

         if( !strtab ) warning( "object '%s' has no symbol string table!!! (skipped)", o->filename );
         else
         {
            int nsyms = s->header->sh_size / s->header->sh_entsize;

            while( nsyms-- )
            {
               Elf32_Sym *sym = (Elf32_Sym *)(s->contents + nsyms * s->header->sh_entsize);

               if( sym->st_shndx == 0 ) continue; // undefined symbol, will resolve later
               if( sym->st_name  == 0 ) continue; // unnamed symbol, can do nothing about it

               // find symbol name
               char *name = strtab->contents + sym->st_name;

               switch( ELF32_ST_BIND(sym->st_info) )
               {
                  case STB_LOCAL:
                     o->add_local( name, sym );
                     break;

                  case STB_GLOBAL:
                  case STB_WEAK:
                     add_global( name, sym, o );
                     break;

                  default:
                     warning( "unknown symbol binding %d in object '%s'", ELF32_ST_BIND(sym->st_info), o->filename );
               }
            }
         }
      }

      o = o->next;
   }

   DEBUG(( stderr, "register_symbols finished\n" ));
   return errors;
}

// Find a global by name and fix its value
int get_global_value( char *name, Elf32_Word *value )
{
   global *g = root_global;

//linked_list.find()? or find() algo
   while( g )
   {
      if( strcmp( name, g->name ) == 0 )
      {
         *value = g->sym->st_value;
         return 0;
      }
      g = g->next;
   }

   error( "required symbol '%s' could not be found", name );
   return 1;
}

// Resolve references between symbols
int resolve_references()
{
   DEBUG(( stderr, "resolve_references():\n" ));

   object *o = root_obj; // process all objects

   while( o )
   {
      // get object's symtab
      section *s = o->get_section_by_type( SHT_SYMTAB );

      if( !s ) warning( "object '%s' got no symbol table!!! (skipped)", o->filename );
      else
      {
         section *strtab = o->sections;
         while( strtab )
         {
            if( (strtab->header->sh_type == SHT_STRTAB) && (strtab->header->sh_name == s->header->sh_link) ) break;
            strtab = strtab->next;
         }

         if( !strtab ) warning( "object '%s' got no symbol string table!!! (skipped)", o->filename );
         else
         {
            int nsyms = s->header->sh_size / s->header->sh_entsize;
            while( nsyms-- )
            {
               Elf32_Sym *sym = (Elf32_Sym *)(s->contents + nsyms * s->header->sh_entsize);

               if( sym->st_shndx != 0 ) continue; // defined symbol, no need to resolve
               if( sym->st_name  == 0 ) continue; // unnamed symbol, can do nothing about it

               char *name = strtab->contents + sym->st_name;

               DEBUG(( stderr, "undefined symbol '%s', ", name ));
               if( !get_global_value( name, &sym->st_value ) ) DEBUG(( stderr, "value fixed to %x\n", sym->st_value ));
            }
         }
      }

      o = o->next;
   }

   DEBUG(( stderr, "resolve_references finished\n" ));
   return errors;
}


// Apply relocation values from resolved symbols to required locations
int apply_relocations()
{
   DEBUG(( stderr, "apply_relocations():\n" ));

   object *o = root_obj;

   while( o )
   {
      section *s = o->sections;
      while( s )
      {
         if( s->header->sh_type == SHT_REL )
         {
            // find section that should be relocated
			//linked_list->find()
            section *target = o->sections;
            while( target )
            {
               if( target->header->sh_name == s->header->sh_info ) break;
               target = target->next;
            }

            if( !target ) warning( "relocation target section does not exist! (no relocation applied)" );
            else
            {
               // find symbol table for this relocation
				//linked_list->find()
               section *symtab = o->sections;
               while( symtab )
               {
                  if( symtab->header->sh_name == s->header->sh_link ) break;
                  symtab = symtab->next;
               }

               if( !symtab ) warning( "relocation symbol table section does not exist! (no relocation applied)" );
               else
               {
                  // actually apply relocations
                  int nrels = s->header->sh_size / s->header->sh_entsize;

                  while( nrels-- )
                  {
                     Elf32_Rel *rel = (Elf32_Rel *)(s->contents + nrels * s->header->sh_entsize);
                     Elf32_Sym *sym = (Elf32_Sym *)(symtab->contents + ELF32_R_SYM(rel->r_info) * symtab->header->sh_entsize);

                     switch( ELF32_R_TYPE(rel->r_info) )
                     {
                        case R_386_32:
                           DEBUG(( stderr, "apply_relocations: applying R_386_32 at %d in %s [old value %ld, ", rel->r_offset, target->name, *(dword *)(target->contents + rel->r_offset) ));
                           *(dword *)(target->contents + rel->r_offset) += sym->st_value;
                           DEBUG(( stderr, "new value %ld, delta %d]\n", *(dword *)(target->contents + rel->r_offset), sym->st_value ));
                           break;

                        case R_386_PC32:
                           DEBUG(( stderr, "apply_relocations: applying R_386_PC32 at %d in %s [old value %ld, ", rel->r_offset, target->name, *(dword *)(target->contents + rel->r_offset) ));
                           *(dword *)(target->contents + rel->r_offset) += sym->st_value - (rel->r_offset + target->offset);
                           DEBUG(( stderr, "new value %ld, delta %d]\n", *(dword *)(target->contents + rel->r_offset), sym->st_value - rel->r_offset ));
                           break;

                        default:
                           error( "relocation type %d in object '%s' is unsupported, relocation not applied", ELF32_R_TYPE(rel->r_info), o->filename );
                     } // switch
                  } // while
               } // else
            } // else
         } // if

         s = s->next;
      } // while
      o = o->next;
   } // while

   DEBUG(( stderr, "apply_relocations() finished\n" ));
   return 0;
}


// Produce symfile
int produce_xrefs( char *filename )
{
   char str[100];
   char fname[1000];
   char *objectname;
   strncpy( fname, filename, 995 ); // 999 - 4 chars for ".sym"
   strcat( fname, ".sym" );

   FILE *fp = fopen( fname, "wt" );
   if( !fp ) { error( "cannot open xref symbol file '%s'", fname ); return 1; }

   // output globals
   for( int i = 0; i < 16; i++ ) fprintf( fp, "------" );
   fprintf( fp, "\nGlobals list\n"
                "%-15s %-45s %-8s   %-12s %-10s\n",
                "Object", "Name", "Value", "Binding", "Section" );
   for( int i = 0; i < 16; i++ ) fprintf( fp, "------" );
   fprintf( fp, "\n" );

   global *g = root_global;
   while( g )
   {
		if (!((linker_options & OPT_XREF_NO_ABS) && (g->sym->st_shndx == SHN_ABS)))
		{
	      sprintf( str, "%x", g->sym->st_shndx );
   	   objectname = g->parent?((objectname = strrchr(g->parent->filename,'/'))?objectname+1:g->parent->filename):(char *)"LINKER";
	      fprintf( fp, "%-15s %-45s %08x   %-12s %-10s\n", objectname, g->name, g->sym->st_value,
   	         ELF32_ST_BIND(g->sym->st_info) == STB_LOCAL ? "STB_LOCAL" :
      	      ELF32_ST_BIND(g->sym->st_info) == STB_GLOBAL ? "STB_GLOBAL" :
         	   ELF32_ST_BIND(g->sym->st_info) == STB_WEAK ? "STB_WEAK" : "unknown",
            	g->sym->st_shndx == SHN_UNDEF ? "SHN_UNDEF" :
	            g->sym->st_shndx == SHN_ABS   ? "SHN_ABS" :
   	         str
      	);
		}
      g = g->next;
   }

   object *o = root_obj;
   while( o )
   {
      local *l = o->locals;
      if( l )
      {
         for( int i = 0; i < 16; i++ ) fprintf( fp, "------" );
         fprintf( fp, "\nLocals for object %s\n", o->filename );
         for( int i = 0; i < 16; i++ ) fprintf( fp, "------" );
         fprintf( fp, "\n" );
      }

      while( l )
      {
			if (!((linker_options & OPT_XREF_NO_ABS) && (l->sym->st_shndx == SHN_ABS)))
			{
	         sprintf( str, "%x", l->sym->st_shndx );
   	   	objectname = (objectname = strrchr(o->filename,'/'))?objectname+1:o->filename;
      	   fprintf( fp, "%-15s %-45s %08x   %-12s %-10s\n", objectname, l->name, l->sym->st_value,
         	      ELF32_ST_BIND(l->sym->st_info) == STB_LOCAL ? "STB_LOCAL" :
            	   ELF32_ST_BIND(l->sym->st_info) == STB_GLOBAL ? "STB_GLOBAL" :
               	ELF32_ST_BIND(l->sym->st_info) == STB_WEAK ? "STB_WEAK" : "unknown",
	               l->sym->st_shndx == SHN_UNDEF ? "SHN_UNDEF" :
   	            l->sym->st_shndx == SHN_ABS   ? "SHN_ABS" :
      	         str
         	);
			}
         l = l->next;
      }
      o = o->next;
   }

   fclose( fp );
   return 0;
}

// Produce bochs symfile
int produce_bochs_sym( char *filename )
{
   char fname[1000];
   strncpy( fname, filename, 995 ); // 999 - 4 chars for ".bsm"
   strcat( fname, ".bsm" );

   FILE *fp = fopen( fname, "wt" );
   if( !fp ) { error( "cannot open bochs symbol file '%s'", fname ); return 1; }

   // output globals
   global *g = root_global;
   while( g )
   {
		if (!((linker_options & OPT_XREF_NO_ABS) && (g->sym->st_shndx == SHN_ABS)))
		{
         fprintf( fp, "%x %s\n", g->sym->st_value, g->name );
		}
      g = g->next;
   }

   fclose( fp );
   return 0;
}


//---------------------------------------------------------------------------------------
// Option parsing handlers
//---------------------------------------------------------------------------------------

int opt_linker_options( char *cmd, char *param, int extra )
{
   DEBUG(( stderr, "opt_linker_options: got option '%s' with param '%s' and extra '%d'\n", cmd, param, extra ));

   linker_options |= extra;

   return 0;
}

int opt_filename_option( char *cmd, char *param, int extra )
{
   DEBUG(( stderr, "opt_filename_option: got option '%s' with param '%s' and extra '%d'\n", cmd, param, extra ));

   if( output_filename != default_output )
   {
      warning( "multiple output file names specified, using latest one" );
      delete output_filename;
   }
   output_filename = new char [strlen(param)+1];
   strcpy( output_filename, param );

   return 0;
}

int opt_mt_option( char *cmd, char *param, int extra )
{
   DEBUG(( stderr, "opt_mt_option: got option '%s' with param '%s' and extra '%d'\n", cmd, param, extra ));

   if( mt_file != NULL )
   {
      warning( "multiple mt file names specified, using latest one" );
      delete mt_file;
   }

   mt_file = new char [strlen(param)+1];
   strcpy( mt_file, param );

   return 0;
}

int opt_mt_dir_option( char *cmd, char *param, int extra )
{
   DEBUG(( stderr, "opt_mt_dir_option: got option '%s' with param '%s' and extra '%d'\n", cmd, param, extra ));

   if( nsearch_dirs >= extra )
   {
      warning( "too many search directories specified, ignoring '%s'", param );
      return 1;
   }

   if( param[strlen(param)] != '\\' && param[strlen(param)] != '/' ) strcat( param, "/" );

   search_dirs[ nsearch_dirs ] = new char [strlen(param)+1];
   strcpy( search_dirs[ nsearch_dirs ], param );
   nsearch_dirs++;

   return 0;
}

int opt_text_align_option( char *cmd, char *param, int extra )
{
   DEBUG(( stderr, "opt_text_align_option: got option '%s' with param '%s' and extra '%d'\n", cmd, param, extra ));

   text_alignment = atoi(param);

   if( !text_alignment )
   {
   	warning( "invalid text alignment specified, using default!" );
   	text_alignment = DEF_TEXT_ALIGN;
   }

   if( (text_alignment - 1) & text_alignment )
   {
   	warning( "text alignment is not power of two, using default!" );
   	text_alignment = DEF_TEXT_ALIGN;
   }

   return 0;
}

int opt_data_align_option( char *cmd, char *param, int extra )
{
   DEBUG(( stderr, "opt_data_align_option: got option '%s' with param '%s' and extra '%d'\n", cmd, param, extra ));

   data_alignment = atoi(param);

   if( !data_alignment )
   {
   	warning( "invalid data alignment specified, using default!" );
   	data_alignment = DEF_DATA_ALIGN;
   }

   if( (data_alignment - 1) & data_alignment )
   {
   	warning( "data alignment is not power of two, using default!" );
   	data_alignment = DEF_DATA_ALIGN;
   }

   return 0;
}

//------------------------------------------------------------------------
// Support Functions
//------------------------------------------------------------------------

void error( const char *mess, ... )
{
   va_list argptr;

   va_start( argptr, mess );
   fprintf( stderr, "ERROR: " );
   vfprintf( stderr, mess, argptr );
   fprintf( stderr, "\n" );
   va_end( argptr );

   errors++;
}

void warning( const char *mess, ... )
{
   va_list argptr;

   va_start( argptr, mess );
   fprintf( stderr, "WARNING: " );
   vfprintf( stderr, mess, argptr );
   fprintf( stderr, "\n" );
   va_end( argptr );

   warnings++;
};


