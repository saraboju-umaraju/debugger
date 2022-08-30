//#define EI_NIDENT	16
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long __s64;
typedef unsigned long __u64;

/* 32-bit ELF base types. */
typedef __u32   Elf32_Addr;
typedef __u16   Elf32_Half;
typedef __u32   Elf32_Off;
typedef __s32   Elf32_Sword;
typedef __u32   Elf32_Word;

/* 64-bit ELF base types. */
typedef __u64   Elf64_Addr;
typedef __u16   Elf64_Half;
typedef __s16   Elf64_SHalf;
typedef __u64   Elf64_Off;
typedef __s32   Elf64_Sword;
typedef __u32   Elf64_Word;
typedef __u64   Elf64_Xword;
typedef __s64   Elf64_Sxword;

typedef struct elf64_hdr {
  unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;		/* Entry point virtual address */
  Elf64_Off e_phoff;		/* Program header table file offset */
  Elf64_Off e_shoff;		/* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} elf64_ehdr;

typedef struct elf64_shdr {
  Elf64_Word sh_name;		/* Section name, index in string tbl */
  Elf64_Word sh_type;		/* Type of section */
  Elf64_Xword sh_flags;		/* Miscellaneous section attributes */
  Elf64_Addr sh_addr;		/* Section virtual addr at execution */
  Elf64_Off sh_offset;		/* Section file offset */
  Elf64_Xword sh_size;		/* Size of section in bytes */
  Elf64_Word sh_link;		/* Index of another section */
  Elf64_Word sh_info;		/* Additional section information */
  Elf64_Xword sh_addralign;	/* Section alignment */
  Elf64_Xword sh_entsize;	/* Entry size if section holds table */
} elf64_shdr;

typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;		/* Segment file offset */
  Elf64_Addr p_vaddr;		/* Segment virtual address */
  Elf64_Addr p_paddr;		/* Segment physical address */
  Elf64_Xword p_filesz;		/* Segment size in file */
  Elf64_Xword p_memsz;		/* Segment size in memory */
  Elf64_Xword p_align;		/* Segment alignment, file & memory */
} elf64_phdr;

typedef struct elf64_sym {
  Elf64_Word st_name;		/* Symbol name, index in string tbl */
  unsigned char	st_info;	/* Type and binding attributes */
  unsigned char	st_other;	/* No defined meaning, 0 */
  Elf64_Half st_shndx;		/* Associated section index */
  Elf64_Addr st_value;		/* Value of the symbol */
  Elf64_Xword st_size;		/* Associated symbol size */
} elf64_sym;


// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: dannyb@google.com (Daniel Berlin)

#ifndef UTIL_DEBUGINFO_DWARF2ENUMS_H__
#define UTIL_DEBUGINFO_DWARF2ENUMS_H__

// These enums do not follow the google3 style only because they are
// known universally (specs, other implementations) by the names in
// exactly this capitalization.
// Tag names and codes.
#if 0
enum DwarfTag {
  DW_TAG_padding = 0x00,
  DW_TAG_array_type = 0x01,
  DW_TAG_class_type = 0x02,
  DW_TAG_entry_point = 0x03,
  DW_TAG_enumeration_type = 0x04,
  DW_TAG_formal_parameter = 0x05,
  DW_TAG_imported_declaration = 0x08,
  DW_TAG_label = 0x0a,
  DW_TAG_lexical_block = 0x0b,
  DW_TAG_member = 0x0d,
  DW_TAG_pointer_type = 0x0f,
  DW_TAG_reference_type = 0x10,
  DW_TAG_compile_unit = 0x11,
  DW_TAG_string_type = 0x12,
  DW_TAG_structure_type = 0x13,
  DW_TAG_subroutine_type = 0x15,
  DW_TAG_typedef = 0x16,
  DW_TAG_union_type = 0x17,
  DW_TAG_unspecified_parameters = 0x18,
  DW_TAG_variant = 0x19,
  DW_TAG_common_block = 0x1a,
  DW_TAG_common_inclusion = 0x1b,
  DW_TAG_inheritance = 0x1c,
  DW_TAG_inlined_subroutine = 0x1d,
  DW_TAG_module = 0x1e,
  DW_TAG_ptr_to_member_type = 0x1f,
  DW_TAG_set_type = 0x20,
  DW_TAG_subrange_type = 0x21,
  DW_TAG_with_stmt = 0x22,
  DW_TAG_access_declaration = 0x23,
  DW_TAG_base_type = 0x24,
  DW_TAG_catch_block = 0x25,
  DW_TAG_const_type = 0x26,
  DW_TAG_constant = 0x27,
  DW_TAG_enumerator = 0x28,
  DW_TAG_file_type = 0x29,
  DW_TAG_friend = 0x2a,
  DW_TAG_namelist = 0x2b,
  DW_TAG_namelist_item = 0x2c,
  DW_TAG_packed_type = 0x2d,
  DW_TAG_subprogram = 0x2e,
  DW_TAG_template_type_param = 0x2f,
  DW_TAG_template_value_param = 0x30,
  DW_TAG_thrown_type = 0x31,
  DW_TAG_try_block = 0x32,
  DW_TAG_variant_part = 0x33,
  DW_TAG_variable = 0x34,
  DW_TAG_volatile_type = 0x35,
  // DWARF 3.
  DW_TAG_dwarf_procedure = 0x36,
  DW_TAG_restrict_type = 0x37,
  DW_TAG_interface_type = 0x38,
  DW_TAG_namespace = 0x39,
  DW_TAG_imported_module = 0x3a,
  DW_TAG_unspecified_type = 0x3b,
  DW_TAG_partial_unit = 0x3c,
  DW_TAG_imported_unit = 0x3d,
  DW_TAG_condition = 0x3f,
  DW_TAG_shared_type = 0x40,
  // DWARF 4.
  DW_TAG_type_unit = 0x41,
  DW_TAG_rvalue_reference_type = 0x42,
  DW_TAG_template_alias = 0x43,
  DW_TAG_lo_user = 0x4080,
  DW_TAG_hi_user = 0xffff,
  // SGI/MIPS Extensions.
  DW_TAG_MIPS_loop = 0x4081,
  // HP extensions.  See:
  // ftp://ftp.hp.com/pub/lang/tools/WDB/wdb-4.0.tar.gz
  DW_TAG_HP_array_descriptor = 0x4090,
  // GNU extensions.
  DW_TAG_format_label = 0x4101,  // For FORTRAN 77 and Fortran 90.
  DW_TAG_function_template = 0x4102,  // For C++.
  DW_TAG_class_template = 0x4103,  // For C++.
  DW_TAG_GNU_BINCL = 0x4104,
  DW_TAG_GNU_EINCL = 0x4105,
  // http://gcc.gnu.org/wiki/TemplateParmsDwarf
  DW_TAG_GNU_template_template_param = 0x4106,
  DW_TAG_GNU_template_parameter_pack = 0x4107,
  DW_TAG_GNU_formal_parameter_pack = 0x4108,
  // http://www.dwarfstd.org/ShowIssue.php?issue=100909.2&type=open
  DW_TAG_GNU_call_site = 0x4109,
  DW_TAG_GNU_call_site_parameter = 0x410a,
  // Apple extensions.
  DW_TAG_APPLE_property = 0x4200,
  // Extensions for UPC.  See: http://upc.gwu.edu/~upc.
  DW_TAG_upc_shared_type = 0x8765,
  DW_TAG_upc_strict_type = 0x8766,
  DW_TAG_upc_relaxed_type = 0x8767,
  // PGI (STMicroelectronics) extensions.  No documentation available.
  DW_TAG_PGI_kanji_type      = 0xA000,
  DW_TAG_PGI_interface_block = 0xA020
};


enum DwarfHasChild {
  DW_children_no = 0,
  DW_children_yes = 1
};

// Form names and codes.
enum DwarfForm {
  DW_FORM_addr = 0x01,
  DW_FORM_block2 = 0x03,
  DW_FORM_block4 = 0x04,
  DW_FORM_data2 = 0x05,
  DW_FORM_data4 = 0x06,
  DW_FORM_data8 = 0x07,
  DW_FORM_string = 0x08,
  DW_FORM_block = 0x09,
  DW_FORM_block1 = 0x0a,
  DW_FORM_data1 = 0x0b,
  DW_FORM_flag = 0x0c,
  DW_FORM_sdata = 0x0d,
  DW_FORM_strp = 0x0e,
  DW_FORM_udata = 0x0f,
  DW_FORM_ref_addr = 0x10,
  DW_FORM_ref1 = 0x11,
  DW_FORM_ref2 = 0x12,
  DW_FORM_ref4 = 0x13,
  DW_FORM_ref8 = 0x14,
  DW_FORM_ref_udata = 0x15,
  DW_FORM_indirect = 0x16,
  // DWARF 4.
  DW_FORM_sec_offset = 0x17,
  DW_FORM_exprloc = 0x18,
  DW_FORM_flag_present = 0x19,
  // DWARF 5.
  DW_FORM_strx = 0x1a,
  DW_FORM_addrx = 0x1b,
  DW_FORM_ref_sup4 = 0x1c,
  DW_FORM_strp_sup = 0x1d,
  DW_FORM_data16 = 0x1e,
  DW_FORM_line_strp = 0x1f,
  // DWARF 4.
  DW_FORM_ref_sig8 = 0x20,
  // DWARF 5.
  DW_FORM_implicit_const = 0x21,
  DW_FORM_loclistx = 0x22,
  DW_FORM_rnglistx = 0x23,
  DW_FORM_ref_sup8 = 0x24,
  DW_FORM_strx1 = 0x25,
  DW_FORM_strx2 = 0x26,
  DW_FORM_strx3 = 0x27,
  DW_FORM_strx4 = 0x28,
  DW_FORM_addrx1 = 0x29,
  DW_FORM_addrx2 = 0x2a,
  DW_FORM_addrx3 = 0x2b,
  DW_FORM_addrx4 = 0x2c,
  // Extensions for Fission.  See http://gcc.gnu.org/wiki/DebugFission.
  DW_FORM_GNU_addr_index = 0x1f01,
  DW_FORM_GNU_str_index = 0x1f02
};

