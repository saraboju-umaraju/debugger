#include "debug.h"
#define PRINTF(...) __VA_ARGS__
//#define PRINTF(...) printf(__VA_ARGS__)
extern unsigned long string_table_length;
extern char *string_table ;
static unsigned int * debug_line_pointer_sizes = NULL;
static unsigned int   num_debug_line_pointer_sizes = 0;
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

void dump_state_machine(char *name)
{
        //printf("%10s 0x%lx [%3d, %3d] %3s %3s\n", name ? name : "null", state_machine_regs.address, state_machine_regs.line, state_machine_regs.column, state_machine_regs.is_stmt ? "NS" : "", state_machine_regs.end_sequence ? "ET" : "");

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

  PRINTF (_("  Extended opcode %d: "), op_code);

  switch (op_code)
    {
    case DW_LNE_end_sequence:
      PRINTF (_("End of Sequence\n\n"));
      state_machine_regs.end_sequence = 1;
      dump_state_machine(NULL);
      reset_state_machine (is_stmt);
      break;

    case DW_LNE_set_address:
      adr = byte_get (data, pointer_size);
      PRINTF (_("set Address to 0x%lx\n"), adr);
      state_machine_regs.address = adr;
      break;

    case DW_LNE_define_file:
      PRINTF (_("  define new File Table entry\n"));
      PRINTF (_("  Entry\tDir\tTime\tSize\tName\n"));

      PRINTF (_("   %d\t"), ++state_machine_regs.last_file_entry);
      name = data;
      data += strlen ((char *) data) + 1;
      PRINTF (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      data += bytes_read;
      PRINTF (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      data += bytes_read;
      PRINTF (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      PRINTF (_("-%s\n\n"), name);
      break;

    default:
      PRINTF (_("UNKNOWN: length %d\n"), len - bytes_read);
      break;
    }

  return len;
}
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
#if 0
static char *do_this(char *data, DWARF2_Internal_LineInfo *info)
{
      while (data < end_of_sequence)
	{
	  unsigned char op_code;
	  int adv;
	  int bytes_read;

	  op_code = *data++;

	  if (op_code >= info->li_opcode_base)
	    {
	      op_code -= info->li_opcode_base;
	      adv      = (op_code / info->li_line_range) * info->li_min_insn_length;
	      state_machine_regs.address += adv;
	      PRINTF (_("  Special opcode %d: advance Address by %d to 0x%lx"),
		      op_code, adv, state_machine_regs.address);
	      adv = (op_code % info->li_line_range) + info->li_line_base;
	      state_machine_regs.line += adv;
	      PRINTF (_(" and Line by %d to %d\n"),
		      adv, state_machine_regs.line);
          dump_state_machine(NULL);
	    }
	  else switch (op_code)
	    {
	    case DW_LNS_extended_op:
	      data += process_extended_line_op (data, info->li_default_is_stmt,
						pointer_size);
	      break;

	    case DW_LNS_copy:
	      PRINTF (_("  Copy\n"));
	      break;

	    case DW_LNS_advance_pc:
	      adv = info->li_min_insn_length * read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      state_machine_regs.address += adv;
	      PRINTF (_("  Advance PC by %d to %lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_advance_line:
	      adv = read_leb128 (data, & bytes_read, 1);
	      data += bytes_read;
	      state_machine_regs.line += adv;
	      PRINTF (_("  Advance Line by %d to %d\n"), adv,
		      state_machine_regs.line);
	      break;

	    case DW_LNS_set_file:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      PRINTF (_("  Set File Name to entry %d in the File Name Table\n"),
		      adv);
	      state_machine_regs.file = adv;
	      break;

	    case DW_LNS_set_column:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      PRINTF (_("  Set column to %d\n"), adv);
	      state_machine_regs.column = adv;
	      break;

	    case DW_LNS_negate_stmt:
	      adv = state_machine_regs.is_stmt;
	      adv = ! adv;
	      PRINTF (_("  Set is_stmt to %d\n"), adv);
	      state_machine_regs.is_stmt = adv;
	      break;

	    case DW_LNS_set_basic_block:
	      PRINTF (_("  Set basic block\n"));
	      state_machine_regs.basic_block = 1;
	      break;

	    case DW_LNS_const_add_pc:
	      adv = (((255 - info->li_opcode_base) / info->li_line_range)
		     * info->li_min_insn_length);
	      state_machine_regs.address += adv;
	      PRINTF (_("  Advance PC by constant %d to 0x%lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_fixed_advance_pc:
	      adv = byte_get (data, 2);
	      data += 2;
	      state_machine_regs.address += adv;
	      PRINTF (_("  Advance PC by fixed size amount %d to 0x%lx\n"),
		      adv, state_machine_regs.address);
	      break;

	    case DW_LNS_set_prologue_end:
	      PRINTF (_("  Set prologue_end to true\n"));
	      break;

	    case DW_LNS_set_epilogue_begin:
	      PRINTF (_("  Set epilogue_begin to true\n"));
	      break;

	    case DW_LNS_set_isa:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      PRINTF (_("  Set ISA to %d\n"), adv);
	      break;

	    default:
	      PRINTF (_("  Unknown opcode %d with operands: "), op_code);
	      {
		int i;
		for (i = standard_opcodes[op_code - 1]; i > 0 ; --i)
		  {
		    PRINTF ("0x%lx%s", read_leb128 (data, &bytes_read, 0),
			    i == 1 ? "" : ", ");
		    data += bytes_read;
		  }
		putchar ('\n');
	      }
	      break;
	    }
	}
}
#endif
int
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

  PRINTF (_("\nDump of debug contents of section %s:\n\n"),
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

      PRINTF (_("  Length:                      %ld\n"), info.li_length);
      PRINTF (_("  DWARF Version:               %d\n"), info.li_version);
      PRINTF (_("  Prologue Length:             %d\n"), info.li_prologue_length);
      PRINTF (_("  Minimum Instruction Length:  %d\n"), info.li_min_insn_length);
      PRINTF (_("  Initial value of 'is_stmt':  %d\n"), info.li_default_is_stmt);
      PRINTF (_("  Line Base:                   %d\n"), info.li_line_base);
      PRINTF (_("  Line Range:                  %d\n"), info.li_line_range);
      PRINTF (_("  Opcode Base:                 %d\n"), info.li_opcode_base);
      PRINTF (_("  (Pointer size:               %u)\n"), pointer_size);

      end_of_sequence = data + info.li_length + initial_length_size;

      reset_state_machine (info.li_default_is_stmt);

      /* Display the contents of the Opcodes table.  */
      standard_opcodes = hdrptr;

      PRINTF (_("\n Opcodes:\n"));

      for (i = 1; i < info.li_opcode_base; i++)
	PRINTF (_("  Opcode %d has %d args\n"), i, standard_opcodes[i - 1]);

      /* Display the contents of the Directory table.  */
      data = standard_opcodes + info.li_opcode_base - 1;

      if (*data == 0)
	PRINTF (_("\n The Directory Table is empty.\n"));
      else
	{
	  PRINTF (_("\n The Directory Table:\n"));

	  while (*data != 0)
	    {
	      PRINTF (_("  %s\n"), data);

	      data += strlen ((char *) data) + 1;
	    }
	}

      /* Skip the NUL at the end of the table.  */
      data++;

      /* Display the contents of the File Name table.  */
      if (*data == 0)
	PRINTF (_("\n The File Name Table is empty.\n"));
      else
	{
	  PRINTF (_("\n The File Name Table:\n"));
	  PRINTF (_("  Entry\tDir\tTime\tSize\tName\n"));

	  while (*data != 0)
	    {
            unsigned char *name;
	      int bytes_read;

	      PRINTF (_("  %d\t"), ++state_machine_regs.last_file_entry);
	      name = data;

	      data += strlen ((char *) data) + 1;

	      PRINTF (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      PRINTF (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      PRINTF (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%s\n"), name);
	    }
	}

      /* Skip the NUL at the end of the table.  */
      data++;

      /* Now display the statements.  */
      PRINTF (_("\n Line Number Statements:\n"));


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
	      PRINTF (_("  Special opcode %d: advance Address by %d to 0x%lx"),
		      op_code, adv, state_machine_regs.address);
	      adv = (op_code % info.li_line_range) + info.li_line_base;
	      state_machine_regs.line += adv;
	      PRINTF (_(" and Line by %d to %d\n"),
		      adv, state_machine_regs.line);
          dump_state_machine(NULL);
	    }
	  else switch (op_code)
	    {
	    case DW_LNS_extended_op:
	      data += process_extended_line_op (data, info.li_default_is_stmt,
						pointer_size);
	      break;

	    case DW_LNS_copy:
	      PRINTF (_("  Copy\n"));
	      break;

	    case DW_LNS_advance_pc:
	      adv = info.li_min_insn_length * read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      state_machine_regs.address += adv;
	      PRINTF (_("  Advance PC by %d to %lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_advance_line:
	      adv = read_leb128 (data, & bytes_read, 1);
	      data += bytes_read;
	      state_machine_regs.line += adv;
	      PRINTF (_("  Advance Line by %d to %d\n"), adv,
		      state_machine_regs.line);
	      break;

	    case DW_LNS_set_file:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      PRINTF (_("  Set File Name to entry %d in the File Name Table\n"),
		      adv);
	      state_machine_regs.file = adv;
	      break;

	    case DW_LNS_set_column:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      PRINTF (_("  Set column to %d\n"), adv);
	      state_machine_regs.column = adv;
	      break;

	    case DW_LNS_negate_stmt:
	      adv = state_machine_regs.is_stmt;
	      adv = ! adv;
	      PRINTF (_("  Set is_stmt to %d\n"), adv);
	      state_machine_regs.is_stmt = adv;
	      break;

	    case DW_LNS_set_basic_block:
	      PRINTF (_("  Set basic block\n"));
	      state_machine_regs.basic_block = 1;
	      break;

	    case DW_LNS_const_add_pc:
	      adv = (((255 - info.li_opcode_base) / info.li_line_range)
		     * info.li_min_insn_length);
	      state_machine_regs.address += adv;
	      PRINTF (_("  Advance PC by constant %d to 0x%lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_fixed_advance_pc:
	      adv = byte_get (data, 2);
	      data += 2;
	      state_machine_regs.address += adv;
	      PRINTF (_("  Advance PC by fixed size amount %d to 0x%lx\n"),
		      adv, state_machine_regs.address);
	      break;

	    case DW_LNS_set_prologue_end:
	      PRINTF (_("  Set prologue_end to true\n"));
	      break;

	    case DW_LNS_set_epilogue_begin:
	      PRINTF (_("  Set epilogue_begin to true\n"));
	      break;

	    case DW_LNS_set_isa:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      PRINTF (_("  Set ISA to %d\n"), adv);
	      break;

	    default:
	      PRINTF (_("  Unknown opcode %d with operands: "), op_code);
	      {
		int i;
		for (i = standard_opcodes[op_code - 1]; i > 0 ; --i)
		  {
		    PRINTF ("0x%lx%s", read_leb128 (data, &bytes_read, 0),
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

