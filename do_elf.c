#include "debug.h"


struct comp_unit
{
    unsigned long cu_offset;
    DWARF2_Internal_CompUnit cu;
    abbrev_entry *first_abbrev;
    abbrev_entry *last_abbrev;
    struct comp_unit *next;
};

void list_cus(struct comp_unit *head)
{
    struct comp_unit *tmp = head;
    for(; tmp ; tmp = tmp->next) {
        debug("%3lu\n", tmp->cu_offset);
    }
}
static int
dump_dwarf_info (struct comp_unit *head)
{
    abbrev_entry *entry;

    struct comp_unit *tmp = head;
    for(; tmp ; tmp = tmp->next) {
        if (tmp->first_abbrev == NULL)
            continue;

        printf (_("  Number TAG\n"));
        for (entry = tmp->first_abbrev; entry; entry = entry->next)
        {
            abbrev_attr *attr;

            printf (_("   %ld      %s    [%s]\n"),
                    entry->entry,
                    get_TAG_name (entry->tag),
                    entry->children ? _("has children") : _("no children"));

            for (attr = entry->first_attr; attr; attr = attr->next)
            {
                printf (_("    %-18s %s\n"),
                        get_AT_name (attr->attribute),
                        get_FORM_name (attr->form));
            }
        }
    }
    printf ("\n");

    return 1;
}

struct comp_unit *comp_unit_head = NULL;

static struct comp_unit *add_node(DWARF2_Internal_CompUnit *node, unsigned long cu_offset)
{
    struct comp_unit *tmp = (struct comp_unit*) malloc(sizeof(struct comp_unit));
    exit_on_error(tmp == NULL);
    tmp->cu_offset = cu_offset;
    memcpy(&tmp->cu, node, sizeof(DWARF2_External_CompUnit));
    tmp->first_abbrev = NULL;
    tmp->last_abbrev = NULL;
    tmp->next = NULL;
    return tmp;
}

struct comp_unit *comp_unit_add(struct comp_unit *head, DWARF2_Internal_CompUnit *node, unsigned long cu_offset)
{
    struct comp_unit *newnode = add_node(node, cu_offset);
    if (NULL == head) {
        return newnode;
    }else {
        newnode->next = head;
        head = newnode;
        return head;
    }
}

const char *debug_str_contents;
size_t debug_str_size;
elf64_ehdr *elf_header = NULL;
elf64_shdr *section_headers = NULL;
elf64_shdr *string_table_header = NULL;
char *string_table = NULL;
unsigned long string_table_length;
static const char *debug_loc_contents;
static size_t debug_loc_size;
static void
free_debug_loc (void)
{
  if (debug_loc_contents == NULL)
    return;

  free ((char *) debug_loc_contents);
  debug_loc_contents = NULL;
  debug_loc_size = 0;
}

static abbrev_entry *first_abbrev = NULL;
static abbrev_entry *last_abbrev = NULL;

    static void
free_abbrevs (void)
{
    abbrev_entry *abbrev;

    for (abbrev = first_abbrev; abbrev;)
    {
        abbrev_entry *next = abbrev->next;
        abbrev_attr *attr;

        for (attr = abbrev->first_attr; attr;)
        {
            abbrev_attr *next = attr->next;

            free (attr);
            attr = next;
        }

        free (abbrev);
        abbrev = next;
    }

    last_abbrev = first_abbrev = NULL;
}

    static void
add_abbrev (unsigned long number, unsigned long tag, int children, struct comp_unit *compunit)
{
    abbrev_entry *entry;

    entry = malloc (sizeof (*entry));

    if (entry == NULL)
        /* ugg */
        return;

    entry->entry      = number;
    entry->tag        = tag;
    entry->children   = children;
    entry->first_attr = NULL;
    entry->last_attr  = NULL;
    entry->next       = NULL;

    if (compunit->first_abbrev == NULL)
        compunit->first_abbrev = entry;
    else
        compunit->last_abbrev->next = entry;

    compunit->last_abbrev = entry;
}

    static void
add_abbrev_attr (unsigned long attribute, unsigned long form, struct comp_unit *compunit)
{
    abbrev_attr *attr;

    attr = malloc (sizeof (*attr));

    if (attr == NULL)
        /* ugg */
        return;

    attr->attribute = attribute;
    attr->form      = form;
    attr->next      = NULL;

    if (compunit->last_abbrev->first_attr == NULL)
        compunit->last_abbrev->first_attr = attr;
    else
        compunit->last_abbrev->last_attr->next = attr;

    compunit->last_abbrev->last_attr = attr;
}



    static unsigned char *