// Attribute names and codes
enum DwarfAttribute {
  DW_AT_sibling = 0x01,
  DW_AT_location = 0x02,
  DW_AT_name = 0x03,
  DW_AT_ordering = 0x09,
  DW_AT_subscr_data = 0x0a,
  DW_AT_byte_size = 0x0b,
  DW_AT_bit_offset = 0x0c,
  DW_AT_bit_size = 0x0d,
  DW_AT_element_list = 0x0f,
  DW_AT_stmt_list = 0x10,
  DW_AT_low_pc = 0x11,
  DW_AT_high_pc = 0x12,
  DW_AT_language = 0x13,
  DW_AT_member = 0x14,
  DW_AT_discr = 0x15,
  DW_AT_discr_value = 0x16,
  DW_AT_visibility = 0x17,
  DW_AT_import = 0x18,
  DW_AT_string_length = 0x19,
  DW_AT_common_reference = 0x1a,
  DW_AT_comp_dir = 0x1b,
  DW_AT_const_value = 0x1c,
  DW_AT_containing_type = 0x1d,
  DW_AT_default_value = 0x1e,
  DW_AT_inline = 0x20,
  DW_AT_is_optional = 0x21,
  DW_AT_lower_bound = 0x22,
  DW_AT_producer = 0x25,
  DW_AT_prototyped = 0x27,
  DW_AT_return_addr = 0x2a,
  DW_AT_start_scope = 0x2c,
  DW_AT_stride_size = 0x2e,
  DW_AT_upper_bound = 0x2f,
  DW_AT_abstract_origin = 0x31,
  DW_AT_accessibility = 0x32,
  DW_AT_address_class = 0x33,
  DW_AT_artificial = 0x34,
  DW_AT_base_types = 0x35,
  DW_AT_calling_convention = 0x36,
  DW_AT_count = 0x37,
  DW_AT_data_member_location = 0x38,
  DW_AT_decl_column = 0x39,
  DW_AT_decl_file = 0x3a,
  DW_AT_decl_line = 0x3b,
  DW_AT_declaration = 0x3c,
  DW_AT_discr_list = 0x3d,
  DW_AT_encoding = 0x3e,
  DW_AT_external = 0x3f,
  DW_AT_frame_base = 0x40,
  DW_AT_friend = 0x41,
  DW_AT_identifier_case = 0x42,
  DW_AT_macro_info = 0x43,
  DW_AT_namelist_items = 0x44,
  DW_AT_priority = 0x45,
  DW_AT_segment = 0x46,
  DW_AT_specification = 0x47,
  DW_AT_static_link = 0x48,
  DW_AT_type = 0x49,
  DW_AT_use_location = 0x4a,
  DW_AT_variable_parameter = 0x4b,
  DW_AT_virtuality = 0x4c,
  DW_AT_vtable_elem_location = 0x4d,
  // DWARF 3 values.
  DW_AT_allocated = 0x4e,
  DW_AT_associated = 0x4f,
  DW_AT_data_location = 0x50,
  DW_AT_stride = 0x51,
  DW_AT_entry_pc = 0x52,
  DW_AT_use_UTF8 = 0x53,
  DW_AT_extension = 0x54,
  DW_AT_ranges = 0x55,
  DW_AT_trampoline = 0x56,
  DW_AT_call_column = 0x57,
  DW_AT_call_file = 0x58,
  DW_AT_call_line = 0x59,
  DW_AT_description = 0x5a,
  DW_AT_binary_scale = 0x5b,
  DW_AT_decimal_scale = 0x5c,
  DW_AT_small = 0x5d,
  DW_AT_descimal_sign = 0x5e,
  DW_AT_digit_count = 0x5f,
  DW_AT_picture_string = 0x60,
  DW_AT_mutable = 0x61,
  DW_AT_threads_scaled = 0x62,
  DW_AT_explicit = 0x63,
  DW_AT_object_pointer = 0x64,
  DW_AT_endianity = 0x65,
  DW_AT_elemental = 0x66,
  DW_AT_pure = 0x67,
  DW_AT_recursive = 0x68,
  DW_AT_lo_user = 0x2000,
  DW_AT_hi_user = 0x3fff,
  // DWARF 4 values.
  DW_AT_signature = 0x69,
  DW_AT_main_subprogram = 0x6a,
  DW_AT_data_bit_offset = 0x6b,
  DW_AT_const_expr = 0x6c,
  DW_AT_enum_class = 0x6d,
  DW_AT_linkage_name = 0x6e,
  // DWARF 5 values.
  DW_AT_string_length_bit_size = 0x6f,
  DW_AT_string_length_byte_size = 0x70,
  DW_AT_rank = 0x71,
  DW_AT_str_offsets_base = 0x72,
  DW_AT_addr_base = 0x73,
  DW_AT_rnglists_base = 0x74,
  DW_AT_dwo_name = 0x76,
  DW_AT_reference = 0x77,
  DW_AT_rvalue_reference = 0x78,
  DW_AT_macros = 0x79,
  DW_AT_call_all_calls = 0x7a,
  DW_AT_call_all_source_calls = 0x7b,
  DW_AT_call_all_tail_calls = 0x7c,
  DW_AT_call_return_pc = 0x7d,
  DW_AT_call_value = 0x7e,
  DW_AT_call_origin = 0x7f,
  DW_AT_call_parameter = 0x80,
  DW_AT_call_pc = 0x81,
  DW_AT_call_tail_call = 0x82,
  DW_AT_call_target = 0x83,
  DW_AT_call_target_clobbered = 0x84,
  DW_AT_call_data_location = 0x85,
  DW_AT_call_data_value = 0x86,
  DW_AT_noreturn = 0x87,
  DW_AT_alignment = 0x88,
  DW_AT_export_symbols = 0x89,
  DW_AT_deleted = 0x8a,
  DW_AT_defaulted = 0x8b,
  DW_AT_loclists_base = 0x8c,
  // SGI/MIPS extensions.
  DW_AT_MIPS_fde = 0x2001,
  DW_AT_MIPS_loop_begin = 0x2002,
  DW_AT_MIPS_tail_loop_begin = 0x2003,
  DW_AT_MIPS_epilog_begin = 0x2004,
  DW_AT_MIPS_loop_unroll_factor = 0x2005,
  DW_AT_MIPS_software_pipeline_depth = 0x2006,
  DW_AT_MIPS_linkage_name = 0x2007,
  DW_AT_MIPS_stride = 0x2008,
  DW_AT_MIPS_abstract_name = 0x2009,
  DW_AT_MIPS_clone_origin = 0x200a,
  DW_AT_MIPS_has_inlines = 0x200b,
  // HP extensions.
  DW_AT_HP_block_index         = 0x2000,
  DW_AT_HP_unmodifiable        = 0x2001,  // Same as DW_AT_MIPS_fde.
  DW_AT_HP_actuals_stmt_list   = 0x2010,
  DW_AT_HP_proc_per_section    = 0x2011,
  DW_AT_HP_raw_data_ptr        = 0x2012,
  DW_AT_HP_pass_by_reference   = 0x2013,
  DW_AT_HP_opt_level           = 0x2014,
  DW_AT_HP_prof_version_id     = 0x2015,
  DW_AT_HP_opt_flags           = 0x2016,
  DW_AT_HP_cold_region_low_pc  = 0x2017,
  DW_AT_HP_cold_region_high_pc = 0x2018,
  DW_AT_HP_all_variables_modifiable = 0x2019,
  DW_AT_HP_linkage_name        = 0x201a,
  DW_AT_HP_prof_flags          = 0x201b,  // In comp unit of procs_info for -g.
  // GNU extensions.
  DW_AT_sf_names   = 0x2101,
  DW_AT_src_info   = 0x2102,
  DW_AT_mac_info   = 0x2103,
  DW_AT_src_coords = 0x2104,
  DW_AT_body_begin = 0x2105,
  DW_AT_body_end   = 0x2106,
  DW_AT_GNU_vector = 0x2107,
  // http://gcc.gnu.org/wiki/ThreadSafetyAnnotation
  DW_AT_GNU_guarded_by = 0x2108,
  DW_AT_GNU_pt_guarded_by = 0x2109,
  DW_AT_GNU_guarded = 0x210a,
  DW_AT_GNU_pt_guarded = 0x210b,
  DW_AT_GNU_locks_excluded = 0x210c,
  DW_AT_GNU_exclusive_locks_required = 0x210d,
  DW_AT_GNU_shared_locks_required = 0x210e,
  // http://gcc.gnu.org/wiki/DwarfSeparateTypeInfo
  DW_AT_GNU_odr_signature = 0x210f,
  // http://gcc.gnu.org/wiki/TemplateParmsDwarf
  DW_AT_GNU_template_name = 0x2110,
  // http://www.dwarfstd.org/ShowIssue.php?issue=100909.2&type=open
  DW_AT_GNU_call_site_value = 0x2111,
  DW_AT_GNU_call_site_data_value = 0x2112,
  DW_AT_GNU_call_site_target = 0x2113,
  DW_AT_GNU_call_site_target_clobbered = 0x2114,
  DW_AT_GNU_tail_call = 0x2115,
  DW_AT_GNU_all_tail_call_sites = 0x2116,
  DW_AT_GNU_all_call_sites = 0x2117,
  DW_AT_GNU_all_source_call_sites = 0x2118,
  DW_AT_GNU_macros = 0x2119,
  // Extensions for Fission.  See http://gcc.gnu.org/wiki/DebugFission.
  DW_AT_GNU_dwo_name = 0x2130,
  DW_AT_GNU_dwo_id = 0x2131,
  DW_AT_GNU_ranges_base = 0x2132,
  DW_AT_GNU_addr_base = 0x2133,
  DW_AT_GNU_pubnames = 0x2134,
  DW_AT_GNU_pubtypes = 0x2135,
  // discriminator.
  DW_AT_GNU_discriminator = 0x2136,
  // VMS extensions.
  DW_AT_VMS_rtnbeg_pd_address = 0x2201,
  // UPC extension.
  DW_AT_upc_threads_scaled = 0x3210,
  // PGI (STMicroelectronics) extensions.
  DW_AT_PGI_lbase    = 0x3a00,
  DW_AT_PGI_soffset  = 0x3a01,
  DW_AT_PGI_lstride  = 0x3a02,
  // Apple extensions.
  DW_AT_APPLE_optimized          = 0x3fe1,
  DW_AT_APPLE_flags              = 0x3fe2,
  DW_AT_APPLE_isa                = 0x3fe3,
  DW_AT_APPLE_block              = 0x3fe4,
  DW_AT_APPLE_major_runtime_vers = 0x3fe5,
  DW_AT_APPLE_runtime_class      = 0x3fe6,
  DW_AT_APPLE_omit_frame_ptr     = 0x3fe7,
  DW_AT_APPLE_property_name      = 0x3fe8,
  DW_AT_APPLE_property_getter    = 0x3fe9,
  DW_AT_APPLE_property_setter    = 0x3fea,
  DW_AT_APPLE_property_attribute = 0x3feb,
  DW_AT_APPLE_objc_complete_type = 0x3fec
};


