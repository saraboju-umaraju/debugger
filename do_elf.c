#include "debug.h"


struct comp_unit
{
    unsigned long cu_offset;
    DWARF2_Internal_CompUnit cu;
    int offset_size;
    int initial_length_size;
    abbrev_entry *first_abbrev;
    abbrev_entry *last_abbrev;
    char *start;
    char *tags;
    struct comp_unit *next;
};

void list_cus(struct comp_unit *head)
{
    struct comp_unit *tmp = head;
    for(; tmp ; tmp = tmp->next) {
        debug("%3lu\n", tmp->cu_offset);
    }
}
#if 0
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
#endif

struct comp_unit *comp_unit_head = NULL;

static struct comp_unit *add_node(DWARF2_Internal_CompUnit *node, unsigned long cu_offset)
{
    struct comp_unit *tmp = (struct comp_unit*) malloc(sizeof(struct comp_unit));
    exit_on_error(tmp == NULL);
    tmp->cu_offset = cu_offset;
    memcpy(&tmp->cu, node, sizeof(DWARF2_Internal_CompUnit));
    tmp->first_abbrev = NULL;
    tmp->last_abbrev = NULL;
    tmp->next = NULL;
    return tmp;
}

struct comp_unit *comp_unit_add(struct comp_unit *head, DWARF2_Internal_CompUnit *node, unsigned long cu_offset ,int offset_size, int initial_length_size, char *start, char *tags)
{
    struct comp_unit *newnode = add_node(node, cu_offset);
    newnode->offset_size = offset_size;
    newnode->initial_length_size = initial_length_size;
    newnode->start = start;
    newnode->tags = tags;
    if (NULL == head) {
        return newnode;
    }else {
        newnode->next = head;
        //head = newnode;
        return newnode;
    }
}

const char *debug_str_contents;
size_t debug_str_size;
const char *debug_abbrev_contents;
size_t debug_abbrev_size;
const char *debug_info_contents;
size_t debug_info_size;
const char *debug_lines_contents;
size_t debug_lines_size;
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

    static void