process_abbrev_section (unsigned char *start, unsigned char *end, struct comp_unit *compunit)
{
    if (compunit->first_abbrev != NULL)
        return NULL;

    while (start < end)
    {
        int bytes_read;
        unsigned long entry;
        unsigned long tag;
        unsigned long attribute;
        int children;

        entry = read_leb128 (start, & bytes_read, 0);
        start += bytes_read;

        /* A single zero is supposed to end the section according
           to the standard.  If there's more, then signal that to
           the caller.  */
        if (entry == 0)
            return start == end ? NULL : start;

        tag = read_leb128 (start, & bytes_read, 0);
        start += bytes_read;

        children = *start++;

        add_abbrev (entry, tag, children, compunit);

        do
        {
            unsigned long form;

            attribute = read_leb128 (start, & bytes_read, 0);
            start += bytes_read;

            form = read_leb128 (start, & bytes_read, 0);
            start += bytes_read;

            if (attribute != 0)
                add_abbrev_attr (attribute, form, compunit);
        }
        while (attribute != 0);
    }

    return NULL;
}

#if 0
    static int
display_debug_abbrev (elf64_shdr *section,
        unsigned char *start,
        int fd)
{
    abbrev_entry *entry;
    unsigned char *end = start + section->sh_size;

    do
    {
        start = process_abbrev_section (start, end);

        if (first_abbrev == NULL)
            continue;

        printf (_("  Number TAG\n"));

        for (entry = first_abbrev; entry; entry = entry->next)
        {
            abbrev_attr *attr;

            printf (_("   %ld      %s    [%s]\n"),
                    entry->entry,
                    get_TAG_name (entry->tag),
                    entry->children ? _("has children") : _("no children"));

            for (attr = entry->first_attr; attr; attr = attr->next)
            {
                printf (_("    %-18s %s\n"),
                        get_AT_name (attr->attribute),
                        get_FORM_name (attr->form));
            }
        }

        free_abbrevs ();
    }
    while (start);

    printf ("\n");

    return 1;
}
#endif
elf64_shdr *
find_section (const char * name)
{
  elf64_shdr *sec;
  unsigned int i;

  for (i = elf_header->e_shnum, sec = section_headers + i - 1;
       i; --i, --sec)
    if (strcmp (SECTION_NAME (sec), name) == 0)
      break;

  if (i && sec && sec->sh_size != 0)
    return sec;

  return NULL;
}


static void
load_debug_str (int fd)
{
  elf64_shdr *sec;

  /* If it is already loaded, do nothing.  */
  if (debug_str_contents != NULL)
    return;

  /* Locate the .debug_str section.  */
  sec = find_section (".debug_str");
  if (sec == NULL)
    return;

  debug_str_size = sec->sh_size;

  debug_str_contents = get_data (NULL, fd, sec->sh_offset, sec->sh_size,
				 _("debug_str section data"));
}

static void
free_debug_str (void)
{
  if (debug_str_contents == NULL)
    return;

  free ((char *) debug_str_contents);
  debug_str_contents = NULL;
  debug_str_size = 0;
}