// Line number opcodes.
enum DwarfLineNumberOps {
  DW_LNS_extended_op = 0,
  DW_LNS_copy = 1,
  DW_LNS_advance_pc = 2,
  DW_LNS_advance_line = 3,
  DW_LNS_set_file = 4,
  DW_LNS_set_column = 5,
  DW_LNS_negate_stmt = 6,
  DW_LNS_set_basic_block = 7,
  DW_LNS_const_add_pc = 8,
  DW_LNS_fixed_advance_pc = 9,
  // DWARF 3.
  DW_LNS_set_prologue_end = 10,
  DW_LNS_set_epilogue_begin = 11,
  DW_LNS_set_isa = 12,
  /* Experimental DWARF 5 extensions.
     See http://wiki.dwarfstd.org/index.php?title=TwoLevelLineTables.  */
  DW_LNS_set_address_from_logical = 13, /* Actuals table only.  */
  DW_LNS_set_subprogram = 13,           /* Logicals table only.  */
  DW_LNS_inlined_call = 14,             /* Logicals table only.  */
  DW_LNS_pop_context = 15               /* Logicals table only.  */
};

// Line number extended opcodes.
enum DwarfLineNumberExtendedOps {
  DW_LNE_end_sequence = 1,
  DW_LNE_set_address = 2,
  DW_LNE_define_file = 3,
  DW_LNE_set_discriminator = 4,
  DW_LNE_lo_user = 0x80,
  DW_LNE_hi_user = 0xff,
  // HP extensions.
  DW_LNE_HP_negate_is_UV_update      = 0x11,
  DW_LNE_HP_push_context             = 0x12,
  DW_LNE_HP_pop_context              = 0x13,
  DW_LNE_HP_set_file_line_column     = 0x14,
  DW_LNE_HP_set_routine_name         = 0x15,
  DW_LNE_HP_set_sequence             = 0x16,
  DW_LNE_HP_negate_post_semantics    = 0x17,
  DW_LNE_HP_negate_function_exit     = 0x18,
  DW_LNE_HP_negate_front_end_logical = 0x19,
  DW_LNE_HP_define_proc              = 0x20
};

// Line number content type codes (DWARF 5).
enum DwarfLineNumberContentType {
  DW_LNCT_path = 1,
  DW_LNCT_directory_index = 2,
  DW_LNCT_timestamp = 3,
  DW_LNCT_size = 4,
  DW_LNCT_MD5 = 5,
  // Experimental DWARF 5 extensions.
  // See http://wiki.dwarfstd.org/index.php?title=TwoLevelLineTables.
  DW_LNCT_subprogram_name = 6,
  DW_LNCT_decl_file = 7,
  DW_LNCT_decl_line = 8
};

// Type encoding names and codes
enum DwarfEncoding {
  DW_ATE_address                     =0x1,
  DW_ATE_boolean                     =0x2,
  DW_ATE_complex_float               =0x3,
  DW_ATE_float                       =0x4,
  DW_ATE_signed                      =0x5,
  DW_ATE_signed_char                 =0x6,
  DW_ATE_unsigned                    =0x7,
  DW_ATE_unsigned_char               =0x8,
  // DWARF3/DWARF3f
  DW_ATE_imaginary_float             =0x9,
  DW_ATE_packed_decimal              =0xa,
  DW_ATE_numeric_string              =0xb,
  DW_ATE_edited                      =0xc,
  DW_ATE_signed_fixed                =0xd,
  DW_ATE_unsigned_fixed              =0xe,
  DW_ATE_decimal_float               =0xf,
  // DWARF4
  DW_ATR_UTF                         =0x10,
  DW_ATE_lo_user                     =0x80,
  DW_ATE_hi_user                     =0xff
};

// Location virtual machine opcodes
enum DwarfOpcode {
  DW_OP_addr                         =0x03,
  DW_OP_deref                        =0x06,
  DW_OP_const1u                      =0x08,
  DW_OP_const1s                      =0x09,
  DW_OP_const2u                      =0x0a,
  DW_OP_const2s                      =0x0b,
  DW_OP_const4u                      =0x0c,
  DW_OP_const4s                      =0x0d,
  DW_OP_const8u                      =0x0e,
  DW_OP_const8s                      =0x0f,
  DW_OP_constu                       =0x10,
  DW_OP_consts                       =0x11,
  DW_OP_dup                          =0x12,
  DW_OP_drop                         =0x13,
  DW_OP_over                         =0x14,
  DW_OP_pick                         =0x15,
  DW_OP_swap                         =0x16,
  DW_OP_rot                          =0x17,
  DW_OP_xderef                       =0x18,
  DW_OP_abs                          =0x19,
  DW_OP_and                          =0x1a,
  DW_OP_div                          =0x1b,
  DW_OP_minus                        =0x1c,
  DW_OP_mod                          =0x1d,
  DW_OP_mul                          =0x1e,
  DW_OP_neg                          =0x1f,
  DW_OP_not                          =0x20,
  DW_OP_or                           =0x21,
  DW_OP_plus                         =0x22,
  DW_OP_plus_uconst                  =0x23,
  DW_OP_shl                          =0x24,
  DW_OP_shr                          =0x25,
  DW_OP_shra                         =0x26,
  DW_OP_xor                          =0x27,
  DW_OP_bra                          =0x28,
  DW_OP_eq                           =0x29,
  DW_OP_ge                           =0x2a,
  DW_OP_gt                           =0x2b,
  DW_OP_le                           =0x2c,
  DW_OP_lt                           =0x2d,
  DW_OP_ne                           =0x2e,
  DW_OP_skip                         =0x2f,
  DW_OP_lit0                         =0x30,
  DW_OP_lit1                         =0x31,
  DW_OP_lit2                         =0x32,
  DW_OP_lit3                         =0x33,
  DW_OP_lit4                         =0x34,
  DW_OP_lit5                         =0x35,
  DW_OP_lit6                         =0x36,
  DW_OP_lit7                         =0x37,
  DW_OP_lit8                         =0x38,
  DW_OP_lit9                         =0x39,
  DW_OP_lit10                        =0x3a,
  DW_OP_lit11                        =0x3b,
  DW_OP_lit12                        =0x3c,
  DW_OP_lit13                        =0x3d,
  DW_OP_lit14                        =0x3e,
  DW_OP_lit15                        =0x3f,
  DW_OP_lit16                        =0x40,
  DW_OP_lit17                        =0x41,
  DW_OP_lit18                        =0x42,
  DW_OP_lit19                        =0x43,
  DW_OP_lit20                        =0x44,
  DW_OP_lit21                        =0x45,
  DW_OP_lit22                        =0x46,
  DW_OP_lit23                        =0x47,
  DW_OP_lit24                        =0x48,
  DW_OP_lit25                        =0x49,
  DW_OP_lit26                        =0x4a,
  DW_OP_lit27                        =0x4b,
  DW_OP_lit28                        =0x4c,
  DW_OP_lit29                        =0x4d,
  DW_OP_lit30                        =0x4e,
  DW_OP_lit31                        =0x4f,
  DW_OP_reg0                         =0x50,
  DW_OP_reg1                         =0x51,
  DW_OP_reg2                         =0x52,
  DW_OP_reg3                         =0x53,
  DW_OP_reg4                         =0x54,
  DW_OP_reg5                         =0x55,
  DW_OP_reg6                         =0x56,
  DW_OP_reg7                         =0x57,
  DW_OP_reg8                         =0x58,
  DW_OP_reg9                         =0x59,
  DW_OP_reg10                        =0x5a,
  DW_OP_reg11                        =0x5b,
  DW_OP_reg12                        =0x5c,
  DW_OP_reg13                        =0x5d,
  DW_OP_reg14                        =0x5e,
  DW_OP_reg15                        =0x5f,
  DW_OP_reg16                        =0x60,
  DW_OP_reg17                        =0x61,
  DW_OP_reg18                        =0x62,
  DW_OP_reg19                        =0x63,
  DW_OP_reg20                        =0x64,
  DW_OP_reg21                        =0x65,
  DW_OP_reg22                        =0x66,
  DW_OP_reg23                        =0x67,
  DW_OP_reg24                        =0x68,
  DW_OP_reg25                        =0x69,
  DW_OP_reg26                        =0x6a,
  DW_OP_reg27                        =0x6b,
  DW_OP_reg28                        =0x6c,
  DW_OP_reg29                        =0x6d,
  DW_OP_reg30                        =0x6e,
  DW_OP_reg31                        =0x6f,
  DW_OP_breg0                        =0x70,
  DW_OP_breg1                        =0x71,
  DW_OP_breg2                        =0x72,
  DW_OP_breg3                        =0x73,
  DW_OP_breg4                        =0x74,
  DW_OP_breg5                        =0x75,
  DW_OP_breg6                        =0x76,
  DW_OP_breg7                        =0x77,
  DW_OP_breg8                        =0x78,
  DW_OP_breg9                        =0x79,
  DW_OP_breg10                       =0x7a,
  DW_OP_breg11                       =0x7b,
  DW_OP_breg12                       =0x7c,
  DW_OP_breg13                       =0x7d,
  DW_OP_breg14                       =0x7e,
  DW_OP_breg15                       =0x7f,
  DW_OP_breg16                       =0x80,
  DW_OP_breg17                       =0x81,
  DW_OP_breg18                       =0x82,
  DW_OP_breg19                       =0x83,
  DW_OP_breg20                       =0x84,
  DW_OP_breg21                       =0x85,
  DW_OP_breg22                       =0x86,
  DW_OP_breg23                       =0x87,
  DW_OP_breg24                       =0x88,
  DW_OP_breg25                       =0x89,
  DW_OP_breg26                       =0x8a,
  DW_OP_breg27                       =0x8b,
  DW_OP_breg28                       =0x8c,
  DW_OP_breg29                       =0x8d,
  DW_OP_breg30                       =0x8e,
  DW_OP_breg31                       =0x8f,
  DW_OP_regX                         =0x90,
  DW_OP_fbreg                        =0x91,
  DW_OP_bregX                        =0x92,
  DW_OP_piece                        =0x93,
  DW_OP_deref_size                   =0x94,
  DW_OP_xderef_size                  =0x95,
  DW_OP_nop                          =0x96,
  // DWARF3/DWARF3f
  DW_OP_push_object_address          =0x97,
  DW_OP_call2                        =0x98,
  DW_OP_call4                        =0x99,
  DW_OP_call_ref                     =0x9a,
  DW_OP_form_tls_address             =0x9b,
  DW_OP_call_frame_cfa               =0x9c,
  DW_OP_bit_piece                    =0x9d,
  // DWARF4
  DW_OP_implicit_value               =0x9e,
  DW_OP_stack_value                  =0x9f,
  DW_OP_lo_user                      =0xe0,
  DW_OP_hi_user                      =0xff,
  // GNU extensions
  DW_OP_GNU_push_tls_address         =0xe0,
  DW_OP_GNU_uninit                   =0xf0,
  DW_OP_GNU_encoded_addr             =0xf1,
  // http://www.dwarfstd.org/ShowIssue.php?issue=100831.1&type=open
  DW_OP_GNU_implicit_pointer         =0xf2,
  // http://www.dwarfstd.org/ShowIssue.php?issue=100909.1&type=open
  DW_OP_GNU_entry_value              =0xf3,
  // Extensions for Fission.  See http://gcc.gnu.org/wiki/DebugFission.
  DW_OP_GNU_addr_index               =0xfb,
  DW_OP_GNU_const_index              =0xfc
};