free_abbrevs (void)
{
    abbrev_entry *abbrev;

    for (abbrev = NULL; abbrev;)
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

    //last_abbrev = first_abbrev = NULL;
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
load_debug_abbrev (int fd)
{
  elf64_shdr *sec;

  /* If it is already loaded, do nothing.  */
  if (debug_abbrev_contents != NULL)
    return;

  /* Locate the .debug_loc section.  */
  sec = find_section (".debug_abbrev");
  if (sec == NULL)
    return;

  debug_abbrev_size = sec->sh_size;

  debug_abbrev_contents = get_data (NULL, fd, sec->sh_offset, sec->sh_size,
				 _("debug_loc section data"));
}

static void
load_debug_info (int fd)
{
  elf64_shdr *sec;

  /* If it is already loaded, do nothing.  */
  if (debug_info_contents != NULL)
    return;

  /* Locate the .debug_loc section.  */
  sec = find_section (".debug_info");
  if (sec == NULL)
    return;

  debug_info_size = sec->sh_size;

  debug_info_contents = get_data (NULL, fd, sec->sh_offset, sec->sh_size,
				 _("debug_loc section data"));
}

static void
load_debug_lines (int fd)
{
  elf64_shdr *sec;

  /* If it is already loaded, do nothing.  */
  if (debug_lines_contents != NULL)
    return;

  /* Locate the .debug_loc section.  */
  sec = find_section (".debug_line");
  if (sec == NULL)
    return;

  debug_lines_size = sec->sh_size;

  debug_lines_contents = get_data (NULL, fd, sec->sh_offset, sec->sh_size,
				 _("debug_loc section data"));
  display_debug_lines (sec, (char *)debug_lines_contents, fd);

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
#if 0
    { ".debug_aranges",		display_debug_aranges },
    { ".debug_frame",		display_debug_frames },
    { ".debug_info",		display_debug_info },
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
static int oops_3(int fd, struct comp_unit *cu)
{
    char *tags = cu->tags;
    char *saved_tags = NULL;
    char *start = cu->start;
    struct attr_data *attrdata = malloc (sizeof(struct attr_data));
    memset(attrdata, 0, sizeof(struct attr_data));
    int level = 0;
    int bytes_read;
    unsigned long abbrev_number;
    abbrev_entry *entry;
    abbrev_attr *attr;
    abbrev_attr *tmpattr;

    for (entry = cu->first_abbrev;
            entry ;
            entry = entry->next) {
        printf (_(" <%d><%lx>: Abbrev Number: %lu (%s)\n"),
                level,
                0l,
                entry->saved_entry,
                get_TAG_name (entry->tag));
        for (attr = entry->first_attr; attr; attr = attr->next) {
            tags = read_and_display_attr (attr->attribute,
                    attr->form,
                    attr->saved_addr, cu->cu_offset,
                    cu->cu.cu_pointer_size,
                    cu->offset_size,
                    cu->cu.cu_version);
            if (entry->children)
                ++level;
        }
    }
    return 1;
}
static int oops_2(int fd, struct comp_unit *cu)
{
    char *tags = cu->tags;
    char *saved_tags = NULL;
    char *start = cu->start;
    struct attr_data *attrdata = malloc (sizeof(struct attr_data));
    memset(attrdata, 0, sizeof(struct attr_data));
    int level = 0;
    while (tags < start)
    {
        int bytes_read;
        unsigned long abbrev_number;
        abbrev_entry *entry;
        abbrev_attr *attr;
        abbrev_attr *tmpattr;

        abbrev_number = read_leb128 (tags, & bytes_read, 0);
        tags += bytes_read;

        if (abbrev_number == 0)
        {
            --level;
            continue;
        }

        for (entry = cu->first_abbrev;
                entry && entry->entry != abbrev_number;
                entry = entry->next)
            continue;
        entry->saved_entry = abbrev_number;

        if (entry == NULL)
        {
            warn (_("Unable to locate entry %lu in the abbreviation table\n"),
                    abbrev_number);
            return 0;
        }
        saved_tags = tags;
        for (attr = entry->first_attr; attr; attr = attr->next) {
            attr->saved_addr = tags;
            tags = read_attr (attr->attribute,
                    attr->form,
                    tags, cu->cu_offset,
                    cu->cu.cu_pointer_size,
                    cu->offset_size,
                    cu->cu.cu_version,
                    attrdata);
        }

        if (entry->children)
            ++level;
    }
    return 1;
}

static int oops(int fd, struct comp_unit *cu, enum dwarf_tag tag)
{
    char *tags = cu->tags;
    char *saved_tags = NULL;
    char *start = cu->start;
    struct attr_data *attrdata = malloc (sizeof(struct attr_data));
    memset(attrdata, 0, sizeof(struct attr_data));
    int level = 0;
    unsigned long low_pc = 0;
    unsigned long high_pc = 0;
    unsigned long pc = 0x4007e5;
    while (tags < start)
    {
        int bytes_read;
        unsigned long abbrev_number;
        abbrev_entry *entry;
        abbrev_attr *attr;
        abbrev_attr *tmpattr;

        abbrev_number = read_leb128 (tags, & bytes_read, 0);
        tags += bytes_read;

        if (abbrev_number == 0)
        {
            --level;
            continue;
        }

        /* Scan through the abbreviation list until we reach the
           correct entry.  */
        for (entry = cu->first_abbrev;
                entry && entry->entry != abbrev_number;
                entry = entry->next)
            continue;

        if (entry == NULL)
        {
            warn (_("Unable to locate entry %lu in the abbreviation table\n"),
                    abbrev_number);
            return 0;
        }
        saved_tags = tags;
        for (attr = entry->first_attr; attr; attr = attr->next) {
            tags = read_attr (attr->attribute,
                    attr->form,
                    tags, cu->cu_offset,
                    cu->cu.cu_pointer_size,
                    cu->offset_size,
                    cu->cu.cu_version,
                    attrdata);
            if (entry->tag == tag)  {
                if (attr->attribute == DW_AT_name) {
                    sdebug ("%lx %s %lu\n", attrdata->uvalue, attrdata->svalue, attrdata->indirect);
                    sdebug("name := %s\n", fetch_indirect_string(attrdata->uvalue));
                    if (strcmp(fetch_indirect_string(attrdata->uvalue), "main") == 0)  {
                        memset(attrdata, 0, sizeof(struct attr_data));
                        for (tmpattr = entry->first_attr; tmpattr; tmpattr = tmpattr->next) {
                            saved_tags = read_and_display_attr (tmpattr->attribute,
                                    tmpattr->form,
                                    saved_tags, cu->cu_offset,
                                    cu->cu.cu_pointer_size,
                                    cu->offset_size,
                                    cu->cu.cu_version);
                        }
                    }
                }
                if (attr->attribute == DW_AT_low_pc) {
                    low_pc = attrdata->uvalue;
                }
                if (attr->attribute == DW_AT_high_pc) {
                    high_pc = attrdata->uvalue;
                }
                if ( pc >= low_pc && pc <= high_pc) {
                    printf (" __UMA__ %s %s %d\n",__FILE__,__func__,__LINE__);
                }
            }
        }

        if (entry->children)
            ++level;
    }
    return 1;
}

static int sub_program_containing_pc(int fd, struct comp_unit *cu, unsigned long pc)
{
    char *tags = cu->tags;
    char *saved_tags = NULL;
    char *start = cu->start;
    struct attr_data *attrdata = malloc (sizeof(struct attr_data));
    memset(attrdata, 0, sizeof(struct attr_data));
    int level = 0;
    unsigned long low_pc = 0;
    unsigned long high_pc = 0;
    enum dwarf_tag tag = DW_TAG_subprogram;
    pc = 0x4007e5;
    while (tags < start)
    {
        int bytes_read;
        unsigned long abbrev_number;
        abbrev_entry *entry;
        abbrev_attr *attr;
        abbrev_attr *tmpattr;

        abbrev_number = read_leb128 (tags, & bytes_read, 0);
        tags += bytes_read;

        if (abbrev_number == 0)
        {
            --level;
            continue;
        }

        /* Scan through the abbreviation list until we reach the
           correct entry.  */
        for (entry = cu->first_abbrev;
                entry && entry->entry != abbrev_number;
                entry = entry->next)
            continue;

        if (entry == NULL)
        {
            warn (_("Unable to locate entry %lu in the abbreviation table\n"),
                    abbrev_number);
            return 0;
        }
        saved_tags = tags;
        for (attr = entry->first_attr; attr; attr = attr->next) {
            tags = read_attr (attr->attribute,
                    attr->form,
                    tags, cu->cu_offset,
                    cu->cu.cu_pointer_size,
                    cu->offset_size,
                    cu->cu.cu_version,
                    attrdata);
            if (entry->tag == tag)  {
                if (attr->attribute == DW_AT_name) {
                    sdebug ("%lx %s %lu\n", attrdata->uvalue, attrdata->svalue, attrdata->indirect);
                    sdebug("name := %s\n", fetch_indirect_string(attrdata->uvalue));
                    if (strcmp(fetch_indirect_string(attrdata->uvalue), "main") == 0)  {
                        memset(attrdata, 0, sizeof(struct attr_data));
                        for (tmpattr = entry->first_attr; tmpattr; tmpattr = tmpattr->next) {
                            saved_tags = read_and_display_attr (tmpattr->attribute,
                                    tmpattr->form,
                                    saved_tags, cu->cu_offset,
                                    cu->cu.cu_pointer_size,
                                    cu->offset_size,
                                    cu->cu.cu_version);
                        }
                    }
                }
                if (attr->attribute == DW_AT_low_pc) {
                    low_pc = attrdata->uvalue;
                }
                if (attr->attribute == DW_AT_high_pc) {
                    high_pc = attrdata->uvalue;
                }
                if ( pc >= low_pc && pc <= high_pc) {
                    printf (" __UMA__ %s %s %d\n",__FILE__,__func__,__LINE__);
                }
            }
        }

        if (entry->children)
            ++level;
    }
    return 1;
}

#if 1
static int go_through_and_find(int fd, struct comp_unit *cu, enum dwarf_tag tag)
{
    char *tags = cu->tags;
    char *start = cu->start;
      int level = 0;
    struct attr_data *attrdata = malloc (sizeof(struct attr_data));
    memset(attrdata, 0, sizeof(struct attr_data));
      while (tags < start)
	{
	  int bytes_read;
	  unsigned long abbrev_number;
	  abbrev_entry *entry;
	  abbrev_attr *attr;

	  abbrev_number = read_leb128 (tags, & bytes_read, 0);
	  tags += bytes_read;

	  if (abbrev_number == 0)
	    {
	      --level;
	      continue;
	    }

	  /* Scan through the abbreviation list until we reach the
	     correct entry.  */
	  for (entry = cu->first_abbrev;
	       entry && entry->entry != abbrev_number;
	       entry = entry->next)
	    continue;

	  if (entry == NULL)
	    {
	      warn (_("Unable to locate entry %lu in the abbreviation table\n"),
		    abbrev_number);
	      return 0;
	    }
      if (entry->tag != tag)  {
          for (attr = entry->first_attr; attr; attr = attr->next)
              tags = read_attr (attr->attribute,
                      attr->form,
                      tags, cu->cu_offset,
                      cu->cu.cu_pointer_size,
                      cu->offset_size,
                      cu->cu.cu_version, attrdata);
      } else {

          for (attr = entry->first_attr; attr; attr = attr->next)
              tags = read_and_display_attr (attr->attribute,
                      attr->form,
                      tags, cu->cu_offset,
                      cu->cu.cu_pointer_size,
                      cu->offset_size,
                      cu->cu.cu_version);
      }

	  if (entry->children)
	    ++level;
	}
	      return 1;
}

static int display_child_dies(struct comp_unit *cu)
{
    char *tags = cu->tags;
    char *start = cu->start;
      int level = 0;
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
	  for (entry = cu->first_abbrev;
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
		  (unsigned long) (tags - debug_info_contents - bytes_read),
		  abbrev_number,
		  get_TAG_name (entry->tag));

	  for (attr = entry->first_attr; attr; attr = attr->next)
	    tags = read_and_display_attr (attr->attribute,
					  attr->form,
					  tags, cu->cu_offset,
					  cu->cu.cu_pointer_size,
					  cu->offset_size,
					  cu->cu.cu_version);

	  if (entry->children)
	    ++level;
	}
	      return 1;
}
#endif
static int
dump_dwarf_info (int fd, struct comp_unit *compptr)
{
    unsigned char *start = (unsigned char *) debug_info_contents;
    unsigned char *end = start + debug_info_size;
    unsigned char *section_begin = start;

    printf (_("The section %s contains:\n\n"), ".debug_info");

    {
        unsigned char *hdrptr;
        unsigned char *tags;
        int level;
        hdrptr = start;
        hdrptr += 4;
        hdrptr += 2;
        hdrptr += compptr->offset_size;
        hdrptr += 1;
        tags = hdrptr;
        start += compptr->cu.cu_length + compptr->initial_length_size;

        printf (_("  Compilation Unit @ %lx:\n"), compptr->cu_offset);
        printf (_("   Length:        %ld\n"), compptr->cu.cu_length);
        printf (_("   Version:       %d\n"), compptr->cu.cu_version);
        printf (_("   Abbrev Offset: %ld\n"), compptr->cu.cu_abbrev_offset);
        printf (_("   Pointer Size:  %d\n"), compptr->cu.cu_pointer_size);

        display_child_dies(compptr);
    }

    printf ("\n");

    return 1;
}

static int
process_debug_info (int fd)
{
  unsigned char *start = (unsigned char *) debug_info_contents;
  unsigned char *end = start + debug_info_size;
  unsigned char *section_begin = start;

  //printf (_("The section %s contains:\n\n"), ".debug_info");

  while (start < end)
    {
      DWARF2_Internal_CompUnit compunit;
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

      cu_abbrev_offset_ptr = hdrptr;
      compunit.cu_abbrev_offset = byte_get (hdrptr, offset_size);
      hdrptr += offset_size;

      compunit.cu_pointer_size = byte_get (hdrptr, 1);
      hdrptr += 1;

      tags = hdrptr;
      cu_offset = start - section_begin;
      start += compunit.cu_length + initial_length_size;
      comp_unit_head = comp_unit_add(comp_unit_head,  &compunit, cu_offset, offset_size, initial_length_size, start, tags);

      if (compunit.cu_version != 2 && compunit.cu_version != 3)
	{
	  warn (_("Only version 2 and 3 DWARF debug information is currently supported.\n"));
	  continue;
	}

      /* Read in the abbrevs used by this compilation unit.  */
	unsigned char *begin = (char *) debug_abbrev_contents;

	process_abbrev_section (begin + compunit.cu_abbrev_offset,
				begin + debug_info_size, comp_unit_head);

    }


  printf ("\n");

  return 1;
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
            //display_debug_section(&section_header, fd);
            restore_offset(fd, off);
        }
        memset(&section_header, 0, sizeof(section_header));
    }
}

