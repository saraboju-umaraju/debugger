#include "debug.h"

extern unsigned long string_table_length;
extern char *string_table ;
int do_debug_frames_interp;

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