// Section identifiers for DWP files
enum DwarfSectionId {
  DW_SECT_INFO = 1,
  DW_SECT_TYPES = 2,
  DW_SECT_ABBREV = 3,
  DW_SECT_LINE = 4,
  DW_SECT_LOC = 5,
  DW_SECT_STR_OFFSETS = 6,
  DW_SECT_MACINFO = 7,
  DW_SECT_MACRO = 8
};

// For .eh_frame, see: http://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/dwarfext.html
enum PointerEncoding {
  DW_EH_PE_absptr      = 0x00,
  DW_EH_PE_uleb128     = 0x01,
  DW_EH_PE_udata2      = 0x02,
  DW_EH_PE_udata4      = 0x03,
  DW_EH_PE_udata8      = 0x04,
  DW_EH_PE_sleb128     = 0x09,
  DW_EH_PE_sdata2      = 0x0A,
  DW_EH_PE_sdata4      = 0x0B,
  DW_EH_PE_sdata8      = 0x0C,
  DW_EH_PE_FORMAT_MASK = 0x0f,

  DW_EH_PE_pcrel       = 0x10,
  DW_EH_PE_textrel     = 0x20,
  DW_EH_PE_datarel     = 0x30,
  DW_EH_PE_funcrel     = 0x40,
  DW_EH_PE_aligned     = 0x50,
  DW_EH_PE_APPLICATION_MASK = 0x70,

  // This acts as a flag.
  DW_EH_PE_indirect    = 0x80,

  DW_EH_PE_omit        = 0xff
};

enum RangeListEntry {
  DW_RLE_end_of_list = 0x00,
  DW_RLE_base_addressx = 0x01,
  DW_RLE_startx_endx = 0x02,
  DW_RLE_startx_length = 0x03,
  DW_RLE_offset_pair = 0x04,
  DW_RLE_base_address = 0x05,
  DW_RLE_start_end = 0x06,
  DW_RLE_start_length = 0x07,
};

enum UnitType {
  DW_UT_compile = 0x01,
  DW_UT_type = 0x02,
  DW_UT_partial = 0x03,
  DW_UT_skeleton = 0x04,
  DW_UT_split_compile = 0x05,
  DW_UT_split_type = 0x06,
  DW_UT_lo_user = 0x80,
  DW_UT_hi_user = 0xff,
};
#endif

#endif  // UTIL_DEBUGINFO_DWARF2ENUMS_H__


/* Declarations and definitions of codes relating to the DWARF2 and
   DWARF3 symbolic debugging information formats.
   Copyright (C) 1992, 1993, 1995, 1996, 1997, 1999, 2000, 2001, 2002,
   2003, 2004, 2005, 2006 Free Software Foundation, Inc.

   Written by Gary Funck (gary@intrepid.com) The Ada Joint Program
   Office (AJPO), Florida State University and Silicon Graphics Inc.
   provided support for this effort -- June 21, 1995.

   Derived from the DWARF 1 implementation written by Ron Guilmette
   (rfg@netcom.com), November 1990.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This file is derived from the DWARF specification (a public document)
   Revision 2.0.0 (July 27, 1993) developed by the UNIX International
   Programming Languages Special Interest Group (UI/PLSIG) and distributed
   by UNIX International.  Copies of this specification are available from
   UNIX International, 20 Waterview Boulevard, Parsippany, NJ, 07054.

   This file also now contains definitions from the DWARF 3 specification.  */

/* This file is shared between GCC and GDB, and should not contain
   prototypes.  */

#ifndef GCC_DWARF2_H
#define GCC_DWARF2_H

/* Structure found in the .debug_line section.  */
typedef struct
{
  unsigned char li_length          [4];
  unsigned char li_version         [2];
  unsigned char li_prologue_length [4];
  unsigned char li_min_insn_length [1];
  unsigned char li_default_is_stmt [1];
  unsigned char li_line_base       [1];
  unsigned char li_line_range      [1];
  unsigned char li_opcode_base     [1];
}
DWARF2_External_LineInfo;

typedef struct
{
  unsigned long  li_length;
  unsigned short li_version;
  unsigned int   li_prologue_length;
  unsigned char  li_min_insn_length;
  unsigned char  li_default_is_stmt;
  int            li_line_base;
  unsigned char  li_line_range;
  unsigned char  li_opcode_base;
}
DWARF2_Internal_LineInfo;

/* Structure found in .debug_pubnames section.  */
typedef struct
{
  unsigned char pn_length  [4];
  unsigned char pn_version [2];
  unsigned char pn_offset  [4];
  unsigned char pn_size    [4];
}
DWARF2_External_PubNames;

typedef struct
{
  unsigned long  pn_length;
  unsigned short pn_version;
  unsigned long  pn_offset;
  unsigned long  pn_size;
}
DWARF2_Internal_PubNames;

/* Structure found in .debug_info section.  */
typedef struct
{
  unsigned char  cu_length        [4];
  unsigned char  cu_version       [2];
  unsigned char  cu_abbrev_offset [4];
  unsigned char  cu_pointer_size  [1];
}
DWARF2_External_CompUnit;

typedef struct
{
  unsigned long  cu_length;
  unsigned short cu_version;
  unsigned long  cu_abbrev_offset;
  unsigned char  cu_pointer_size;
}
DWARF2_Internal_CompUnit;

typedef struct
{
  unsigned char  ar_length       [4];
  unsigned char  ar_version      [2];
  unsigned char  ar_info_offset  [4];
  unsigned char  ar_pointer_size [1];
  unsigned char  ar_segment_size [1];
}
DWARF2_External_ARange;

typedef struct
{
  unsigned long  ar_length;
  unsigned short ar_version;
  unsigned long  ar_info_offset;
  unsigned char  ar_pointer_size;
  unsigned char  ar_segment_size;
}
DWARF2_Internal_ARange;


/* Tag names and codes.  */
enum dwarf_tag
  {
    DW_TAG_padding = 0x00,
    DW_TAG_array_type = 0x01,
    DW_TAG_class_type = 0x02,
    DW_TAG_entry_point = 0x03,
    DW_TAG_enumeration_type = 0x04,
    DW_TAG_formal_parameter = 0x05,
    DW_TAG_imported_declaration = 0x08,
    DW_TAG_label = 0x0a,
    DW_TAG_lexical_block = 0x0b,
    DW_TAG_member = 0x0d,
    DW_TAG_pointer_type = 0x0f,
    DW_TAG_reference_type = 0x10,
    DW_TAG_compile_unit = 0x11,
    DW_TAG_string_type = 0x12,
    DW_TAG_structure_type = 0x13,
    DW_TAG_subroutine_type = 0x15,
    DW_TAG_typedef = 0x16,
    DW_TAG_union_type = 0x17,
    DW_TAG_unspecified_parameters = 0x18,
    DW_TAG_variant = 0x19,
    DW_TAG_common_block = 0x1a,
    DW_TAG_common_inclusion = 0x1b,
    DW_TAG_inheritance = 0x1c,
    DW_TAG_inlined_subroutine = 0x1d,
    DW_TAG_module = 0x1e,
    DW_TAG_ptr_to_member_type = 0x1f,
    DW_TAG_set_type = 0x20,
    DW_TAG_subrange_type = 0x21,
    DW_TAG_with_stmt = 0x22,
    DW_TAG_access_declaration = 0x23,
    DW_TAG_base_type = 0x24,
    DW_TAG_catch_block = 0x25,
    DW_TAG_const_type = 0x26,
    DW_TAG_constant = 0x27,
    DW_TAG_enumerator = 0x28,
    DW_TAG_file_type = 0x29,
    DW_TAG_friend = 0x2a,
    DW_TAG_namelist = 0x2b,
    DW_TAG_namelist_item = 0x2c,
    DW_TAG_packed_type = 0x2d,
    DW_TAG_subprogram = 0x2e,
    DW_TAG_template_type_param = 0x2f,
    DW_TAG_template_value_param = 0x30,
    DW_TAG_thrown_type = 0x31,
    DW_TAG_try_block = 0x32,
    DW_TAG_variant_part = 0x33,
    DW_TAG_variable = 0x34,
    DW_TAG_volatile_type = 0x35,
    /* DWARF 3.  */
    DW_TAG_dwarf_procedure = 0x36,
    DW_TAG_restrict_type = 0x37,
    DW_TAG_interface_type = 0x38,
    DW_TAG_namespace = 0x39,
    DW_TAG_imported_module = 0x3a,
    DW_TAG_unspecified_type = 0x3b,
    DW_TAG_partial_unit = 0x3c,
    DW_TAG_imported_unit = 0x3d,
    DW_TAG_condition = 0x3f,
    DW_TAG_shared_type = 0x40,
    /* SGI/MIPS Extensions.  */
    DW_TAG_MIPS_loop = 0x4081,
    /* HP extensions.  See: ftp://ftp.hp.com/pub/lang/tools/WDB/wdb-4.0.tar.gz .  */
    DW_TAG_HP_array_descriptor = 0x4090,
    /* GNU extensions.  */
    DW_TAG_format_label = 0x4101,	/* For FORTRAN 77 and Fortran 90.  */
    DW_TAG_function_template = 0x4102,	/* For C++.  */
    DW_TAG_class_template = 0x4103,	/* For C++.  */
    DW_TAG_GNU_BINCL = 0x4104,
    DW_TAG_GNU_EINCL = 0x4105,
    /* Extensions for UPC.  See: http://upc.gwu.edu/~upc.  */
    DW_TAG_upc_shared_type = 0x8765,
    DW_TAG_upc_strict_type = 0x8766,
    DW_TAG_upc_relaxed_type = 0x8767,
    /* PGI (STMicroelectronics) extensions.  No documentation available.  */
    DW_TAG_PGI_kanji_type      = 0xA000,
    DW_TAG_PGI_interface_block = 0xA020
  };

