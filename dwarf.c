#include "debug.h"

#define SECTION_NAME(X)	((X) == NULL ? "<none>" : \
        ((X)->sh_name >= string_table_length \
         ? "<corrupt>" : string_table + (X)->sh_name))
#define NUM_ELEM(array) 	(sizeof (array) / sizeof ((array)[0]))
#define error debug
#define warn debug
#define _(...) __VA_ARGS__
#include "dwarf.h"
#define xmalloc malloc
#define xrealloc realloc
int do_debug_frames_interp;

static const char *debug_str_contents;
static size_t debug_str_size;
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

typedef struct abbrev_attr
{
    unsigned long attribute;
    unsigned long form;
    unsigned char *saved_addr;
    struct abbrev_attr *next;
}
abbrev_attr;

typedef struct abbrev_entry
{
    unsigned long entry;
    unsigned long tag;
    int children;
    struct abbrev_attr *first_attr;
    struct abbrev_attr *last_attr;
    struct abbrev_entry *next;
}
abbrev_entry;

static abbrev_entry *first_abbrev = NULL;
static abbrev_entry *last_abbrev = NULL;
static unsigned long int
read_leb128 (unsigned char *data, int *length_return, int sign)
{
  unsigned long int result = 0;
  unsigned int num_read = 0;
  int shift = 0;
  unsigned char byte;

  do
    {
      byte = *data++;
      num_read++;

      result |= (byte & 0x7f) << shift;

      shift += 7;

    }
  while (byte & 0x80);

  if (length_return != NULL)
    *length_return = num_read;

  if (sign && (shift < 32) && (byte & 0x40))
    result |= -1 << shift;

  return result;
}
static const char *
fetch_indirect_string (unsigned long offset)
{
  if (debug_str_contents == NULL)
    return _("<no .debug_str section>");

  if (offset > debug_str_size)
    return _("<offset is too big>");

  return debug_str_contents + offset;
}
uint64_t
byte_get (const unsigned char *field, unsigned int size)
{
  switch (size)
    {
    case 1:
      return *field;

    case 2:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8));

    case 3:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8)
	      | ((uint64_t) field[2] << 16));

    case 4:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8)
	      | ((uint64_t) field[2] << 16)
	      | ((uint64_t) field[3] << 24));

    case 5:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8)
	      | ((uint64_t) field[2] << 16)
	      | ((uint64_t) field[3] << 24)
	      | ((uint64_t) field[4] << 32));

    case 6:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8)
	      | ((uint64_t) field[2] << 16)
	      | ((uint64_t) field[3] << 24)
	      | ((uint64_t) field[4] << 32)
	      | ((uint64_t) field[5] << 40));

    case 7:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8)
	      | ((uint64_t) field[2] << 16)
	      | ((uint64_t) field[3] << 24)
	      | ((uint64_t) field[4] << 32)
	      | ((uint64_t) field[5] << 40)
	      | ((uint64_t) field[6] << 48));

    case 8:
      return ((uint64_t) field[0]
	      | ((uint64_t) field[1] << 8)
	      | ((uint64_t) field[2] << 16)
	      | ((uint64_t) field[3] << 24)
	      | ((uint64_t) field[4] << 32)
	      | ((uint64_t) field[5] << 40)
	      | ((uint64_t) field[6] << 48)
	      | ((uint64_t) field[7] << 56));

    default:
      error (_("Unhandled data length: %d\n"), size);
      abort ();
    }
}
static unsigned char *
display_block (unsigned char *data, unsigned long length)
{
  printf (_(" %lu byte block: "), length);

  while (length --)
    printf ("%lx ", (unsigned long) byte_get (data++, 1));

  return data;
}
static void
decode_location_expression (unsigned char * data,
			    unsigned int pointer_size,
			    unsigned long length)
{
  unsigned op;
  int bytes_read;
  unsigned long uvalue;
  unsigned char *end = data + length;

  while (data < end)
    {
      op = *data++;

      switch (op)
	{
	case DW_OP_addr:
	  printf ("DW_OP_addr: %lx",
		  (unsigned long) byte_get (data, pointer_size));
	  data += pointer_size;
	  break;
	case DW_OP_deref:
	  printf ("DW_OP_deref");
	  break;
	case DW_OP_const1u:
	  printf ("DW_OP_const1u: %lu", (unsigned long) byte_get (data++, 1));
	  break;
	case DW_OP_const1s:
	  printf ("DW_OP_const1s: %ld", (long) byte_get (data++, 1));
	  break;
	case DW_OP_const2u:
	  printf ("DW_OP_const2u: %lu", (unsigned long) byte_get (data, 2));
	  data += 2;
	  break;
	case DW_OP_const2s:
	  printf ("DW_OP_const2s: %ld", (long) byte_get (data, 2));
	  data += 2;
	  break;
	case DW_OP_const4u:
	  printf ("DW_OP_const4u: %lu", (unsigned long) byte_get (data, 4));
	  data += 4;
	  break;
	case DW_OP_const4s:
	  printf ("DW_OP_const4s: %ld", (long) byte_get (data, 4));
	  data += 4;
	  break;
	case DW_OP_const8u:
	  printf ("DW_OP_const8u: %lu %lu", (unsigned long) byte_get (data, 4),
		  (unsigned long) byte_get (data + 4, 4));
	  data += 8;
	  break;
	case DW_OP_const8s:
	  printf ("DW_OP_const8s: %ld %ld", (long) byte_get (data, 4),
		  (long) byte_get (data + 4, 4));
	  data += 8;
	  break;
	case DW_OP_constu:
	  printf ("DW_OP_constu: %lu", read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_consts:
	  printf ("DW_OP_consts: %ld", read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;
	case DW_OP_dup:
	  printf ("DW_OP_dup");
	  break;
	case DW_OP_drop:
	  printf ("DW_OP_drop");
	  break;
	case DW_OP_over:
	  printf ("DW_OP_over");
	  break;
	case DW_OP_pick:
	  printf ("DW_OP_pick: %ld", (unsigned long) byte_get (data++, 1));
	  break;
	case DW_OP_swap:
	  printf ("DW_OP_swap");
	  break;
	case DW_OP_rot:
	  printf ("DW_OP_rot");
	  break;
	case DW_OP_xderef:
	  printf ("DW_OP_xderef");
	  break;
	case DW_OP_abs:
	  printf ("DW_OP_abs");
	  break;
	case DW_OP_and:
	  printf ("DW_OP_and");
	  break;
	case DW_OP_div:
	  printf ("DW_OP_div");
	  break;
	case DW_OP_minus:
	  printf ("DW_OP_minus");
	  break;
	case DW_OP_mod:
	  printf ("DW_OP_mod");
	  break;
	case DW_OP_mul:
	  printf ("DW_OP_mul");
	  break;
	case DW_OP_neg:
	  printf ("DW_OP_neg");
	  break;
	case DW_OP_not:
	  printf ("DW_OP_not");
	  break;
	case DW_OP_or:
	  printf ("DW_OP_or");
	  break;
	case DW_OP_plus:
	  printf ("DW_OP_plus");
	  break;
	case DW_OP_plus_uconst:
	  printf ("DW_OP_plus_uconst: %lu",
		  read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_shl:
	  printf ("DW_OP_shl");
	  break;
	case DW_OP_shr:
	  printf ("DW_OP_shr");
	  break;
	case DW_OP_shra:
	  printf ("DW_OP_shra");
	  break;
	case DW_OP_xor:
	  printf ("DW_OP_xor");
	  break;
	case DW_OP_bra:
	  printf ("DW_OP_bra: %ld", (long) byte_get (data, 2));
	  data += 2;
	  break;
	case DW_OP_eq:
	  printf ("DW_OP_eq");
	  break;
	case DW_OP_ge:
	  printf ("DW_OP_ge");
	  break;
	case DW_OP_gt:
	  printf ("DW_OP_gt");
	  break;
	case DW_OP_le:
	  printf ("DW_OP_le");
	  break;
	case DW_OP_lt:
	  printf ("DW_OP_lt");
	  break;
	case DW_OP_ne:
	  printf ("DW_OP_ne");
	  break;
	case DW_OP_skip:
	  printf ("DW_OP_skip: %ld", (long) byte_get (data, 2));
	  data += 2;
	  break;

	case DW_OP_lit0:
	case DW_OP_lit1:
	case DW_OP_lit2:
	case DW_OP_lit3:
	case DW_OP_lit4:
	case DW_OP_lit5:
	case DW_OP_lit6:
	case DW_OP_lit7:
	case DW_OP_lit8:
	case DW_OP_lit9:
	case DW_OP_lit10:
	case DW_OP_lit11:
	case DW_OP_lit12:
	case DW_OP_lit13:
	case DW_OP_lit14:
	case DW_OP_lit15:
	case DW_OP_lit16:
	case DW_OP_lit17:
	case DW_OP_lit18:
	case DW_OP_lit19:
	case DW_OP_lit20:
	case DW_OP_lit21:
	case DW_OP_lit22:
	case DW_OP_lit23:
	case DW_OP_lit24:
	case DW_OP_lit25:
	case DW_OP_lit26:
	case DW_OP_lit27:
	case DW_OP_lit28:
	case DW_OP_lit29:
	case DW_OP_lit30:
	case DW_OP_lit31:
	  printf ("DW_OP_lit%d", op - DW_OP_lit0);
	  break;

	case DW_OP_reg0:
	case DW_OP_reg1:
	case DW_OP_reg2:
	case DW_OP_reg3:
	case DW_OP_reg4:
	case DW_OP_reg5:
	case DW_OP_reg6:
	case DW_OP_reg7:
	case DW_OP_reg8:
	case DW_OP_reg9:
	case DW_OP_reg10:
	case DW_OP_reg11:
	case DW_OP_reg12:
	case DW_OP_reg13:
	case DW_OP_reg14:
	case DW_OP_reg15:
	case DW_OP_reg16:
	case DW_OP_reg17:
	case DW_OP_reg18:
	case DW_OP_reg19:
	case DW_OP_reg20:
	case DW_OP_reg21:
	case DW_OP_reg22:
	case DW_OP_reg23:
	case DW_OP_reg24:
	case DW_OP_reg25:
	case DW_OP_reg26:
	case DW_OP_reg27:
	case DW_OP_reg28:
	case DW_OP_reg29:
	case DW_OP_reg30:
	case DW_OP_reg31:
	  printf ("DW_OP_reg%d", op - DW_OP_reg0);
	  break;

	case DW_OP_breg0:
	case DW_OP_breg1:
	case DW_OP_breg2:
	case DW_OP_breg3:
	case DW_OP_breg4:
	case DW_OP_breg5:
	case DW_OP_breg6:
	case DW_OP_breg7:
	case DW_OP_breg8:
	case DW_OP_breg9:
	case DW_OP_breg10:
	case DW_OP_breg11:
	case DW_OP_breg12:
	case DW_OP_breg13:
	case DW_OP_breg14:
	case DW_OP_breg15:
	case DW_OP_breg16:
	case DW_OP_breg17:
	case DW_OP_breg18:
	case DW_OP_breg19:
	case DW_OP_breg20:
	case DW_OP_breg21:
	case DW_OP_breg22:
	case DW_OP_breg23:
	case DW_OP_breg24:
	case DW_OP_breg25:
	case DW_OP_breg26:
	case DW_OP_breg27:
	case DW_OP_breg28:
	case DW_OP_breg29:
	case DW_OP_breg30:
	case DW_OP_breg31:
	  printf ("DW_OP_breg%d: %ld", op - DW_OP_breg0,
		  read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;

	case DW_OP_regx:
	  printf ("DW_OP_regx: %lu", read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_fbreg:
	  printf ("DW_OP_fbreg: %ld", read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;
	case DW_OP_bregx:
	  uvalue = read_leb128 (data, &bytes_read, 0);
	  data += bytes_read;
	  printf ("DW_OP_bregx: %lu %ld", uvalue,
		  read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;
	case DW_OP_piece:
	  printf ("DW_OP_piece: %lu", read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_deref_size:
	  printf ("DW_OP_deref_size: %ld", (long) byte_get (data++, 1));
	  break;
	case DW_OP_xderef_size:
	  printf ("DW_OP_xderef_size: %ld", (long) byte_get (data++, 1));
	  break;
	case DW_OP_nop:
	  printf ("DW_OP_nop");
	  break;

	  /* DWARF 3 extensions.  */
	case DW_OP_push_object_address:
	  printf ("DW_OP_push_object_address");
	  break;
	case DW_OP_call2:
	  printf ("DW_OP_call2: <%lx>", (long) byte_get (data, 2));
	  data += 2;
	  break;
	case DW_OP_call4:
	  printf ("DW_OP_call4: <%lx>", (long) byte_get (data, 4));
	  data += 4;
	  break;
	case DW_OP_call_ref:
	  printf ("DW_OP_call_ref");
	  break;

	  /* GNU extensions.  */
	case DW_OP_GNU_push_tls_address:
	  printf ("DW_OP_GNU_push_tls_address");
	  break;

	default:
	  if (op >= DW_OP_lo_user
	      && op <= DW_OP_hi_user)
	    printf (_("(User defined location op)"));
	  else
	    printf (_("(Unknown location op)"));
	  /* No way to tell where the next op is, so just bail.  */
	  return;
	}

      /* Separate the ops.  */
      if (data < end)
	printf ("; ");
    }
}


    static void *
get_data (void *var, int fd, long offset, size_t size, const char *reason)
{
    void *mvar;

    if (size == 0)
        return NULL;

    if (lseek (fd, offset, SEEK_SET))
    {
    }

    mvar = var;
    if (mvar == NULL)
    {
        mvar = malloc (size);

        if (mvar == NULL)
        {
            exit_on_error (mvar == NULL);
        }
    }

    if (read (fd, mvar, size) != 1)
    {
    }

    return mvar;
}

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
add_abbrev (unsigned long number, unsigned long tag, int children)
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

    if (first_abbrev == NULL)
        first_abbrev = entry;
    else
        last_abbrev->next = entry;

    last_abbrev = entry;
}

    static void
add_abbrev_attr (unsigned long attribute, unsigned long form)
{
    abbrev_attr *attr;

    attr = malloc (sizeof (*attr));

    if (attr == NULL)
        /* ugg */
        return;

    attr->attribute = attribute;
    attr->form      = form;
    attr->next      = NULL;

    if (last_abbrev->first_attr == NULL)
        last_abbrev->first_attr = attr;
    else
        last_abbrev->last_attr->next = attr;

    last_abbrev->last_attr = attr;
}



    static unsigned char *
process_abbrev_section (unsigned char *start, unsigned char *end)
{
    if (first_abbrev != NULL)
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

        add_abbrev (entry, tag, children);

        do
        {
            unsigned long form;

            attribute = read_leb128 (start, & bytes_read, 0);
            start += bytes_read;

            form = read_leb128 (start, & bytes_read, 0);
            start += bytes_read;

            if (attribute != 0)
                add_abbrev_attr (attribute, form);
        }
        while (attribute != 0);
    }

    return NULL;
}

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
static elf64_shdr *
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
static unsigned char *
read_and_display_attr_value (unsigned long attribute,
			     unsigned long form,
			     unsigned char *data,
			     unsigned long cu_offset,
			     unsigned long pointer_size,
			     unsigned long offset_size,
			     int dwarf_version)
{
  unsigned long uvalue = 0;
  unsigned char *block_start = NULL;
  int bytes_read;

  switch (form)
    {
    default:
      break;

    case DW_FORM_ref_addr:
      if (dwarf_version == 2)
	{
	  uvalue = byte_get (data, pointer_size);
	  data += pointer_size;
	}
      else if (dwarf_version == 3)
	{
	  uvalue = byte_get (data, offset_size);
	  data += offset_size;
	}
      else
        {
	  error (_("Internal error: DWARF version is not 2 or 3.\n"));
	}
      break;

    case DW_FORM_addr:
      uvalue = byte_get (data, pointer_size);
      data += pointer_size;
      break;

    case DW_FORM_strp:
      uvalue = byte_get (data, offset_size);
      data += offset_size;
      break;

    case DW_FORM_ref1:
    case DW_FORM_flag:
    case DW_FORM_data1:
      uvalue = byte_get (data++, 1);
      break;

    case DW_FORM_ref2:
    case DW_FORM_data2:
      uvalue = byte_get (data, 2);
      data += 2;
      break;

    case DW_FORM_ref4:
    case DW_FORM_data4:
      uvalue = byte_get (data, 4);
      data += 4;
      break;

    case DW_FORM_sdata:
      uvalue = read_leb128 (data, & bytes_read, 1);
      data += bytes_read;
      break;

    case DW_FORM_ref_udata:
    case DW_FORM_udata:
      uvalue = read_leb128 (data, & bytes_read, 0);
      data += bytes_read;
      break;

    case DW_FORM_indirect:
      form = read_leb128 (data, & bytes_read, 0);
      data += bytes_read;
      printf (" %s", get_FORM_name (form));
      return read_and_display_attr_value (attribute, form, data, cu_offset,
					  pointer_size, offset_size,
					  dwarf_version);
    }

  switch (form)
    {
    case DW_FORM_ref_addr:
      printf (" <#%lx>", uvalue);
      break;

    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref_udata:
      printf (" <%lx>", uvalue + cu_offset);
      break;

    case DW_FORM_addr:
      printf (" %#lx", uvalue);
      break;

    case DW_FORM_flag:
    case DW_FORM_data1:
    case DW_FORM_data2:
    case DW_FORM_data4:
    case DW_FORM_sdata:
    case DW_FORM_udata:
      printf (" %ld", uvalue);
      break;

    case DW_FORM_ref8:
    case DW_FORM_data8:
      uvalue = byte_get (data, 4);
      printf (" %lx", uvalue);
      printf (" %lx", (unsigned long) byte_get (data + 4, 4));
      data += 8;
      break;

    case DW_FORM_string:
      printf (" %s", data);
      data += strlen ((char *) data) + 1;
      break;

    case DW_FORM_block:
      uvalue = read_leb128 (data, & bytes_read, 0);
      block_start = data + bytes_read;
      data = display_block (block_start, uvalue);
      break;

    case DW_FORM_block1:
      uvalue = byte_get (data, 1);
      block_start = data + 1;
      data = display_block (block_start, uvalue);
      break;

    case DW_FORM_block2:
      uvalue = byte_get (data, 2);
      block_start = data + 2;
      data = display_block (block_start, uvalue);
      break;

    case DW_FORM_block4:
      uvalue = byte_get (data, 4);
      block_start = data + 4;
      data = display_block (block_start, uvalue);
      break;

    case DW_FORM_strp:
      printf (_(" (indirect string, offset: 0x%lx): %s"),
	      uvalue, fetch_indirect_string (uvalue));
      break;

    case DW_FORM_indirect:
      /* Handled above.  */
      break;

    default:
      //warn (_("Unrecognized form: %d\n"), form);
      break;
    }

  /* For some attributes we can display further information.  */

  printf ("\t");

  switch (attribute)
    {
    case DW_AT_inline:
      switch (uvalue)
	{
	case DW_INL_not_inlined:
	  printf (_("(not inlined)"));
	  break;
	case DW_INL_inlined:
	  printf (_("(inlined)"));
	  break;
	case DW_INL_declared_not_inlined:
	  printf (_("(declared as inline but ignored)"));
	  break;
	case DW_INL_declared_inlined:
	  printf (_("(declared as inline and inlined)"));
	  break;
	default:
	  printf (_("  (Unknown inline attribute value: %lx)"), uvalue);
	  break;
	}
      break;

    case DW_AT_language:
      switch (uvalue)
	{
	case DW_LANG_C:			printf ("(non-ANSI C)"); break;
	case DW_LANG_C89:		printf ("(ANSI C)"); break;
	case DW_LANG_C_plus_plus:	printf ("(C++)"); break;
	case DW_LANG_Fortran77:		printf ("(FORTRAN 77)"); break;
	case DW_LANG_Fortran90:		printf ("(Fortran 90)"); break;
	case DW_LANG_Modula2:		printf ("(Modula 2)"); break;
	case DW_LANG_Pascal83:		printf ("(ANSI Pascal)"); break;
	case DW_LANG_Ada83:		printf ("(Ada)"); break;
	case DW_LANG_Cobol74:		printf ("(Cobol 74)"); break;
	case DW_LANG_Cobol85:		printf ("(Cobol 85)"); break;
	  /* DWARF 2.1 values.	*/
	case DW_LANG_C99:		printf ("(ANSI C99)"); break;
	case DW_LANG_Ada95:		printf ("(ADA 95)"); break;
	case DW_LANG_Fortran95:		printf ("(Fortran 95)"); break;
	  /* MIPS extension.  */
	case DW_LANG_Mips_Assembler:	printf ("(MIPS assembler)"); break;
	  /* UPC extension.  */
	case DW_LANG_Upc:		printf ("(Unified Parallel C)"); break;
	default:
	  printf ("(Unknown: %lx)", uvalue);
	  break;
	}
      break;

    case DW_AT_encoding:
      switch (uvalue)
	{
	case DW_ATE_void:		printf ("(void)"); break;
	case DW_ATE_address:		printf ("(machine address)"); break;
	case DW_ATE_boolean:		printf ("(boolean)"); break;
	case DW_ATE_complex_float:	printf ("(complex float)"); break;
	case DW_ATE_float:		printf ("(float)"); break;
	case DW_ATE_signed:		printf ("(signed)"); break;
	case DW_ATE_signed_char:	printf ("(signed char)"); break;
	case DW_ATE_unsigned:		printf ("(unsigned)"); break;
	case DW_ATE_unsigned_char:	printf ("(unsigned char)"); break;
	  /* DWARF 2.1 value.  */
	case DW_ATE_imaginary_float:	printf ("(imaginary float)"); break;
	default:
	  if (uvalue >= DW_ATE_lo_user
	      && uvalue <= DW_ATE_hi_user)
	    printf ("(user defined type)");
	  else
	    printf ("(unknown type)");
	  break;
	}
      break;

    case DW_AT_accessibility:
      switch (uvalue)
	{
	case DW_ACCESS_public:		printf ("(public)"); break;
	case DW_ACCESS_protected:	printf ("(protected)"); break;
	case DW_ACCESS_private:		printf ("(private)"); break;
	default:
	  printf ("(unknown accessibility)");
	  break;
	}
      break;

    case DW_AT_visibility:
      switch (uvalue)
	{
	case DW_VIS_local:		printf ("(local)"); break;
	case DW_VIS_exported:		printf ("(exported)"); break;
	case DW_VIS_qualified:		printf ("(qualified)"); break;
	default:			printf ("(unknown visibility)"); break;
	}
      break;

    case DW_AT_virtuality:
      switch (uvalue)
	{
	case DW_VIRTUALITY_none:	printf ("(none)"); break;
	case DW_VIRTUALITY_virtual:	printf ("(virtual)"); break;
	case DW_VIRTUALITY_pure_virtual:printf ("(pure_virtual)"); break;
	default:			printf ("(unknown virtuality)"); break;
	}
      break;

    case DW_AT_identifier_case:
      switch (uvalue)
	{
	case DW_ID_case_sensitive:	printf ("(case_sensitive)"); break;
	case DW_ID_up_case:		printf ("(up_case)"); break;
	case DW_ID_down_case:		printf ("(down_case)"); break;
	case DW_ID_case_insensitive:	printf ("(case_insensitive)"); break;
	default:			printf ("(unknown case)"); break;
	}
      break;

    case DW_AT_calling_convention:
      switch (uvalue)
	{
	case DW_CC_normal:	printf ("(normal)"); break;
	case DW_CC_program:	printf ("(program)"); break;
	case DW_CC_nocall:	printf ("(nocall)"); break;
	default:
	  if (uvalue >= DW_CC_lo_user
	      && uvalue <= DW_CC_hi_user)
	    printf ("(user defined)");
	  else
	    printf ("(unknown convention)");
	}
      break;

    case DW_AT_ordering:
      switch (uvalue)
	{
	case -1: printf ("(undefined)"); break;
	case 0:  printf ("(row major)"); break;
	case 1:  printf ("(column major)"); break;
	}
      break;

    case DW_AT_frame_base:
    case DW_AT_location:
    case DW_AT_data_member_location:
    case DW_AT_vtable_elem_location:
    case DW_AT_allocated:
    case DW_AT_associated:
    case DW_AT_data_location:
    case DW_AT_stride:
    case DW_AT_upper_bound:
    case DW_AT_lower_bound:
      if (block_start)
	{
	  printf ("(");
	  decode_location_expression (block_start, pointer_size, uvalue);
	  printf (")");
	}
      else if (form == DW_FORM_data4 || form == DW_FORM_data8)
	{
	  printf ("(");
	  printf ("location list");
	  printf (")");
	}
      break;

    default:
      break;
    }

  return data;
}

static unsigned char *
read_and_display_attr (unsigned long attribute,
		       unsigned long form,
		       unsigned char *data,
		       unsigned long cu_offset,
		       unsigned long pointer_size,
		       unsigned long offset_size,
		       int dwarf_version)
{
  printf ("     %-18s:", get_AT_name (attribute));
  data = read_and_display_attr_value (attribute, form, data, cu_offset,
				      pointer_size, offset_size, dwarf_version);
  printf ("\n");
  return data;
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
        printf (" __UMA__ %s %s %d\n",__FILE__,__func__,__LINE__);
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
				begin + sec->sh_size);

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
	  for (entry = first_abbrev;
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
static unsigned int * debug_line_pointer_sizes = NULL;
static unsigned int   num_debug_line_pointer_sizes = 0;
static unsigned int
get_debug_line_pointer_sizes (int file)
{
  elf64_shdr * section;
  unsigned char *     start;
  unsigned char *     end;
  unsigned char *     begin;
  unsigned long       length;
  unsigned int        num_units;
  unsigned int        unit;

  section = find_section (".debug_info");
  if (section == NULL)
    return 0;

  length = section->sh_size;
  start = get_data (NULL, file, section->sh_offset, section->sh_size,
		    _("extracting pointer sizes from .debug_info section"));
  if (start == NULL)
    return 0;

  end = start + section->sh_size;
  /* First scan the section to get the number of comp units.  */
  for (begin = start, num_units = 0; begin < end; num_units++)
    {
      /* Read the first 4 bytes.  For a 32-bit DWARF section, this will
	 be the length.  For a 64-bit DWARF section, it'll be the escape
	 code 0xffffffff followed by an 8 byte length.  */
      length = byte_get (begin, 4);

      if (length == 0xffffffff)
	{
	  length = byte_get (begin + 4, 8);
	  begin += length + 12;
	}
      else
	begin += length + 4;
    }

  if (num_units == 0)
    {
      error (_("No comp units in .debug_info section ?"));
      free (start);
      return 0;
    }

  /* Then allocate an array to hold the pointer sizes.  */
  debug_line_pointer_sizes = malloc (num_units * sizeof * debug_line_pointer_sizes);
  if (debug_line_pointer_sizes == NULL)
    {
      error (_("Not enough memory for a pointer size array of %u entries"),
	     num_units);
      free (start);
      return 0;
    }

  /* Populate the array.  */
  for (begin = start, unit = 0; begin < end; unit++)
    {
      length = byte_get (begin, 4);
      if (length == 0xffffffff)
	{
	  /* For 64-bit DWARF, the 1-byte address_size field is 22 bytes
	     from the start of the section.  This is computed as follows:

	     unit_length:         12 bytes
	     version:              2 bytes
	     debug_abbrev_offset:  8 bytes
	     -----------------------------
	     Total:               22 bytes  */

	  debug_line_pointer_sizes [unit] = byte_get (begin + 22, 1);
	  length = byte_get (begin + 4, 8);
	  begin += length + 12;
	}
      else
	{
	  /* For 32-bit DWARF, the 1-byte address_size field is 10 bytes from
	     the start of the section:

	     unit_length:          4 bytes
	     version:              2 bytes
	     debug_abbrev_offset:  4 bytes
	     -----------------------------
	     Total:               10 bytes  */

	  debug_line_pointer_sizes [unit] = byte_get (begin + 10, 1);
	  begin += length + 4;
	}
    }

  free (start);
  num_debug_line_pointer_sizes = num_units;
  return num_units;
}
typedef struct State_Machine_Registers
{
  unsigned long address;
  unsigned int file;
  unsigned int line;
  unsigned int column;
  int is_stmt;
  int basic_block;
  int end_sequence;
/* This variable hold the number of the last entry seen
   in the File Table.  */
  unsigned int last_file_entry;
} SMR;
static SMR state_machine_regs;
static void
reset_state_machine (int is_stmt)
{
  state_machine_regs.address = 0;
  state_machine_regs.file = 1;
  state_machine_regs.line = 1;
  state_machine_regs.column = 0;
  state_machine_regs.is_stmt = is_stmt;
  state_machine_regs.basic_block = 0;
  state_machine_regs.end_sequence = 0;
  state_machine_regs.last_file_entry = 0;
}
static int
process_extended_line_op (unsigned char *data, int is_stmt, int pointer_size)
{
  unsigned char op_code;
  int bytes_read;
  unsigned int len;
  unsigned char *name;
  unsigned long adr;

  len = read_leb128 (data, & bytes_read, 0);
  data += bytes_read;

  if (len == 0)
    {
      warn (_("badly formed extended line op encountered!\n"));
      return bytes_read;
    }

  len += bytes_read;
  op_code = *data++;

  printf (_("  Extended opcode %d: "), op_code);

  switch (op_code)
    {
    case DW_LNE_end_sequence:
      printf (_("End of Sequence\n\n"));
      reset_state_machine (is_stmt);
      break;

    case DW_LNE_set_address:
      adr = byte_get (data, pointer_size);
      printf (_("set Address to 0x%lx\n"), adr);
      state_machine_regs.address = adr;
      break;

    case DW_LNE_define_file:
      printf (_("  define new File Table entry\n"));
      printf (_("  Entry\tDir\tTime\tSize\tName\n"));

      printf (_("   %d\t"), ++state_machine_regs.last_file_entry);
      name = data;
      data += strlen ((char *) data) + 1;
      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      data += bytes_read;
      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      data += bytes_read;
      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      printf (_("%s\n\n"), name);
      break;

    default:
      printf (_("UNKNOWN: length %d\n"), len - bytes_read);
      break;
    }

  return len;
}
static int
display_debug_lines (elf64_shdr *section,
		     unsigned char *start, int file)
{
  unsigned char *hdrptr;
  DWARF2_Internal_LineInfo info;
  unsigned char *standard_opcodes;
  unsigned char *data = start;
  unsigned char *end = start + section->sh_size;
  unsigned char *end_of_sequence;
  int i;
  int offset_size;
  int initial_length_size;
  unsigned int comp_unit = 0;

  printf (_("\nDump of debug contents of section %s:\n\n"),
	  SECTION_NAME (section));

  if (num_debug_line_pointer_sizes == 0)
    get_debug_line_pointer_sizes (file);

  while (data < end)
    {
      unsigned int pointer_size;

      hdrptr = data;

      /* Check the length of the block.  */
      info.li_length = byte_get (hdrptr, 4);
      hdrptr += 4;

      if (info.li_length == 0xffffffff)
	{
	  /* This section is 64-bit DWARF 3.  */
	  info.li_length = byte_get (hdrptr, 8);
	  hdrptr += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      if (info.li_length + initial_length_size > section->sh_size)
	{
	  warn
	    (_("The line info appears to be corrupt - the section is too small\n"));
	  return 0;
	}

      /* Check its version number.  */
      info.li_version = byte_get (hdrptr, 2);
      hdrptr += 2;
      if (info.li_version != 2 && info.li_version != 3)
	{
	  warn (_("Only DWARF version 2 and 3 line info is currently supported.\n"));
	  return 0;
	}

      info.li_prologue_length = byte_get (hdrptr, offset_size);
      hdrptr += offset_size;
      info.li_min_insn_length = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_default_is_stmt = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_line_base = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_line_range = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_opcode_base = byte_get (hdrptr, 1);
      hdrptr++;

      /* Sign extend the line base field.  */
      info.li_line_base <<= 24;
      info.li_line_base >>= 24;

      /* Get the pointer size from the comp unit associated
	 with this block of line number information.  */
      if (comp_unit >= num_debug_line_pointer_sizes)
	{
	  error (_("Not enough comp units for .debug_lines section\n"));
	  return 0;
	}
      else
	{
	  pointer_size = debug_line_pointer_sizes [comp_unit];
	  comp_unit ++;
	}

      printf (_("  Length:                      %ld\n"), info.li_length);
      printf (_("  DWARF Version:               %d\n"), info.li_version);
      printf (_("  Prologue Length:             %d\n"), info.li_prologue_length);
      printf (_("  Minimum Instruction Length:  %d\n"), info.li_min_insn_length);
      printf (_("  Initial value of 'is_stmt':  %d\n"), info.li_default_is_stmt);
      printf (_("  Line Base:                   %d\n"), info.li_line_base);
      printf (_("  Line Range:                  %d\n"), info.li_line_range);
      printf (_("  Opcode Base:                 %d\n"), info.li_opcode_base);
      printf (_("  (Pointer size:               %u)\n"), pointer_size);

      end_of_sequence = data + info.li_length + initial_length_size;

      reset_state_machine (info.li_default_is_stmt);

      /* Display the contents of the Opcodes table.  */
      standard_opcodes = hdrptr;

      printf (_("\n Opcodes:\n"));

      for (i = 1; i < info.li_opcode_base; i++)
	printf (_("  Opcode %d has %d args\n"), i, standard_opcodes[i - 1]);

      /* Display the contents of the Directory table.  */
      data = standard_opcodes + info.li_opcode_base - 1;

      if (*data == 0)
	printf (_("\n The Directory Table is empty.\n"));
      else
	{
	  printf (_("\n The Directory Table:\n"));

	  while (*data != 0)
	    {
	      printf (_("  %s\n"), data);

	      data += strlen ((char *) data) + 1;
	    }
	}

      /* Skip the NUL at the end of the table.  */
      data++;

      /* Display the contents of the File Name table.  */
      if (*data == 0)
	printf (_("\n The File Name Table is empty.\n"));
      else
	{
	  printf (_("\n The File Name Table:\n"));
	  printf (_("  Entry\tDir\tTime\tSize\tName\n"));

	  while (*data != 0)
	    {
	      unsigned char *name;
	      int bytes_read;

	      printf (_("  %d\t"), ++state_machine_regs.last_file_entry);
	      name = data;

	      data += strlen ((char *) data) + 1;

	      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%s\n"), name);
	    }
	}

      /* Skip the NUL at the end of the table.  */
      data++;

      /* Now display the statements.  */
      printf (_("\n Line Number Statements:\n"));


      while (data < end_of_sequence)
	{
	  unsigned char op_code;
	  int adv;
	  int bytes_read;

	  op_code = *data++;

	  if (op_code >= info.li_opcode_base)
	    {
	      op_code -= info.li_opcode_base;
	      adv      = (op_code / info.li_line_range) * info.li_min_insn_length;
	      state_machine_regs.address += adv;
	      printf (_("  Special opcode %d: advance Address by %d to 0x%lx"),
		      op_code, adv, state_machine_regs.address);
	      adv = (op_code % info.li_line_range) + info.li_line_base;
	      state_machine_regs.line += adv;
	      printf (_(" and Line by %d to %d\n"),
		      adv, state_machine_regs.line);
	    }
	  else switch (op_code)
	    {
	    case DW_LNS_extended_op:
	      data += process_extended_line_op (data, info.li_default_is_stmt,
						pointer_size);
	      break;

	    case DW_LNS_copy:
	      printf (_("  Copy\n"));
	      break;

	    case DW_LNS_advance_pc:
	      adv = info.li_min_insn_length * read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      state_machine_regs.address += adv;
	      printf (_("  Advance PC by %d to %lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_advance_line:
	      adv = read_leb128 (data, & bytes_read, 1);
	      data += bytes_read;
	      state_machine_regs.line += adv;
	      printf (_("  Advance Line by %d to %d\n"), adv,
		      state_machine_regs.line);
	      break;

	    case DW_LNS_set_file:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      printf (_("  Set File Name to entry %d in the File Name Table\n"),
		      adv);
	      state_machine_regs.file = adv;
	      break;

	    case DW_LNS_set_column:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      printf (_("  Set column to %d\n"), adv);
	      state_machine_regs.column = adv;
	      break;

	    case DW_LNS_negate_stmt:
	      adv = state_machine_regs.is_stmt;
	      adv = ! adv;
	      printf (_("  Set is_stmt to %d\n"), adv);
	      state_machine_regs.is_stmt = adv;
	      break;

	    case DW_LNS_set_basic_block:
	      printf (_("  Set basic block\n"));
	      state_machine_regs.basic_block = 1;
	      break;

	    case DW_LNS_const_add_pc:
	      adv = (((255 - info.li_opcode_base) / info.li_line_range)
		     * info.li_min_insn_length);
	      state_machine_regs.address += adv;
	      printf (_("  Advance PC by constant %d to 0x%lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_fixed_advance_pc:
	      adv = byte_get (data, 2);
	      data += 2;
	      state_machine_regs.address += adv;
	      printf (_("  Advance PC by fixed size amount %d to 0x%lx\n"),
		      adv, state_machine_regs.address);
	      break;

	    case DW_LNS_set_prologue_end:
	      printf (_("  Set prologue_end to true\n"));
	      break;

	    case DW_LNS_set_epilogue_begin:
	      printf (_("  Set epilogue_begin to true\n"));
	      break;

	    case DW_LNS_set_isa:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      printf (_("  Set ISA to %d\n"), adv);
	      break;

	    default:
	      printf (_("  Unknown opcode %d with operands: "), op_code);
	      {
		int i;
		for (i = standard_opcodes[op_code - 1]; i > 0 ; --i)
		  {
		    printf ("0x%lx%s", read_leb128 (data, &bytes_read, 0),
			    i == 1 ? "" : ", ");
		    data += bytes_read;
		  }
		putchar ('\n');
	      }
	      break;
	    }
	}
      putchar ('\n');
    }

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
#if 1
typedef struct Frame_Chunk
{
  struct Frame_Chunk *next;
  unsigned char *chunk_start;
  int ncols;
  /* DW_CFA_{undefined,same_value,offset,register,unreferenced}  */
  short int *col_type;
  int *col_offset;
  char *augmentation;
  unsigned int code_factor;
  int data_factor;
  unsigned long pc_begin;
  unsigned long pc_range;
  int cfa_reg;
  int cfa_offset;
  int ra;
  unsigned char fde_encoding;
  unsigned char cfa_exp;
}
Frame_Chunk;

/* A marker for a col_type that means this column was never referenced
   in the frame info.  */
#define DW_CFA_unreferenced (-1)

static void
frame_need_space (Frame_Chunk *fc, int reg)
{
  int prev = fc->ncols;

  if (reg < fc->ncols)
    return;

  fc->ncols = reg + 1;
  fc->col_type = xrealloc (fc->col_type, fc->ncols * sizeof (short int));
  fc->col_offset = xrealloc (fc->col_offset, fc->ncols * sizeof (int));

  while (prev < fc->ncols)
    {
      fc->col_type[prev] = DW_CFA_unreferenced;
      fc->col_offset[prev] = 0;
      prev++;
    }
}

static void
frame_display_row (Frame_Chunk *fc, int *need_col_headers, int *max_regs)
{
  int r;
  char tmp[100];

  if (*max_regs < fc->ncols)
    *max_regs = fc->ncols;

  if (*need_col_headers)
    {
      *need_col_headers = 0;

      printf ("   LOC   CFA      ");

      for (r = 0; r < *max_regs; r++)
	if (fc->col_type[r] != DW_CFA_unreferenced)
	  {
	    if (r == fc->ra)
	      printf ("ra   ");
	    else
	      printf ("r%-4d", r);
	  }

      printf ("\n");
    }

  printf ("%08lx ", fc->pc_begin);
  if (fc->cfa_exp)
    strcpy (tmp, "exp");
  else
    sprintf (tmp, "r%d%+d", fc->cfa_reg, fc->cfa_offset);
  printf ("%-8s ", tmp);

  for (r = 0; r < fc->ncols; r++)
    {
      if (fc->col_type[r] != DW_CFA_unreferenced)
	{
	  switch (fc->col_type[r])
	    {
	    case DW_CFA_undefined:
	      strcpy (tmp, "u");
	      break;
	    case DW_CFA_same_value:
	      strcpy (tmp, "s");
	      break;
	    case DW_CFA_offset:
	      sprintf (tmp, "c%+d", fc->col_offset[r]);
	      break;
	    case DW_CFA_register:
	      sprintf (tmp, "r%d", fc->col_offset[r]);
	      break;
	    case DW_CFA_expression:
	      strcpy (tmp, "exp");
	      break;
	    default:
	      strcpy (tmp, "n/a");
	      break;
	    }
	  printf ("%-5s", tmp);
	}
    }
  printf ("\n");
}

static int
size_of_encoded_value (int encoding)
{
  switch (encoding & 0x7)
    {
    default:	/* ??? */
    case 0:	return 0 ? 4 : 8;
    case 2:	return 2;
    case 3:	return 4;
    case 4:	return 8;
    }
}

static size_t
byte_get_signed (unsigned char *field, int size)
{
  size_t x = byte_get (field, size);

  switch (size)
    {
    case 1:
      return (x ^ 0x80) - 0x80;
    case 2:
      return (x ^ 0x8000) - 0x8000;
    case 4:
      return (x ^ 0x80000000) - 0x80000000;
    case 8:
    case -8:
      return x;
    default:
      abort ();
    }
}
static size_t
get_encoded_value (unsigned char *data, int encoding)
{
  int size = size_of_encoded_value (encoding);
  if (encoding & DW_EH_PE_signed)
    return byte_get_signed (data, size);
  else
    return byte_get (data, size);
}

#define GET(N)	byte_get (start, N); start += N
#define LEB()	read_leb128 (start, & length_return, 0); start += length_return
#define SLEB()	read_leb128 (start, & length_return, 1); start += length_return

static int
display_debug_frames (elf64_shdr *section,
		      unsigned char *start,
		      int file )
{
  unsigned char *end = start + section->sh_size;
  unsigned char *section_start = start;
  Frame_Chunk *chunks = 0;
  Frame_Chunk *remembered_state = 0;
  Frame_Chunk *rs;
  int is_eh = (strcmp (SECTION_NAME (section), ".eh_frame") == 0);
  int length_return;
  int max_regs = 0;
  int addr_size = 0 ? 4 : 8;

  printf (_("The section %s contains:\n"), SECTION_NAME (section));

  while (start < end)
    {
      unsigned char *saved_start;
      unsigned char *block_end;
      unsigned long length;
      unsigned long cie_id;
      Frame_Chunk *fc;
      Frame_Chunk *cie;
      int need_col_headers = 1;
      unsigned char *augmentation_data = NULL;
      unsigned long augmentation_data_len = 0;
      int encoded_ptr_size = addr_size;
      int offset_size;
      int initial_length_size;

      saved_start = start;
      length = byte_get (start, 4); start += 4;

      if (length == 0)
	{
	  printf ("\n%08lx ZERO terminator\n\n",
		    (unsigned long)(saved_start - section_start));
	  return 1;
	}

      if (length == 0xffffffff)
	{
	  length = byte_get (start, 8);
	  start += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      block_end = saved_start + length + initial_length_size;
      cie_id = byte_get (start, offset_size); start += offset_size;

      if (is_eh ? (cie_id == 0) : (cie_id == DW_CIE_ID))
	{
	  int version;

	  fc = xmalloc (sizeof (Frame_Chunk));
	  memset (fc, 0, sizeof (Frame_Chunk));

	  fc->next = chunks;
	  chunks = fc;
	  fc->chunk_start = saved_start;
	  fc->ncols = 0;
	  fc->col_type = xmalloc (sizeof (short int));
	  fc->col_offset = xmalloc (sizeof (int));
	  frame_need_space (fc, max_regs-1);

	  version = *start++;

	  fc->augmentation = start;
	  start = strchr (start, '\0') + 1;

	  if (fc->augmentation[0] == 'z')
	    {
	      fc->code_factor = LEB ();
	      fc->data_factor = SLEB ();
	      fc->ra = byte_get (start, 1); start += 1;
	      augmentation_data_len = LEB ();
	      augmentation_data = start;
	      start += augmentation_data_len;
	    }
	  else if (strcmp (fc->augmentation, "eh") == 0)
	    {
	      start += addr_size;
	      fc->code_factor = LEB ();
	      fc->data_factor = SLEB ();
	      fc->ra = byte_get (start, 1); start += 1;
	    }
	  else
	    {
	      fc->code_factor = LEB ();
	      fc->data_factor = SLEB ();
	      fc->ra = byte_get (start, 1); start += 1;
	    }
	  cie = fc;

	  if (do_debug_frames_interp)
	    printf ("\n%08lx %08lx %08lx CIE \"%s\" cf=%d df=%d ra=%d\n",
		    (unsigned long)(saved_start - section_start), length, cie_id,
		    fc->augmentation, fc->code_factor, fc->data_factor,
		    fc->ra);
	  else
	    {
	      printf ("\n%08lx %08lx %08lx CIE\n",
		      (unsigned long)(saved_start - section_start), length, cie_id);
	      printf ("  Version:               %d\n", version);
	      printf ("  Augmentation:          \"%s\"\n", fc->augmentation);
	      printf ("  Code alignment factor: %u\n", fc->code_factor);
	      printf ("  Data alignment factor: %d\n", fc->data_factor);
	      printf ("  Return address column: %d\n", fc->ra);

	      if (augmentation_data_len)
		{
		  unsigned long i;
		  printf ("  Augmentation data:    ");
		  for (i = 0; i < augmentation_data_len; ++i)
		    printf (" %02x", augmentation_data[i]);
		  putchar ('\n');
		}
	      putchar ('\n');
	    }

	  if (augmentation_data_len)
	    {
	      unsigned char *p, *q;
	      p = fc->augmentation + 1;
	      q = augmentation_data;

	      while (1)
		{
		  if (*p == 'L')
		    q++;
		  else if (*p == 'P')
		    q += 1 + size_of_encoded_value (*q);
		  else if (*p == 'R')
		    fc->fde_encoding = *q++;
		  else
		    break;
		  p++;
		}

	      if (fc->fde_encoding)
		encoded_ptr_size = size_of_encoded_value (fc->fde_encoding);
	    }

	  frame_need_space (fc, fc->ra);
	}
      else
	{
	  unsigned char *look_for;
	  static Frame_Chunk fde_fc;

	  fc = & fde_fc;
	  memset (fc, 0, sizeof (Frame_Chunk));

	  look_for = is_eh ? start - 4 - cie_id : section_start + cie_id;

	  for (cie = chunks; cie ; cie = cie->next)
	    if (cie->chunk_start == look_for)
	      break;

	  if (!cie)
	    {
	      start = block_end;
	      fc->ncols = 0;
	      fc->col_type = xmalloc (sizeof (short int));
	      fc->col_offset = xmalloc (sizeof (int));
	      frame_need_space (fc, max_regs - 1);
	      cie = fc;
	      fc->augmentation = "";
	      fc->fde_encoding = 0;
	    }
	  else
	    {
	      fc->ncols = cie->ncols;
	      fc->col_type = xmalloc (fc->ncols * sizeof (short int));
	      fc->col_offset = xmalloc (fc->ncols * sizeof (int));
	      memcpy (fc->col_type, cie->col_type, fc->ncols * sizeof (short int));
	      memcpy (fc->col_offset, cie->col_offset, fc->ncols * sizeof (int));
	      fc->augmentation = cie->augmentation;
	      fc->code_factor = cie->code_factor;
	      fc->data_factor = cie->data_factor;
	      fc->cfa_reg = cie->cfa_reg;
	      fc->cfa_offset = cie->cfa_offset;
	      fc->ra = cie->ra;
	      frame_need_space (fc, max_regs-1);
	      fc->fde_encoding = cie->fde_encoding;
	    }

	  if (fc->fde_encoding)
	    encoded_ptr_size = size_of_encoded_value (fc->fde_encoding);

	  fc->pc_begin = get_encoded_value (start, fc->fde_encoding);
	  if ((fc->fde_encoding & 0x70) == DW_EH_PE_pcrel)
	    fc->pc_begin += section->sh_addr + (start - section_start);
	  start += encoded_ptr_size;
	  fc->pc_range = byte_get (start, encoded_ptr_size);
	  start += encoded_ptr_size;

	  if (cie->augmentation[0] == 'z')
	    {
	      augmentation_data_len = LEB ();
	      augmentation_data = start;
	      start += augmentation_data_len;
	    }

	  printf ("\n%08lx %08lx %08lx FDE cie=%08lx pc=%08lx..%08lx\n",
		  (unsigned long)(saved_start - section_start), length, cie_id,
		  (unsigned long)(cie->chunk_start - section_start),
		  fc->pc_begin, fc->pc_begin + fc->pc_range);
	  if (! do_debug_frames_interp && augmentation_data_len)
	    {
	      unsigned long i;
	      printf ("  Augmentation data:    ");
	      for (i = 0; i < augmentation_data_len; ++i)
	        printf (" %02x", augmentation_data[i]);
	      putchar ('\n');
	      putchar ('\n');
	    }
	}

      /* At this point, fc is the current chunk, cie (if any) is set, and we're
	 about to interpret instructions for the chunk.  */
      /* ??? At present we need to do this always, since this sizes the
	 fc->col_type and fc->col_offset arrays, which we write into always.
	 We should probably split the interpreted and non-interpreted bits
	 into two different routines, since there's so much that doesn't
	 really overlap between them.  */
      if (1 || do_debug_frames_interp)
	{
	  /* Start by making a pass over the chunk, allocating storage
	     and taking note of what registers are used.  */
	  unsigned char *tmp = start;

	  while (start < block_end)
	    {
	      unsigned op, opa;
	      unsigned long reg, tmp;

	      op = *start++;
	      opa = op & 0x3f;
	      if (op & 0xc0)
		op &= 0xc0;

	      /* Warning: if you add any more cases to this switch, be
	         sure to add them to the corresponding switch below.  */
	      switch (op)
		{
		case DW_CFA_advance_loc:
		  break;
		case DW_CFA_offset:
		  LEB ();
		  frame_need_space (fc, opa);
		  fc->col_type[opa] = DW_CFA_undefined;
		  break;
		case DW_CFA_restore:
		  frame_need_space (fc, opa);
		  fc->col_type[opa] = DW_CFA_undefined;
		  break;
		case DW_CFA_set_loc:
		  start += encoded_ptr_size;
		  break;
		case DW_CFA_advance_loc1:
		  start += 1;
		  break;
		case DW_CFA_advance_loc2:
		  start += 2;
		  break;
		case DW_CFA_advance_loc4:
		  start += 4;
		  break;
		case DW_CFA_offset_extended:
		  reg = LEB (); LEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_restore_extended:
		  reg = LEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_undefined:
		  reg = LEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_same_value:
		  reg = LEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_register:
		  reg = LEB (); LEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_def_cfa:
		  LEB (); LEB ();
		  break;
		case DW_CFA_def_cfa_register:
		  LEB ();
		  break;
		case DW_CFA_def_cfa_offset:
		  LEB ();
		  break;
		case DW_CFA_def_cfa_expression:
		  tmp = LEB ();
		  start += tmp;
		  break;
		case DW_CFA_expression:
		  reg = LEB ();
		  tmp = LEB ();
		  start += tmp;
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_offset_extended_sf:
		  reg = LEB (); SLEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_def_cfa_sf:
		  LEB (); SLEB ();
		  break;
		case DW_CFA_def_cfa_offset_sf:
		  SLEB ();
		  break;
		case DW_CFA_MIPS_advance_loc8:
		  start += 8;
		  break;
		case DW_CFA_GNU_args_size:
		  LEB ();
		  break;
		case DW_CFA_GNU_negative_offset_extended:
		  reg = LEB (); LEB ();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;

		default:
		  break;
		}
	    }
	  start = tmp;
	}

      /* Now we know what registers are used, make a second pass over
         the chunk, this time actually printing out the info.  */

      while (start < block_end)
	{
	  unsigned op, opa;
	  unsigned long ul, reg, roffs;
	  long l, ofs;
	  size_t vma;

	  op = *start++;
	  opa = op & 0x3f;
	  if (op & 0xc0)
	    op &= 0xc0;

	  /* Warning: if you add any more cases to this switch, be
	     sure to add them to the corresponding switch above.  */
	  switch (op)
	    {
	    case DW_CFA_advance_loc:
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc: %d to %08lx\n",
			opa * fc->code_factor,
			fc->pc_begin + opa * fc->code_factor);
	      fc->pc_begin += opa * fc->code_factor;
	      break;

	    case DW_CFA_offset:
	      roffs = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_offset: r%d at cfa%+ld\n",
			opa, roffs * fc->data_factor);
	      fc->col_type[opa] = DW_CFA_offset;
	      fc->col_offset[opa] = roffs * fc->data_factor;
	      break;

	    case DW_CFA_restore:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_restore: r%d\n", opa);
	      fc->col_type[opa] = cie->col_type[opa];
	      fc->col_offset[opa] = cie->col_offset[opa];
	      break;

	    case DW_CFA_set_loc:
	      vma = get_encoded_value (start, fc->fde_encoding);
	      if ((fc->fde_encoding & 0x70) == DW_EH_PE_pcrel)
		vma += section->sh_addr + (start - section_start);
	      start += encoded_ptr_size;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_set_loc: %08lx\n", (unsigned long)vma);
	      fc->pc_begin = vma;
	      break;

	    case DW_CFA_advance_loc1:
	      ofs = byte_get (start, 1); start += 1;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc1: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_advance_loc2:
	      ofs = byte_get (start, 2); start += 2;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc2: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_advance_loc4:
	      ofs = byte_get (start, 4); start += 4;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc4: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_offset_extended:
	      reg = LEB ();
	      roffs = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_offset_extended: r%ld at cfa%+ld\n",
			reg, roffs * fc->data_factor);
	      fc->col_type[reg] = DW_CFA_offset;
	      fc->col_offset[reg] = roffs * fc->data_factor;
	      break;

	    case DW_CFA_restore_extended:
	      reg = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_restore_extended: r%ld\n", reg);
	      fc->col_type[reg] = cie->col_type[reg];
	      fc->col_offset[reg] = cie->col_offset[reg];
	      break;

	    case DW_CFA_undefined:
	      reg = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_undefined: r%ld\n", reg);
	      fc->col_type[reg] = DW_CFA_undefined;
	      fc->col_offset[reg] = 0;
	      break;

	    case DW_CFA_same_value:
	      reg = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_same_value: r%ld\n", reg);
	      fc->col_type[reg] = DW_CFA_same_value;
	      fc->col_offset[reg] = 0;
	      break;

	    case DW_CFA_register:
	      reg = LEB ();
	      roffs = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_register: r%ld in r%ld\n", reg, roffs);
	      fc->col_type[reg] = DW_CFA_register;
	      fc->col_offset[reg] = roffs;
	      break;

	    case DW_CFA_remember_state:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_remember_state\n");
	      rs = xmalloc (sizeof (Frame_Chunk));
	      rs->ncols = fc->ncols;
	      rs->col_type = xmalloc (rs->ncols * sizeof (short int));
	      rs->col_offset = xmalloc (rs->ncols * sizeof (int));
	      memcpy (rs->col_type, fc->col_type, rs->ncols);
	      memcpy (rs->col_offset, fc->col_offset, rs->ncols * sizeof (int));
	      rs->next = remembered_state;
	      remembered_state = rs;
	      break;

	    case DW_CFA_restore_state:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_restore_state\n");
	      rs = remembered_state;
	      if (rs)
		{
		  remembered_state = rs->next;
		  frame_need_space (fc, rs->ncols-1);
		  memcpy (fc->col_type, rs->col_type, rs->ncols);
		  memcpy (fc->col_offset, rs->col_offset,
			  rs->ncols * sizeof (int));
		  free (rs->col_type);
		  free (rs->col_offset);
		  free (rs);
		}
	      else if (do_debug_frames_interp)
		printf ("Mismatched DW_CFA_restore_state\n");
	      break;

	    case DW_CFA_def_cfa:
	      fc->cfa_reg = LEB ();
	      fc->cfa_offset = LEB ();
	      fc->cfa_exp = 0;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa: r%d ofs %d\n",
			fc->cfa_reg, fc->cfa_offset);
	      break;

	    case DW_CFA_def_cfa_register:
	      fc->cfa_reg = LEB ();
	      fc->cfa_exp = 0;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_reg: r%d\n", fc->cfa_reg);
	      break;

	    case DW_CFA_def_cfa_offset:
	      fc->cfa_offset = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_offset: %d\n", fc->cfa_offset);
	      break;

	    case DW_CFA_nop:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_nop\n");
	      break;

	    case DW_CFA_def_cfa_expression:
	      ul = LEB ();
	      if (! do_debug_frames_interp)
		{
		  printf ("  DW_CFA_def_cfa_expression (");
		  decode_location_expression (start, addr_size, ul);
		  printf (")\n");
		}
	      fc->cfa_exp = 1;
	      start += ul;
	      break;

	    case DW_CFA_expression:
	      reg = LEB ();
	      ul = LEB ();
	      if (! do_debug_frames_interp)
		{
		  printf ("  DW_CFA_expression: r%ld (", reg);
		  decode_location_expression (start, addr_size, ul);
		  printf (")\n");
		}
	      fc->col_type[reg] = DW_CFA_expression;
	      start += ul;
	      break;

	    case DW_CFA_offset_extended_sf:
	      reg = LEB ();
	      l = SLEB ();
	      frame_need_space (fc, reg);
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_offset_extended_sf: r%ld at cfa%+ld\n",
			reg, l * fc->data_factor);
	      fc->col_type[reg] = DW_CFA_offset;
	      fc->col_offset[reg] = l * fc->data_factor;
	      break;

	    case DW_CFA_def_cfa_sf:
	      fc->cfa_reg = LEB ();
	      fc->cfa_offset = SLEB ();
	      fc->cfa_exp = 0;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_sf: r%d ofs %d\n",
			fc->cfa_reg, fc->cfa_offset);
	      break;

	    case DW_CFA_def_cfa_offset_sf:
	      fc->cfa_offset = SLEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_offset_sf: %d\n", fc->cfa_offset);
	      break;

	    case DW_CFA_MIPS_advance_loc8:
	      ofs = byte_get (start, 8); start += 8;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_MIPS_advance_loc8: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_GNU_window_save:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_GNU_window_save\n");
	      break;

	    case DW_CFA_GNU_args_size:
	      ul = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_GNU_args_size: %ld\n", ul);
	      break;

	    case DW_CFA_GNU_negative_offset_extended:
	      reg = LEB ();
	      l = - LEB ();
	      frame_need_space (fc, reg);
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_GNU_negative_offset_extended: r%ld at cfa%+ld\n",
			reg, l * fc->data_factor);
	      fc->col_type[reg] = DW_CFA_offset;
	      fc->col_offset[reg] = l * fc->data_factor;
	      break;

	    default:
	      fprintf (stderr, "unsupported or unknown DW_CFA_%d\n", op);
	      start = block_end;
	    }
	}

      if (do_debug_frames_interp)
	frame_display_row (fc, &need_col_headers, &max_regs);

      start = block_end;
    }

  printf ("\n");

  return 1;
}
#endif


struct
{
    const char *const name;
    int (*display) (elf64_shdr *, unsigned char *,int);
}
debug_displays[] =
{
    { ".debug_line",		display_debug_lines },
#if 0
    { ".debug_info",		display_debug_info },
    { ".debug_abbrev",		display_debug_abbrev },
    { ".debug_aranges",		display_debug_aranges },
    { ".debug_frame",		display_debug_frames },
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
        debug (" [%3d] = [%16s]\n", k, string_table + section_header.sh_name);
        if (0 == strncmp(name, ".debug_", 7)) {
            save_offset(fd, &off);
            display_debug_section(&section_header, fd);
            restore_offset(fd, off);
        }
        memset(&section_header, 0, sizeof(section_header));
    }
}

int main (int argc, char *argv[])
{
    int fd = -1;


    fd = open("./for-dwarf", O_RDONLY);
    exit_on_error(fd == -1);

    read_elf_header(fd);
    read_str_table(fd);
    read_section_headers(fd);
    dump_section_headers(fd);

        return 0;
}