static int fd;

int dump_compilation_units(int fd, struct comp_unit *compptr)
{
        printf (_("  Compilation Unit @ %lx:\n"), compptr->cu_offset);
        printf (_("   Length:        %ld\n"), compptr->cu.cu_length);
        printf (_("   Version:       %d\n"), compptr->cu.cu_version);
        printf (_("   Abbrev Offset: %ld\n"), compptr->cu.cu_abbrev_offset);
        printf (_("   Pointer Size:  %d\n"), compptr->cu.cu_pointer_size);
}

int print_dwarf_info(struct comp_unit *tmp, int (*func)(int, struct comp_unit*))
{
    if (tmp) {
        print_dwarf_info(tmp->next, func);
        func(0, tmp);
    }
}
int print_dwarf_info_with_tag(struct comp_unit *tmp, int (*func)(int, struct comp_unit*, enum dwarf_tag), enum dwarf_tag tag)
{
    if (tmp) {
        print_dwarf_info_with_tag(tmp->next, func, tag);
        return func(0, tmp, tag);
    }
    return 0;
}

int handle_elf(struct argdata *arg)
{
    if (arg->v[1] && ( 0 == strcmp("dw", arg->v[1]))) {
        print_dwarf_info(comp_unit_head, dump_dwarf_info);
    } else if (arg->v[1] && ( 0 == strcmp("cu", arg->v[1]))) {
        print_dwarf_info(comp_unit_head, dump_compilation_units);
    } else if (arg->v[1] && ( 0 == strcmp("sub", arg->v[1]))) {
        print_dwarf_info_with_tag(comp_unit_head, go_through_and_find, DW_TAG_subprogram);
    } else if (arg->v[1] && ( 0 == strcmp("cus", arg->v[1]))) {
        print_dwarf_info_with_tag(comp_unit_head, go_through_and_find, DW_TAG_compile_unit);
    } else if (arg->v[1] && ( 0 == strcmp("variable", arg->v[1]))) {
        print_dwarf_info_with_tag(comp_unit_head, go_through_and_find, DW_TAG_variable);
    } else if (arg->v[1] && ( 0 == strcmp("struct", arg->v[1]))) {
        print_dwarf_info_with_tag(comp_unit_head, go_through_and_find, DW_TAG_structure_type);
    } else if (arg->v[1] && ( 0 == strcmp("line", arg->v[1]))) {
    display_line_info_uma_2 (0);
    } else if (arg->v[1] == NULL) {
        print_dwarf_info_with_tag(comp_unit_head, oops, DW_TAG_subprogram);
    }
}

static int
load_required_sections()
{
    load_debug_str (fd);
    load_debug_loc (fd);
    load_debug_abbrev(fd);
    load_debug_info(fd);
}

int do_elf_load ()
{
    fd = open(CHILD_PROCESS, O_RDONLY);
    exit_on_error(fd == -1);

    read_elf_header(fd);
    read_str_table(fd);
    read_section_headers(fd);
    load_required_sections();
    process_debug_info(0);
    //dump_section_headers(fd);
    load_debug_lines(fd);
    print_dwarf_info(comp_unit_head, oops_2);
    //print_dwarf_info(comp_unit_head, oops_3);
    display_line_info_uma (0);

    return 0;
}