#define DW_TAG_lo_user	0x4080
#define DW_TAG_hi_user	0xffff

/* Flag that tells whether entry has a child or not.  */
#define DW_children_no   0
#define	DW_children_yes  1

/* Form names and codes.  */
enum dwarf_form
  {
    DW_FORM_addr = 0x01,
    DW_FORM_block2 = 0x03,
    DW_FORM_block4 = 0x04,
    DW_FORM_data2 = 0x05,
    DW_FORM_data4 = 0x06,
    DW_FORM_data8 = 0x07,
    DW_FORM_string = 0x08,
    DW_FORM_block = 0x09,
    DW_FORM_block1 = 0x0a,
    DW_FORM_data1 = 0x0b,
    DW_FORM_flag = 0x0c,
    DW_FORM_sdata = 0x0d,
    DW_FORM_strp = 0x0e,
    DW_FORM_udata = 0x0f,
    DW_FORM_ref_addr = 0x10,
    DW_FORM_ref1 = 0x11,
    DW_FORM_ref2 = 0x12,
    DW_FORM_ref4 = 0x13,
    DW_FORM_ref8 = 0x14,
    DW_FORM_ref_udata = 0x15,
    DW_FORM_indirect = 0x16
  };

/* Attribute names and codes.  */
enum dwarf_attribute
  {
    DW_AT_sibling = 0x01,
    DW_AT_location = 0x02,
    DW_AT_name = 0x03,
    DW_AT_ordering = 0x09,
    DW_AT_subscr_data = 0x0a,
    DW_AT_byte_size = 0x0b,
    DW_AT_bit_offset = 0x0c,
    DW_AT_bit_size = 0x0d,
    DW_AT_element_list = 0x0f,
    DW_AT_stmt_list = 0x10,
    DW_AT_low_pc = 0x11,
    DW_AT_high_pc = 0x12,
    DW_AT_language = 0x13,
    DW_AT_member = 0x14,
    DW_AT_discr = 0x15,
    DW_AT_discr_value = 0x16,
    DW_AT_visibility = 0x17,
    DW_AT_import = 0x18,
    DW_AT_string_length = 0x19,
    DW_AT_common_reference = 0x1a,
    DW_AT_comp_dir = 0x1b,
    DW_AT_const_value = 0x1c,
    DW_AT_containing_type = 0x1d,
    DW_AT_default_value = 0x1e,
    DW_AT_inline = 0x20,
    DW_AT_is_optional = 0x21,
    DW_AT_lower_bound = 0x22,
    DW_AT_producer = 0x25,
    DW_AT_prototyped = 0x27,
    DW_AT_return_addr = 0x2a,
    DW_AT_start_scope = 0x2c,
    DW_AT_stride_size = 0x2e,
    DW_AT_upper_bound = 0x2f,
    DW_AT_abstract_origin = 0x31,
    DW_AT_accessibility = 0x32,
    DW_AT_address_class = 0x33,
    DW_AT_artificial = 0x34,
    DW_AT_base_types = 0x35,
    DW_AT_calling_convention = 0x36,
    DW_AT_count = 0x37,
    DW_AT_data_member_location = 0x38,
    DW_AT_decl_column = 0x39,
    DW_AT_decl_file = 0x3a,
    DW_AT_decl_line = 0x3b,
    DW_AT_declaration = 0x3c,
    DW_AT_discr_list = 0x3d,
    DW_AT_encoding = 0x3e,
    DW_AT_external = 0x3f,
    DW_AT_frame_base = 0x40,
    DW_AT_friend = 0x41,
    DW_AT_identifier_case = 0x42,
    DW_AT_macro_info = 0x43,
    DW_AT_namelist_items = 0x44,
    DW_AT_priority = 0x45,
    DW_AT_segment = 0x46,
    DW_AT_specification = 0x47,
    DW_AT_static_link = 0x48,
    DW_AT_type = 0x49,
    DW_AT_use_location = 0x4a,
    DW_AT_variable_parameter = 0x4b,
    DW_AT_virtuality = 0x4c,
    DW_AT_vtable_elem_location = 0x4d,
    /* DWARF 3 values.  */
    DW_AT_allocated     = 0x4e,
    DW_AT_associated    = 0x4f,
    DW_AT_data_location = 0x50,
    DW_AT_stride        = 0x51,
    DW_AT_entry_pc      = 0x52,
    DW_AT_use_UTF8      = 0x53,
    DW_AT_extension     = 0x54,
    DW_AT_ranges        = 0x55,
    DW_AT_trampoline    = 0x56,
    DW_AT_call_column   = 0x57,
    DW_AT_call_file     = 0x58,
    DW_AT_call_line     = 0x59,
    DW_AT_description   = 0x5a,
    DW_AT_binary_scale  = 0x5b,
    DW_AT_decimal_scale = 0x5c,
    DW_AT_small         = 0x5d,
    DW_AT_decimal_sign  = 0x5e,
    DW_AT_digit_count   = 0x5f,
    DW_AT_picture_string = 0x60,
    DW_AT_mutable       = 0x61,
    DW_AT_threads_scaled = 0x62,
    DW_AT_explicit      = 0x63,
    DW_AT_object_pointer = 0x64,
    DW_AT_endianity     = 0x65,
    DW_AT_elemental     = 0x66,
    DW_AT_pure          = 0x67,
    DW_AT_recursive     = 0x68,
    /* SGI/MIPS extensions.  */
    DW_AT_MIPS_fde = 0x2001,
    DW_AT_MIPS_loop_begin = 0x2002,
    DW_AT_MIPS_tail_loop_begin = 0x2003,
    DW_AT_MIPS_epilog_begin = 0x2004,
    DW_AT_MIPS_loop_unroll_factor = 0x2005,
    DW_AT_MIPS_software_pipeline_depth = 0x2006,
    DW_AT_MIPS_linkage_name = 0x2007,
    DW_AT_MIPS_stride = 0x2008,
    DW_AT_MIPS_abstract_name = 0x2009,
    DW_AT_MIPS_clone_origin = 0x200a,
    DW_AT_MIPS_has_inlines = 0x200b,
    /* HP extensions.  */
    DW_AT_HP_block_index         = 0x2000,
    DW_AT_HP_unmodifiable        = 0x2001, /* Same as DW_AT_MIPS_fde.  */
    DW_AT_HP_actuals_stmt_list   = 0x2010,
    DW_AT_HP_proc_per_section    = 0x2011,
    DW_AT_HP_raw_data_ptr        = 0x2012,
    DW_AT_HP_pass_by_reference   = 0x2013,
    DW_AT_HP_opt_level           = 0x2014,
    DW_AT_HP_prof_version_id     = 0x2015,
    DW_AT_HP_opt_flags           = 0x2016,
    DW_AT_HP_cold_region_low_pc  = 0x2017,
    DW_AT_HP_cold_region_high_pc = 0x2018,
    DW_AT_HP_all_variables_modifiable = 0x2019,
    DW_AT_HP_linkage_name        = 0x201a,
    DW_AT_HP_prof_flags          = 0x201b,  /* In comp unit of procs_info for -g.  */
    /* GNU extensions.  */
    DW_AT_sf_names   = 0x2101,
    DW_AT_src_info   = 0x2102,
    DW_AT_mac_info   = 0x2103,
    DW_AT_src_coords = 0x2104,
    DW_AT_body_begin = 0x2105,
    DW_AT_body_end   = 0x2106,
    DW_AT_GNU_vector = 0x2107,
    /* VMS extensions.  */
    DW_AT_VMS_rtnbeg_pd_address = 0x2201,
    /* UPC extension.  */
    DW_AT_upc_threads_scaled = 0x3210,
    /* PGI (STMicroelectronics) extensions.  */
    DW_AT_PGI_lbase    = 0x3a00,
    DW_AT_PGI_soffset  = 0x3a01,
    /* APPLE LOCAL begin radar 5811943 - Fix type of pointers to blocks  */
    DW_AT_PGI_lstride  = 0x3a02,
    /* APPLE LOCAL begin radar 6386976  */
    DW_AT_APPLE_block    = 0x3fe4,
    /* APPLE LOCAL end radar 5811943 - Fix type of pointers to blocks  */
    DW_AT_APPLE_major_runtime_vers = 0x3fe5,
    DW_AT_APPLE_runtime_class = 0x3fe6
    /* APPLE LOCAL end radar 6386976  */
  };

#define DW_AT_lo_user	0x2000	/* Implementation-defined range start.  */
#define DW_AT_hi_user	0x3ff0	/* Implementation-defined range end.  */