static void
load_debug_loc (int fd)
{
  elf64_shdr *sec;

  /* If it is already loaded, do nothing.  */
  if (debug_loc_contents != NULL)
    return;

  /* Locate the .debug_loc section.  */
  sec = find_section (".debug_loc");
  if (sec == NULL)
    return;

  debug_loc_size = sec->sh_size;

  debug_loc_contents = get_data (NULL, fd, sec->sh_offset, sec->sh_size,
				 _("debug_loc section data"));
}
static int
display_debug_info (elf64_shdr *section,
		    unsigned char *start,
		    int fd)
{
  unsigned char *end = start + section->sh_size;
  unsigned char *section_begin = start;

  printf (_("The section %s contains:\n\n"), SECTION_NAME (section));

  load_debug_str (fd);
  load_debug_loc (fd);

  while (start < end)
    {
      DWARF2_Internal_CompUnit compunit;
      //Elf_Internal_Shdr *relsec;
      unsigned char *hdrptr;
      unsigned char *cu_abbrev_offset_ptr;
      unsigned char *tags;
      int level;
      unsigned long cu_offset;
      int offset_size;
      int initial_length_size;

      hdrptr = start;

      compunit.cu_length = byte_get (hdrptr, 4);
      hdrptr += 4;

      if (compunit.cu_length == 0xffffffff)
	{
	  compunit.cu_length = byte_get (hdrptr, 8);
	  hdrptr += 8;
	  offset_size = 8;
	  initial_length_size = 12;
    }

      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      compunit.cu_version = byte_get (hdrptr, 2);
      hdrptr += 2;
#if 0
      /* Apply addends of RELA relocations.  */
      for (relsec = section_headers;
	   relsec < section_headers + elf_header.e_shnum;
	   ++relsec)
	{
	  unsigned long nrelas;
	  Elf_Internal_Rela *rela, *rp;
	  Elf_Internal_Shdr *symsec;
	  Elf_Internal_Sym *symtab;
	  Elf_Internal_Sym *sym;

	  if (relsec->sh_type != SHT_RELA
	      || SECTION_HEADER (relsec->sh_info) != section
	      || relsec->sh_size == 0)
	    continue;

	  if (!slurp_rela_relocs (file, relsec->sh_offset, relsec->sh_size,
				  & rela, & nrelas))
	    return 0;

	  symsec = SECTION_HEADER (relsec->sh_link);
	  symtab = GET_ELF_SYMBOLS (file, symsec);

	  for (rp = rela; rp < rela + nrelas; ++rp)
	    {
	      unsigned char *loc;

	      if (rp->r_offset >= (bfd_vma) (hdrptr - section_begin)
		  && section->sh_size > (bfd_vma) offset_size
		  && rp->r_offset <= section->sh_size - offset_size)
		loc = section_begin + rp->r_offset;
	      else
		continue;

	      if (is_32bit_elf)
		{
		  sym = symtab + ELF32_R_SYM (rp->r_info);

		  if (ELF32_R_SYM (rp->r_info) != 0
		      && ELF32_ST_TYPE (sym->st_info) != STT_SECTION)
		    {
		      warn (_("Skipping unexpected symbol type %u\n"),
			    ELF32_ST_TYPE (sym->st_info));
		      continue;
		    }
		}
	      else
		{
		  sym = symtab + ELF64_R_SYM (rp->r_info);

		  if (ELF64_R_SYM (rp->r_info) != 0
		      && ELF64_ST_TYPE (sym->st_info) != STT_SECTION)
		    {
		      warn (_("Skipping unexpected symbol type %u\n"),
			    ELF64_ST_TYPE (sym->st_info));
		      continue;
		    }
		}

	      byte_put (loc, rp->r_addend, offset_size);
	    }

	  free (rela);
	  break;
	}
#endif

      cu_abbrev_offset_ptr = hdrptr;
      compunit.cu_abbrev_offset = byte_get (hdrptr, offset_size);
      hdrptr += offset_size;

      compunit.cu_pointer_size = byte_get (hdrptr, 1);
      hdrptr += 1;

      tags = hdrptr;
      cu_offset = start - section_begin;
      start += compunit.cu_length + initial_length_size;

      printf (_("  Compilation Unit @ %lx:\n"), cu_offset);
      printf (_("   Length:        %ld\n"), compunit.cu_length);
      printf (_("   Version:       %d\n"), compunit.cu_version);
      printf (_("   Abbrev Offset: %ld\n"), compunit.cu_abbrev_offset);
      printf (_("   Pointer Size:  %d\n"), compunit.cu_pointer_size);
      comp_unit_head = comp_unit_add(comp_unit_head,  &compunit, cu_offset);

      if (compunit.cu_version != 2 && compunit.cu_version != 3)
	{
	  warn (_("Only version 2 and 3 DWARF debug information is currently supported.\n"));
	  continue;
	}

      free_abbrevs ();

      /* Read in the abbrevs used by this compilation unit.  */
      {
	elf64_shdr *sec;
	unsigned char *begin;

	/* Locate the .debug_abbrev section and process it.  */
	sec = find_section (".debug_abbrev");
	if (sec == NULL)
	  {
	    warn (_("Unable to locate .debug_abbrev section!\n"));
	    return 0;
	  }

	begin = get_data (NULL, fd, sec->sh_offset, sec->sh_size,
			  _("debug_abbrev section data"));
	if (!begin)
	  return 0;

	process_abbrev_section (begin + compunit.cu_abbrev_offset,
				begin + sec->sh_size, comp_unit_head);

	free (begin);
      }

      level = 0;
      while (tags < start)
	{
	  int bytes_read;
	  unsigned long abbrev_number;
	  abbrev_entry *entry;
	  abbrev_attr *attr;

	  abbrev_number = read_leb128 (tags, & bytes_read, 0);
	  tags += bytes_read;

	  /* A null DIE marks the end of a list of children.  */
	  if (abbrev_number == 0)
	    {
	      --level;
	      continue;
	    }

	  /* Scan through the abbreviation list until we reach the
	     correct entry.  */
	  for (entry = comp_unit_head->first_abbrev;
	       entry && entry->entry != abbrev_number;
	       entry = entry->next)
	    continue;

	  if (entry == NULL)
	    {
	      warn (_("Unable to locate entry %lu in the abbreviation table\n"),
		    abbrev_number);
	      return 0;
	    }

	  printf (_(" <%d><%lx>: Abbrev Number: %lu (%s)\n"),
		  level,
		  (unsigned long) (tags - section_begin - bytes_read),
		  abbrev_number,
		  get_TAG_name (entry->tag));

	  for (attr = entry->first_attr; attr; attr = attr->next)
	    tags = read_and_display_attr (attr->attribute,
					  attr->form,
					  tags, cu_offset,
					  compunit.cu_pointer_size,
					  offset_size,
					  compunit.cu_version);

	  if (entry->children)
	    ++level;
	}
    }

  free_debug_str ();
  free_debug_loc ();

  printf ("\n");

  return 1;
}
static int
display_debug_aranges (elf64_shdr *section,
		       unsigned char *start,
		       int file )
{
  unsigned char *end = start + section->sh_size;

  printf (_("The section %s contains:\n\n"), SECTION_NAME (section));

  while (start < end)
    {
      unsigned char *hdrptr;
      DWARF2_Internal_ARange arange;
      unsigned char *ranges;
      unsigned long length;
      unsigned long address;
      int excess;
      int offset_size;
      int initial_length_size;

      hdrptr = start;

      arange.ar_length = byte_get (hdrptr, 4);
      hdrptr += 4;

      if (arange.ar_length == 0xffffffff)
	{
	  arange.ar_length = byte_get (hdrptr, 8);
	  hdrptr += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
        {
	  offset_size = 4;
	  initial_length_size = 4;
	}

      arange.ar_version = byte_get (hdrptr, 2);
      hdrptr += 2;

      arange.ar_info_offset = byte_get (hdrptr, offset_size);
      hdrptr += offset_size;

      arange.ar_pointer_size = byte_get (hdrptr, 1);
      hdrptr += 1;

      arange.ar_segment_size = byte_get (hdrptr, 1);
      hdrptr += 1;

      if (arange.ar_version != 2 && arange.ar_version != 3)
	{
	  warn (_("Only DWARF 2 and 3 aranges are currently supported.\n"));
	  break;
	}

      printf (_("  Length:                   %ld\n"), arange.ar_length);
      printf (_("  Version:                  %d\n"), arange.ar_version);
      printf (_("  Offset into .debug_info:  %lx\n"), arange.ar_info_offset);
      printf (_("  Pointer Size:             %d\n"), arange.ar_pointer_size);
      printf (_("  Segment Size:             %d\n"), arange.ar_segment_size);

      printf (_("\n    Address  Length\n"));

      ranges = hdrptr;

      /* Must pad to an alignment boundary that is twice the pointer size.  */
      excess = (hdrptr - start) % (2 * arange.ar_pointer_size);
      if (excess)
	ranges += (2 * arange.ar_pointer_size) - excess;

      for (;;)
	{
	  address = byte_get (ranges, arange.ar_pointer_size);

	  ranges += arange.ar_pointer_size;

	  length  = byte_get (ranges, arange.ar_pointer_size);

	  ranges += arange.ar_pointer_size;

	  /* A pair of zeros marks the end of the list.  */
	  if (address == 0 && length == 0)
	    break;

	  printf ("    %8.8lx %lu\n", address, length);
	}

      start += arange.ar_length + initial_length_size;
    }

  printf ("\n");

  return 1;
}


struct
{
    const char *const name;
    int (*display) (elf64_shdr *, unsigned char *,int);
}
debug_displays[] =
{
    { ".debug_line",		display_debug_lines },
    { ".debug_info",		display_debug_info },
    { ".debug_aranges",		display_debug_aranges },
    { ".debug_frame",		display_debug_frames },
#if 0
    { ".debug_abbrev",		display_debug_abbrev },
    { ".debug_pubnames",		display_debug_pubnames },
    { ".eh_frame",		display_debug_frames },
    { ".debug_macinfo",		display_debug_macinfo },
    { ".debug_pubtypes",		display_debug_pubnames },
    { ".debug_ranges",		display_debug_not_supported },
    { ".debug_static_func",	display_debug_not_supported },
    { ".debug_static_vars",	display_debug_not_supported },
    { ".debug_types",		display_debug_not_supported },
    { ".debug_weaknames",		display_debug_not_supported }
#endif
};
static int
display_debug_section (elf64_shdr *section, int fd)
{
  char *name = SECTION_NAME (section);
  size_t length;
  unsigned char *start;
  int i;

  length = section->sh_size;
  if (length == 0)
    {
      printf (_("\nSection '%s' has no debugging data.\n"), name);
      return 0;
    }

  start = get_data (NULL, fd, section->sh_offset, length,
		    _("debug section data"));
  if (!start)
    return 0;

  /* See if we know how to display the contents of this section.  */
  if (strncmp (name, ".gnu.linkonce.wi.", 17) == 0)
    name = ".debug_info";

  for (i = NUM_ELEM (debug_displays); i--;)
    if (strcmp (debug_displays[i].name, name) == 0)
      {
	if (debug_displays[i].display)
	debug_displays[i].display (section, start, fd);
	break;
      }

  if (i == -1)
    printf (_("Unrecognized debug section: %s\n"), name);

  free (start);

  /* If we loaded in the abbrev section at some point,
     we must release it here.  */
  free_abbrevs ();

  return 1;
}

void
save_offset(int fd, unsigned long long *off)
{
    *off = lseek(fd, 0, SEEK_CUR);
}

void
restore_offset(int fd, unsigned long long off)
{
    lseek(fd, off, SEEK_SET);
}


void read_elf_header(int fd)
{
    elf_header = malloc (sizeof(elf64_ehdr));
    memset(elf_header, 0, sizeof (elf64_ehdr));
    read(fd, elf_header, sizeof(elf64_ehdr));
}
void read_section_headers(int fd)
{
    section_headers = get_data (NULL, fd, elf_header->e_shoff, elf_header->e_shentsize * elf_header->e_shnum, _("section headers"));

}
void read_str_table(int fd)
{
    string_table_header = malloc (sizeof(elf64_shdr));
    memset(string_table_header, 0, sizeof (elf64_shdr));

    lseek(fd, elf_header->e_shoff, SEEK_SET);
    lseek(fd, elf_header->e_shstrndx * sizeof (elf64_shdr), SEEK_CUR);
    read(fd, string_table_header, sizeof(elf64_shdr));

    string_table_length = string_table_header->sh_size;
    string_table = malloc (string_table_header->sh_size);
    lseek(fd, string_table_header->sh_offset, SEEK_SET);
    read(fd, string_table, string_table_header->sh_size);
}

void dump_section_headers(int fd)
{
    lseek(fd, elf_header->e_shoff, SEEK_SET);
    int k = 0;
    unsigned long long off = 0;
    elf64_shdr section_header;
    memset(&section_header, 0, sizeof(section_header));
    for(; k < elf_header->e_shnum; k++) {
        read(fd, &section_header, sizeof(section_header));
        char *name = string_table + section_header.sh_name;
        sdebug (" [%3d] = [%16s]\n", k, string_table + section_header.sh_name);
        if (0 == strncmp(name, ".debug_", 7)) {
            save_offset(fd, &off);
            display_debug_section(&section_header, fd);
            restore_offset(fd, off);
        }
        memset(&section_header, 0, sizeof(section_header));
    }
}

static int fd;

int handle_elf(struct argdata *arg)
{
    if (arg->v[1] && ( 0 == strcmp("cu", arg->v[1]))) {
        dump_dwarf_info(comp_unit_head);
    } else if (arg->v[1] == NULL) {
    }
}

int do_elf_load ()
{
    fd = open(CHILD_PROCESS, O_RDONLY);
    exit_on_error(fd == -1);

    read_elf_header(fd);
    read_str_table(fd);
    read_section_headers(fd);
    dump_section_headers(fd);

        return 0;
}