/* Location atom names and codes.  */
enum dwarf_location_atom
  {
    DW_OP_addr = 0x03,
    DW_OP_deref = 0x06,
    DW_OP_const1u = 0x08,
    DW_OP_const1s = 0x09,
    DW_OP_const2u = 0x0a,
    DW_OP_const2s = 0x0b,
    DW_OP_const4u = 0x0c,
    DW_OP_const4s = 0x0d,
    DW_OP_const8u = 0x0e,
    DW_OP_const8s = 0x0f,
    DW_OP_constu = 0x10,
    DW_OP_consts = 0x11,
    DW_OP_dup = 0x12,
    DW_OP_drop = 0x13,
    DW_OP_over = 0x14,
    DW_OP_pick = 0x15,
    DW_OP_swap = 0x16,
    DW_OP_rot = 0x17,
    DW_OP_xderef = 0x18,
    DW_OP_abs = 0x19,
    DW_OP_and = 0x1a,
    DW_OP_div = 0x1b,
    DW_OP_minus = 0x1c,
    DW_OP_mod = 0x1d,
    DW_OP_mul = 0x1e,
    DW_OP_neg = 0x1f,
    DW_OP_not = 0x20,
    DW_OP_or = 0x21,
    DW_OP_plus = 0x22,
    DW_OP_plus_uconst = 0x23,
    DW_OP_shl = 0x24,
    DW_OP_shr = 0x25,
    DW_OP_shra = 0x26,
    DW_OP_xor = 0x27,
    DW_OP_bra = 0x28,
    DW_OP_eq = 0x29,
    DW_OP_ge = 0x2a,
    DW_OP_gt = 0x2b,
    DW_OP_le = 0x2c,
    DW_OP_lt = 0x2d,
    DW_OP_ne = 0x2e,
    DW_OP_skip = 0x2f,
    DW_OP_lit0 = 0x30,
    DW_OP_lit1 = 0x31,
    DW_OP_lit2 = 0x32,
    DW_OP_lit3 = 0x33,
    DW_OP_lit4 = 0x34,
    DW_OP_lit5 = 0x35,
    DW_OP_lit6 = 0x36,
    DW_OP_lit7 = 0x37,
    DW_OP_lit8 = 0x38,
    DW_OP_lit9 = 0x39,
    DW_OP_lit10 = 0x3a,
    DW_OP_lit11 = 0x3b,
    DW_OP_lit12 = 0x3c,
    DW_OP_lit13 = 0x3d,
    DW_OP_lit14 = 0x3e,
    DW_OP_lit15 = 0x3f,
    DW_OP_lit16 = 0x40,
    DW_OP_lit17 = 0x41,
    DW_OP_lit18 = 0x42,
    DW_OP_lit19 = 0x43,
    DW_OP_lit20 = 0x44,
    DW_OP_lit21 = 0x45,
    DW_OP_lit22 = 0x46,
    DW_OP_lit23 = 0x47,
    DW_OP_lit24 = 0x48,
    DW_OP_lit25 = 0x49,
    DW_OP_lit26 = 0x4a,
    DW_OP_lit27 = 0x4b,
    DW_OP_lit28 = 0x4c,
    DW_OP_lit29 = 0x4d,
    DW_OP_lit30 = 0x4e,
    DW_OP_lit31 = 0x4f,
    DW_OP_reg0 = 0x50,
    DW_OP_reg1 = 0x51,
    DW_OP_reg2 = 0x52,
    DW_OP_reg3 = 0x53,
    DW_OP_reg4 = 0x54,
    DW_OP_reg5 = 0x55,
    DW_OP_reg6 = 0x56,
    DW_OP_reg7 = 0x57,
    DW_OP_reg8 = 0x58,
    DW_OP_reg9 = 0x59,
    DW_OP_reg10 = 0x5a,
    DW_OP_reg11 = 0x5b,
    DW_OP_reg12 = 0x5c,
    DW_OP_reg13 = 0x5d,
    DW_OP_reg14 = 0x5e,
    DW_OP_reg15 = 0x5f,
    DW_OP_reg16 = 0x60,
    DW_OP_reg17 = 0x61,
    DW_OP_reg18 = 0x62,
    DW_OP_reg19 = 0x63,
    DW_OP_reg20 = 0x64,
    DW_OP_reg21 = 0x65,
    DW_OP_reg22 = 0x66,
    DW_OP_reg23 = 0x67,
    DW_OP_reg24 = 0x68,
    DW_OP_reg25 = 0x69,
    DW_OP_reg26 = 0x6a,
    DW_OP_reg27 = 0x6b,
    DW_OP_reg28 = 0x6c,
    DW_OP_reg29 = 0x6d,
    DW_OP_reg30 = 0x6e,
    DW_OP_reg31 = 0x6f,
    DW_OP_breg0 = 0x70,
    DW_OP_breg1 = 0x71,
    DW_OP_breg2 = 0x72,
    DW_OP_breg3 = 0x73,
    DW_OP_breg4 = 0x74,
    DW_OP_breg5 = 0x75,
    DW_OP_breg6 = 0x76,
    DW_OP_breg7 = 0x77,
    DW_OP_breg8 = 0x78,
    DW_OP_breg9 = 0x79,
    DW_OP_breg10 = 0x7a,
    DW_OP_breg11 = 0x7b,
    DW_OP_breg12 = 0x7c,
    DW_OP_breg13 = 0x7d,
    DW_OP_breg14 = 0x7e,
    DW_OP_breg15 = 0x7f,
    DW_OP_breg16 = 0x80,
    DW_OP_breg17 = 0x81,
    DW_OP_breg18 = 0x82,
    DW_OP_breg19 = 0x83,
    DW_OP_breg20 = 0x84,
    DW_OP_breg21 = 0x85,
    DW_OP_breg22 = 0x86,
    DW_OP_breg23 = 0x87,
    DW_OP_breg24 = 0x88,
    DW_OP_breg25 = 0x89,
    DW_OP_breg26 = 0x8a,
    DW_OP_breg27 = 0x8b,
    DW_OP_breg28 = 0x8c,
    DW_OP_breg29 = 0x8d,
    DW_OP_breg30 = 0x8e,
    DW_OP_breg31 = 0x8f,
    DW_OP_regx = 0x90,
    DW_OP_fbreg = 0x91,
    DW_OP_bregx = 0x92,
    DW_OP_piece = 0x93,
    DW_OP_deref_size = 0x94,
    DW_OP_xderef_size = 0x95,
    DW_OP_nop = 0x96,
    /* DWARF 3 extensions.  */
    DW_OP_push_object_address = 0x97,
    DW_OP_call2 = 0x98,
    DW_OP_call4 = 0x99,
    DW_OP_call_ref = 0x9a,
    DW_OP_form_tls_address = 0x9b,
    DW_OP_call_frame_cfa = 0x9c,
    DW_OP_bit_piece = 0x9d,
    /* GNU extensions.  */
    DW_OP_GNU_push_tls_address = 0xe0,
    /* HP extensions.  */
    DW_OP_HP_unknown     = 0xe0, /* Ouch, the same as GNU_push_tls_address.  */
    DW_OP_HP_is_value    = 0xe1,
    DW_OP_HP_fltconst4   = 0xe2,
    DW_OP_HP_fltconst8   = 0xe3,
    DW_OP_HP_mod_range   = 0xe4,
    DW_OP_HP_unmod_range = 0xe5,
    DW_OP_HP_tls         = 0xe6
  };

#define DW_OP_lo_user	0xe0	/* Implementation-defined range start.  */
#define DW_OP_hi_user	0xff	/* Implementation-defined range end.  */

/* Type encodings.  */
enum dwarf_type
  {
    DW_ATE_void = 0x0,
    DW_ATE_address = 0x1,
    DW_ATE_boolean = 0x2,
    DW_ATE_complex_float = 0x3,
    DW_ATE_float = 0x4,
    DW_ATE_signed = 0x5,
    DW_ATE_signed_char = 0x6,
    DW_ATE_unsigned = 0x7,
    DW_ATE_unsigned_char = 0x8,
    /* DWARF 3.  */
    DW_ATE_imaginary_float = 0x9,
    DW_ATE_packed_decimal = 0xa,
    DW_ATE_numeric_string = 0xb,
    DW_ATE_edited = 0xc,
    DW_ATE_signed_fixed = 0xd,
    DW_ATE_unsigned_fixed = 0xe,
    DW_ATE_decimal_float = 0xf,
    /* HP extensions.  */
    DW_ATE_HP_float80            = 0x80, /* Floating-point (80 bit).  */
    DW_ATE_HP_complex_float80    = 0x81, /* Complex floating-point (80 bit).  */
    DW_ATE_HP_float128           = 0x82, /* Floating-point (128 bit).  */
    DW_ATE_HP_complex_float128   = 0x83, /* Complex floating-point (128 bit).  */
    DW_ATE_HP_floathpintel       = 0x84, /* Floating-point (82 bit IA64).  */
    DW_ATE_HP_imaginary_float80  = 0x85,
    DW_ATE_HP_imaginary_float128 = 0x86
  };

#define	DW_ATE_lo_user 0x80
#define	DW_ATE_hi_user 0xff

/* Decimal sign encodings.  */
enum dwarf_decimal_sign_encoding
  {
    /* DWARF 3.  */
    DW_DS_unsigned = 0x01,
    DW_DS_leading_overpunch = 0x02,
    DW_DS_trailing_overpunch = 0x03,
    DW_DS_leading_separate = 0x04,
    DW_DS_trailing_separate = 0x05
  };

/* Endianity encodings.  */
enum dwarf_endianity_encoding
  {
    /* DWARF 3.  */
    DW_END_default = 0x00,
    DW_END_big = 0x01,
    DW_END_little = 0x02
  };

#define DW_END_lo_user 0x40
#define DW_END_hi_user 0xff

/* Array ordering names and codes.  */
enum dwarf_array_dim_ordering
  {
    DW_ORD_row_major = 0,
    DW_ORD_col_major = 1
  };

/* Access attribute.  */
enum dwarf_access_attribute
  {
    DW_ACCESS_public = 1,
    DW_ACCESS_protected = 2,
    DW_ACCESS_private = 3
  };

/* Visibility.  */
enum dwarf_visibility_attribute
  {
    DW_VIS_local = 1,
    DW_VIS_exported = 2,
    DW_VIS_qualified = 3
  };

/* Virtuality.  */
enum dwarf_virtuality_attribute
  {
    DW_VIRTUALITY_none = 0,
    DW_VIRTUALITY_virtual = 1,
    DW_VIRTUALITY_pure_virtual = 2
  };

/* Case sensitivity.  */
enum dwarf_id_case
  {
    DW_ID_case_sensitive = 0,
    DW_ID_up_case = 1,
    DW_ID_down_case = 2,
    DW_ID_case_insensitive = 3
  };

/* Calling convention.  */
enum dwarf_calling_convention
  {
    DW_CC_normal = 0x1,
    DW_CC_program = 0x2,
    DW_CC_nocall = 0x3,
    DW_CC_GNU_renesas_sh = 0x40
  };

#define DW_CC_lo_user 0x40
#define DW_CC_hi_user 0xff

/* Inline attribute.  */
enum dwarf_inline_attribute
  {
    DW_INL_not_inlined = 0,
    DW_INL_inlined = 1,
    DW_INL_declared_not_inlined = 2,
    DW_INL_declared_inlined = 3
  };

/* Discriminant lists.  */
enum dwarf_discrim_list
  {
    DW_DSC_label = 0,
    DW_DSC_range = 1
  };

/* Line number opcodes.  */
enum dwarf_line_number_ops
  {
    DW_LNS_extended_op = 0,
    DW_LNS_copy = 1,
    DW_LNS_advance_pc = 2,
    DW_LNS_advance_line = 3,
    DW_LNS_set_file = 4,
    DW_LNS_set_column = 5,
    DW_LNS_negate_stmt = 6,
    DW_LNS_set_basic_block = 7,
    DW_LNS_const_add_pc = 8,
    DW_LNS_fixed_advance_pc = 9,
    /* DWARF 3.  */
    DW_LNS_set_prologue_end = 10,
    DW_LNS_set_epilogue_begin = 11,
    DW_LNS_set_isa = 12
  };

/* Line number extended opcodes.  */
enum dwarf_line_number_x_ops
  {
    DW_LNE_end_sequence = 1,
    DW_LNE_set_address = 2,
    DW_LNE_define_file = 3,
    /* HP extensions.  */
    DW_LNE_HP_negate_is_UV_update      = 0x11,
    DW_LNE_HP_push_context             = 0x12,
    DW_LNE_HP_pop_context              = 0x13,
    DW_LNE_HP_set_file_line_column     = 0x14,
    DW_LNE_HP_set_routine_name         = 0x15,
    DW_LNE_HP_set_sequence             = 0x16,
    DW_LNE_HP_negate_post_semantics    = 0x17,
    DW_LNE_HP_negate_function_exit     = 0x18,
    DW_LNE_HP_negate_front_end_logical = 0x19,
    DW_LNE_HP_define_proc              = 0x20
  };

#define DW_LNE_lo_user 0x80
#define DW_LNE_hi_user 0xff

/* Call frame information.  */
enum dwarf_call_frame_info
  {
    DW_CFA_advance_loc = 0x40,
    DW_CFA_offset = 0x80,
    DW_CFA_restore = 0xc0,
    DW_CFA_nop = 0x00,
    DW_CFA_set_loc = 0x01,
    DW_CFA_advance_loc1 = 0x02,
    DW_CFA_advance_loc2 = 0x03,
    DW_CFA_advance_loc4 = 0x04,
    DW_CFA_offset_extended = 0x05,
    DW_CFA_restore_extended = 0x06,
    DW_CFA_undefined = 0x07,
    DW_CFA_same_value = 0x08,
    DW_CFA_register = 0x09,
    DW_CFA_remember_state = 0x0a,
    DW_CFA_restore_state = 0x0b,
    DW_CFA_def_cfa = 0x0c,
    DW_CFA_def_cfa_register = 0x0d,
    DW_CFA_def_cfa_offset = 0x0e,
    /* DWARF 3.  */
    DW_CFA_def_cfa_expression = 0x0f,
    DW_CFA_expression = 0x10,
    DW_CFA_offset_extended_sf = 0x11,
    DW_CFA_def_cfa_sf = 0x12,
    DW_CFA_def_cfa_offset_sf = 0x13,
    DW_CFA_val_offset = 0x14,
    DW_CFA_val_offset_sf = 0x15,
    DW_CFA_val_expression = 0x16,
    /* SGI/MIPS specific.  */
    DW_CFA_MIPS_advance_loc8 = 0x1d,
    /* GNU extensions.  */
    DW_CFA_GNU_window_save = 0x2d,
    DW_CFA_GNU_args_size = 0x2e,
    DW_CFA_GNU_negative_offset_extended = 0x2f
  };

#define DW_CIE_ID	  0xffffffff
#define DW64_CIE_ID	  0xffffffffffffffffULL
#define DW_CIE_VERSION	  1

#define DW_CFA_extended   0
#define DW_CFA_lo_user    0x1c
#define DW_CFA_hi_user    0x3f

#define DW_CHILDREN_no		     0x00
#define DW_CHILDREN_yes		     0x01

#define DW_ADDR_none		0

/* Source language names and codes.  */
enum dwarf_source_language
  {
    DW_LANG_C89 = 0x0001,
    DW_LANG_C = 0x0002,
    DW_LANG_Ada83 = 0x0003,
    DW_LANG_C_plus_plus = 0x0004,
    DW_LANG_Cobol74 = 0x0005,
    DW_LANG_Cobol85 = 0x0006,
    DW_LANG_Fortran77 = 0x0007,
    DW_LANG_Fortran90 = 0x0008,
    DW_LANG_Pascal83 = 0x0009,
    DW_LANG_Modula2 = 0x000a,
    /* DWARF 3.  */
    DW_LANG_Java = 0x000b,
    DW_LANG_C99 = 0x000c,
    DW_LANG_Ada95 = 0x000d,
    DW_LANG_Fortran95 = 0x000e,
    DW_LANG_PLI = 0x000f,
    DW_LANG_ObjC = 0x0010,
    DW_LANG_ObjC_plus_plus = 0x0011,
    DW_LANG_UPC = 0x0012,
    DW_LANG_D = 0x0013,
    /* MIPS.  */
    DW_LANG_Mips_Assembler = 0x8001,
    /* UPC.  */
    DW_LANG_Upc = 0x8765
  };

#define DW_LANG_lo_user 0x8000	/* Implementation-defined range start.  */
#define DW_LANG_hi_user 0xffff	/* Implementation-defined range start.  */

/* Names and codes for macro information.  */
enum dwarf_macinfo_record_type
  {
    DW_MACINFO_define = 1,
    DW_MACINFO_undef = 2,
    DW_MACINFO_start_file = 3,
    DW_MACINFO_end_file = 4,
    DW_MACINFO_vendor_ext = 255
  };

/* @@@ For use with GNU frame unwind information.  */

#define DW_EH_PE_absptr		0x00
#define DW_EH_PE_omit		0xff

#define DW_EH_PE_uleb128	0x01
#define DW_EH_PE_udata2		0x02
#define DW_EH_PE_udata4		0x03
#define DW_EH_PE_udata8		0x04
#define DW_EH_PE_sleb128	0x09
#define DW_EH_PE_sdata2		0x0A
#define DW_EH_PE_sdata4		0x0B
#define DW_EH_PE_sdata8		0x0C
#define DW_EH_PE_signed		0x08

#define DW_EH_PE_pcrel		0x10
#define DW_EH_PE_textrel	0x20
#define DW_EH_PE_datarel	0x30
#define DW_EH_PE_funcrel	0x40
#define DW_EH_PE_aligned	0x50

#define DW_EH_PE_indirect	0x80

    static char *
get_AT_name (unsigned long attribute)
{
    switch (attribute)
    {
        case DW_AT_sibling:			return "DW_AT_sibling";
        case DW_AT_location:		return "DW_AT_location";
        case DW_AT_name:			return "DW_AT_name";
        case DW_AT_ordering:		return "DW_AT_ordering";
        case DW_AT_subscr_data:		return "DW_AT_subscr_data";
        case DW_AT_byte_size:		return "DW_AT_byte_size";
        case DW_AT_bit_offset:		return "DW_AT_bit_offset";
        case DW_AT_bit_size:		return "DW_AT_bit_size";
        case DW_AT_element_list:		return "DW_AT_element_list";
        case DW_AT_stmt_list:		return "DW_AT_stmt_list";
        case DW_AT_low_pc:			return "DW_AT_low_pc";
        case DW_AT_high_pc:			return "DW_AT_high_pc";
        case DW_AT_language:		return "DW_AT_language";
        case DW_AT_member:			return "DW_AT_member";
        case DW_AT_discr:			return "DW_AT_discr";
        case DW_AT_discr_value:		return "DW_AT_discr_value";
        case DW_AT_visibility:		return "DW_AT_visibility";
        case DW_AT_import:			return "DW_AT_import";
        case DW_AT_string_length:		return "DW_AT_string_length";
        case DW_AT_common_reference:	return "DW_AT_common_reference";
        case DW_AT_comp_dir:		return "DW_AT_comp_dir";
        case DW_AT_const_value:		return "DW_AT_const_value";
        case DW_AT_containing_type:		return "DW_AT_containing_type";
        case DW_AT_default_value:		return "DW_AT_default_value";
        case DW_AT_inline:			return "DW_AT_inline";
        case DW_AT_is_optional:		return "DW_AT_is_optional";
        case DW_AT_lower_bound:		return "DW_AT_lower_bound";
        case DW_AT_producer:		return "DW_AT_producer";
        case DW_AT_prototyped:		return "DW_AT_prototyped";
        case DW_AT_return_addr:		return "DW_AT_return_addr";
        case DW_AT_start_scope:		return "DW_AT_start_scope";
        case DW_AT_stride_size:		return "DW_AT_stride_size";
        case DW_AT_upper_bound:		return "DW_AT_upper_bound";
        case DW_AT_abstract_origin:		return "DW_AT_abstract_origin";
        case DW_AT_accessibility:		return "DW_AT_accessibility";
        case DW_AT_address_class:		return "DW_AT_address_class";
        case DW_AT_artificial:		return "DW_AT_artificial";
        case DW_AT_base_types:		return "DW_AT_base_types";
        case DW_AT_calling_convention:	return "DW_AT_calling_convention";
        case DW_AT_count:			return "DW_AT_count";
        case DW_AT_data_member_location:	return "DW_AT_data_member_location";
        case DW_AT_decl_column:		return "DW_AT_decl_column";
        case DW_AT_decl_file:		return "DW_AT_decl_file";
        case DW_AT_decl_line:		return "DW_AT_decl_line";
        case DW_AT_declaration:		return "DW_AT_declaration";
        case DW_AT_discr_list:		return "DW_AT_discr_list";
        case DW_AT_encoding:		return "DW_AT_encoding";
        case DW_AT_external:		return "DW_AT_external";
        case DW_AT_frame_base:		return "DW_AT_frame_base";
        case DW_AT_friend:			return "DW_AT_friend";
        case DW_AT_identifier_case:		return "DW_AT_identifier_case";
        case DW_AT_macro_info:		return "DW_AT_macro_info";
        case DW_AT_namelist_items:		return "DW_AT_namelist_items";
        case DW_AT_priority:		return "DW_AT_priority";
        case DW_AT_segment:			return "DW_AT_segment";
        case DW_AT_specification:		return "DW_AT_specification";
        case DW_AT_static_link:		return "DW_AT_static_link";
        case DW_AT_type:			return "DW_AT_type";
        case DW_AT_use_location:		return "DW_AT_use_location";
        case DW_AT_variable_parameter:	return "DW_AT_variable_parameter";
        case DW_AT_virtuality:		return "DW_AT_virtuality";
        case DW_AT_vtable_elem_location:	return "DW_AT_vtable_elem_location";
                                            /* DWARF 2.1 values.  */
        case DW_AT_allocated:		return "DW_AT_allocated";
        case DW_AT_associated:		return "DW_AT_associated";
        case DW_AT_data_location:		return "DW_AT_data_location";
        case DW_AT_stride:			return "DW_AT_stride";
        case DW_AT_entry_pc:		return "DW_AT_entry_pc";
        case DW_AT_use_UTF8:		return "DW_AT_use_UTF8";
        case DW_AT_extension:		return "DW_AT_extension";
        case DW_AT_ranges:			return "DW_AT_ranges";
        case DW_AT_trampoline:		return "DW_AT_trampoline";
        case DW_AT_call_column:		return "DW_AT_call_column";
        case DW_AT_call_file:		return "DW_AT_call_file";
        case DW_AT_call_line:		return "DW_AT_call_line";
                                    /* SGI/MIPS extensions.  */
        case DW_AT_MIPS_fde:		return "DW_AT_MIPS_fde";
        case DW_AT_MIPS_loop_begin:		return "DW_AT_MIPS_loop_begin";
        case DW_AT_MIPS_tail_loop_begin:	return "DW_AT_MIPS_tail_loop_begin";
        case DW_AT_MIPS_epilog_begin:	return "DW_AT_MIPS_epilog_begin";
        case DW_AT_MIPS_loop_unroll_factor: return "DW_AT_MIPS_loop_unroll_factor";
        case DW_AT_MIPS_software_pipeline_depth:
                                            return "DW_AT_MIPS_software_pipeline_depth";
        case DW_AT_MIPS_linkage_name:	return "DW_AT_MIPS_linkage_name";
        case DW_AT_MIPS_stride:		return "DW_AT_MIPS_stride";
        case DW_AT_MIPS_abstract_name:	return "DW_AT_MIPS_abstract_name";
        case DW_AT_MIPS_clone_origin:	return "DW_AT_MIPS_clone_origin";
        case DW_AT_MIPS_has_inlines:	return "DW_AT_MIPS_has_inlines";
                                        /* GNU extensions.  */
        case DW_AT_sf_names:		return "DW_AT_sf_names";
        case DW_AT_src_info:		return "DW_AT_src_info";
        case DW_AT_mac_info:		return "DW_AT_mac_info";
        case DW_AT_src_coords:		return "DW_AT_src_coords";
        case DW_AT_body_begin:		return "DW_AT_body_begin";
        case DW_AT_body_end:		return "DW_AT_body_end";
        case DW_AT_GNU_vector:		return "DW_AT_GNU_vector";
                                    /* UPC extension.  */
        case DW_AT_upc_threads_scaled:	return "DW_AT_upc_threads_scaled";
        default:
                                        {
                                            static char buffer[100];

                                            sprintf (buffer, _("Unknown AT value: %lx"), attribute);
                                            return buffer;
                                        }
    }
}

    static char *
get_FORM_name (unsigned long form)
{
    switch (form)
    {
        case DW_FORM_addr:		return "DW_FORM_addr";
        case DW_FORM_block2:	return "DW_FORM_block2";
        case DW_FORM_block4:	return "DW_FORM_block4";
        case DW_FORM_data2:		return "DW_FORM_data2";
        case DW_FORM_data4:		return "DW_FORM_data4";
        case DW_FORM_data8:		return "DW_FORM_data8";
        case DW_FORM_string:	return "DW_FORM_string";
        case DW_FORM_block:		return "DW_FORM_block";
        case DW_FORM_block1:	return "DW_FORM_block1";
        case DW_FORM_data1:		return "DW_FORM_data1";
        case DW_FORM_flag:		return "DW_FORM_flag";
        case DW_FORM_sdata:		return "DW_FORM_sdata";
        case DW_FORM_strp:		return "DW_FORM_strp";
        case DW_FORM_udata:		return "DW_FORM_udata";
        case DW_FORM_ref_addr:	return "DW_FORM_ref_addr";
        case DW_FORM_ref1:		return "DW_FORM_ref1";
        case DW_FORM_ref2:		return "DW_FORM_ref2";
        case DW_FORM_ref4:		return "DW_FORM_ref4";
        case DW_FORM_ref8:		return "DW_FORM_ref8";
        case DW_FORM_ref_udata:	return "DW_FORM_ref_udata";
        case DW_FORM_indirect:	return "DW_FORM_indirect";
        default:
                                {
                                    static char buffer[100];

                                    sprintf (buffer, _("Unknown FORM value: %lx"), form);
                                    return buffer;
                                }
    }
}

    static char *
get_TAG_name (unsigned long tag)
{
    switch (tag)
    {
        case DW_TAG_padding:		return "DW_TAG_padding";
        case DW_TAG_array_type:		return "DW_TAG_array_type";
        case DW_TAG_class_type:		return "DW_TAG_class_type";
        case DW_TAG_entry_point:		return "DW_TAG_entry_point";
        case DW_TAG_enumeration_type:	return "DW_TAG_enumeration_type";
        case DW_TAG_formal_parameter:	return "DW_TAG_formal_parameter";
        case DW_TAG_imported_declaration:	return "DW_TAG_imported_declaration";
        case DW_TAG_label:			return "DW_TAG_label";
        case DW_TAG_lexical_block:		return "DW_TAG_lexical_block";
        case DW_TAG_member:			return "DW_TAG_member";
        case DW_TAG_pointer_type:		return "DW_TAG_pointer_type";
        case DW_TAG_reference_type:		return "DW_TAG_reference_type";
        case DW_TAG_compile_unit:		return "DW_TAG_compile_unit";
        case DW_TAG_string_type:		return "DW_TAG_string_type";
        case DW_TAG_structure_type:		return "DW_TAG_structure_type";
        case DW_TAG_subroutine_type:	return "DW_TAG_subroutine_type";
        case DW_TAG_typedef:		return "DW_TAG_typedef";
        case DW_TAG_union_type:		return "DW_TAG_union_type";
        case DW_TAG_unspecified_parameters: return "DW_TAG_unspecified_parameters";
        case DW_TAG_variant:		return "DW_TAG_variant";
        case DW_TAG_common_block:		return "DW_TAG_common_block";
        case DW_TAG_common_inclusion:	return "DW_TAG_common_inclusion";
        case DW_TAG_inheritance:		return "DW_TAG_inheritance";
        case DW_TAG_inlined_subroutine:	return "DW_TAG_inlined_subroutine";
        case DW_TAG_module:			return "DW_TAG_module";
        case DW_TAG_ptr_to_member_type:	return "DW_TAG_ptr_to_member_type";
        case DW_TAG_set_type:		return "DW_TAG_set_type";
        case DW_TAG_subrange_type:		return "DW_TAG_subrange_type";
        case DW_TAG_with_stmt:		return "DW_TAG_with_stmt";
        case DW_TAG_access_declaration:	return "DW_TAG_access_declaration";
        case DW_TAG_base_type:		return "DW_TAG_base_type";
        case DW_TAG_catch_block:		return "DW_TAG_catch_block";
        case DW_TAG_const_type:		return "DW_TAG_const_type";
        case DW_TAG_constant:		return "DW_TAG_constant";
        case DW_TAG_enumerator:		return "DW_TAG_enumerator";
        case DW_TAG_file_type:		return "DW_TAG_file_type";
        case DW_TAG_friend:			return "DW_TAG_friend";
        case DW_TAG_namelist:		return "DW_TAG_namelist";
        case DW_TAG_namelist_item:		return "DW_TAG_namelist_item";
        case DW_TAG_packed_type:		return "DW_TAG_packed_type";
        case DW_TAG_subprogram:		return "DW_TAG_subprogram";
        case DW_TAG_template_type_param:	return "DW_TAG_template_type_param";
        case DW_TAG_template_value_param:	return "DW_TAG_template_value_param";
        case DW_TAG_thrown_type:		return "DW_TAG_thrown_type";
        case DW_TAG_try_block:		return "DW_TAG_try_block";
        case DW_TAG_variant_part:		return "DW_TAG_variant_part";
        case DW_TAG_variable:		return "DW_TAG_variable";
        case DW_TAG_volatile_type:		return "DW_TAG_volatile_type";
        case DW_TAG_MIPS_loop:		return "DW_TAG_MIPS_loop";
        case DW_TAG_format_label:		return "DW_TAG_format_label";
        case DW_TAG_function_template:	return "DW_TAG_function_template";
        case DW_TAG_class_template:		return "DW_TAG_class_template";
                                        /* DWARF 2.1 values.  */
        case DW_TAG_dwarf_procedure:	return "DW_TAG_dwarf_procedure";
        case DW_TAG_restrict_type:		return "DW_TAG_restrict_type";
        case DW_TAG_interface_type:		return "DW_TAG_interface_type";
        case DW_TAG_namespace:		return "DW_TAG_namespace";
        case DW_TAG_imported_module:	return "DW_TAG_imported_module";
        case DW_TAG_unspecified_type:	return "DW_TAG_unspecified_type";
        case DW_TAG_partial_unit:		return "DW_TAG_partial_unit";
        case DW_TAG_imported_unit:		return "DW_TAG_imported_unit";
                                        /* UPC values.  */
        case DW_TAG_upc_shared_type:        return "DW_TAG_upc_shared_type";
        case DW_TAG_upc_strict_type:        return "DW_TAG_upc_strict_type";
        case DW_TAG_upc_relaxed_type:       return "DW_TAG_upc_relaxed_type";
        default:
                                            {
                                                static char buffer[100];

                                                sprintf (buffer, _("Unknown TAG value: %lx"), tag);
                                                return buffer;
                                            }
    }
}
#endif /* dwarf2.h */
