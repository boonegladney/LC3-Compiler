/*
 * "Copyright (c) 2014 by Fritz Sieker."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written
 * agreement is hereby granted, provided that the above copyright notice
 * and the following two paragraphs appear in all copies of this software,
 * that the files COPYING and NO_WARRANTY are included verbatim with
 * any distribution, and that the contents of the file README are included
 * verbatim as part of a file named README with any distribution.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE AUTHOR
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND THE AUTHOR NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 */

/** @file lc3.c
 *  @brief provided utility functions for LC3 assembler (do not modify)
 *  @details This file implements the interface defined in <code>lc3.h</code>.
 *  <p>
 *  @author Fritz Sieker
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define LC3_VAR

#include "lc3.h"

/** Global flag defining whether using .obj file or .hex file. If false, each
 *  LC3 instruction is encoded in 16 bits. If true, each instruction is encoded
 *  by 4 hex digits followed by a newline.
 */
static bool inHex;

/** A compact representation of the LC3 ISA plus pseudo-ops */
static LC3_inst_t lc3_instructions[] = {
  { -1, {{ "BR",        FMT_CL,    0x0000 }, { 0,      0,          0     }} },
  {  5, {{ "ADD",       FMT_RRR,   0x1000 }, { "ADD",  FMT_RRI5,   0x1020}} },
  { -1, {{ "LD",        FMT_RL,    0x2000 }, { 0,      0,          0     }} },
  { -1, {{ "ST",        FMT_RL,    0x3000 }, { 0,      0,          0     }} },
  { 11, {{ "JSRR",      FMT_R2,    0x4000 }, { "JSR",  FMT_I11,    0x4800}} },
  {  5, {{ "AND",       FMT_RRR,   0x5000 }, { "AND",  FMT_RRI5,   0x5020}} },
  { -1, {{ "LDR",       FMT_RRI6,  0x6000 }, { 0,      0,          0     }} },
  { -1, {{ "STR",       FMT_RRI6,  0x7000 }, { 0,      0,          0     }} },
  { -1, {{ "RTI",       FMT_,      0x8000 }, { 0,      0,          0     }} },
  { -1, {{ "NOT",       FMT_RR,    0x903F }, { 0,      0,          0     }} },
  { -1, {{ "LDI",       FMT_RL,    0xA000 }, { 0,      0,          0     }} },
  { -1, {{ "STI",       FMT_RL,    0xB000 }, { 0,      0,          0     }} },
  { -1, {{ "JMP",       FMT_R2,    0xC000 }, { "RET",  FMT_,       0xC1C0}} },

#ifdef STACK_OPS /* Reserved is pop/push */
  {  5, {{ "POP",       FMT_R1,    0xD181 }, { "PUSH", FMT_R1,     0xD1BF}} },

#else /* Reserved not used */
  { -1, {{ "???",       FMT_,      0x0000 }, { 0,      0,          0     }} },
#endif

  { -1, {{ "LEA",       FMT_RL,    0xE000 }, { 0,      0,          0     }} },
  { -1, {{ "TRAP",      FMT_V,     0xF000 }, { 0,      0,          0     }} },
  { -1, {{ ".ORIG",     FMT_16,    0x0000 }, { 0,      0,          0     }} },
  { -1, {{ ".END",      FMT_,      0x0000 }, { 0,      0,          0     }} },
  { -1, {{ ".BLKW",     FMT_16,    0x0000 }, { 0,      0,          0     }} },
  { -1, {{ ".FILL",     FMT_FILL,  0x0000 }, { 0,      0,          0     }} },
  { -1, {{ ".STRINGZ",  FMT_S,     0x0000 }, { 0,      0,          0     }} },
  { -1, {{ "GETC",      FMT_,      0xF020 }, { 0,      0,          0     }} },
  { -1, {{ "OUT",       FMT_,      0xF021 }, { 0,      0,          0     }} },
  { -1, {{ "PUTS",      FMT_,      0xF022 }, { 0,      0,          0     }} },
  { -1, {{ "IN",        FMT_,      0xF023 }, { 0,      0,          0     }} },
  { -1, {{ "PUTSP",     FMT_,      0xF024 }, { 0,      0,          0     }} },
  { -1, {{ "HALT",      FMT_,      0xF025 }, { 0,      0,          0     }} },
  { -1, {{ "GETS",      FMT_,      0xF026 }, { 0,      0,          0     }} },
  { -1, {{ "NEWLN",     FMT_,      0xF027 }, { 0,      0,          0     }} },
  { -1, {{ ".ZERO",     FMT_R1,    0x5020 }, { 0,      0,          0     }} },
  { -1, {{ ".COPY",     FMT_RR,    0x1020 }, { 0,      0,          0     }} },
  { -1, {{ ".SETCC",    FMT_R1,    0x1020 }, { 0,      0,          0     }} },
  {  5, {{ ".SUB",      FMT_RRR,   0x0000 }, { ".SUB", FMT_RRI5,   0x1020}} },
};

LC3_inst_t* lc3_get_inst_info (opcode_t opcode) {
  LC3_inst_t* inst = NULL;

  if ((opcode > OP_INVALID) && (opcode < NUM_OPCODES))
    inst = &(lc3_instructions[opcode]);

  return inst;
}

char* lc3_get_suffix (char* file_name) {
  char *suffix = strrchr(file_name, '.');

  if (suffix == NULL)
    suffix = file_name + strlen(file_name);

  return suffix;
}

bool lc3_file_has_suffix (const char* file_name, const char* suffix) {
  char* theSuffix = lc3_get_suffix((char *) file_name);
  bool  matches   = (strcmp(theSuffix, suffix) == 0);

  return matches;
}

char* lc3_replace_suffix (char* file_name, char* new_suffix) {
  char* theSuffix = lc3_get_suffix(file_name);
  int   len       = theSuffix - file_name;
  int   newLen    = len + strlen(new_suffix);
  char* newName   = malloc(newLen + 1);

  strncpy(newName, file_name, len);  
  strcpy(newName + len, new_suffix);

  return newName;
}

void lc3_set_obj_file_mode (const char* name) {
  inHex = lc3_file_has_suffix(name, ".hex");
}

int lc3_read_LC3_word (FILE *f) {
  int word = -1;

  if (inHex) {
    int bit16;
   
    if (fscanf(f, "%x\n", &bit16) != EOF)
      word = bit16;
  }
  else { /* FIXME: just htons... */
    unsigned char buf[2];

    if (fread (buf, 2, 1, f) == 1)
      word = (buf[0] << 8) | buf[1];
  }

  return word;
}

void lc3_write_LC3_word (FILE* f, int val) {
  val &= 0xFFFF; // make sure it is only 16 bits

  if (inHex) {
    fprintf(f, "%04x\n", val);
  }
  else { /* FIXME: just htons... */
    unsigned char out[2];

    out[0] = (val >> 8);
    out[1] = (val & 0xFF);
    fwrite(out, 2, 1, f);
  }
}

char lc3_escaped_char (char c) { // c is character after the backslash '\'
  switch (c) {
    case 'a': c = 0x7; break; // bell
    case 'b': c = 0x8; break; // backspace
    case 'f': c = 0xC; break; // formfeed
    case 'n': c = 0xA; break; // newline
    case 'r': c = 0xD; break; // carriage return
    case 't': c = 0x9; break; // tab
    case 'v': c = 0xB; break; // vertical tab
    default:           break; // handles: \\ \' \" \?
                              // not handled: \nnn \xhh
  }

  return c;
}

bool lc3_get_int (const char* token, int* value) {
  if (! token)
    return false;

  // Try the C style number format [+/-] digit [x|X] digit*
  char* endptr;
  int   num = (int) strtol(token, &endptr, 0);

  if (*endptr == '\0') {
    *value = num;
    return true;
  }

  // Try the LC3 syntax
  // hexadecimal can optionally be preceded by x or X
  // decimal must be preceded by #
  // based on routine parse_addresss() by Steve Lumatta in original lc3sim
  // code from McGraw-Hill

  int negated = 0;

  if (token[0] == '-') {
    token++;
    negated = 1;
  }

  char* fmt;

  if (*token == '#')
    fmt = "#%d";
  else if (tolower(*token) == 'x')
    fmt = "x%x";
  else
    fmt = "%x";

  int count = sscanf(token, fmt, &num);

  if (count == 1) {
    if (negated)
      num = -num;

    *value = num;
    return true;
  }

  if (*token == '\'') { // check for character constant
    int len = strlen(token);
    if (token[len] == '\'') {
    }
  }

  return false; // failed to get value
}

bool lc3_get_address (const char* addr, int* value) {
  /* default matching order: symbol, C-style number, LC3 syntax */

  if (! addr)
    return false;

  symbol_t* label = symbol_find_by_name(lc3_sym_tab, addr);

  if (label) {
    *value = label->addr;
    return true;
  }

  return lc3_get_int(addr, value);
}

void lc3_read_sym_table (FILE* sym_file) {
  if (sym_file && lc3_sym_tab) {
    int adding = 0;
    char buf[100];
    char sym[81];
    int addr;

    while (fgets (buf, 100, sym_file) != NULL) {
      if (adding) {
        if (sscanf (buf, "%*s%80s%x", sym, &addr) != 2)
          break;

        symbol_add(lc3_sym_tab, sym, addr);
      }
      else if (   (sscanf (buf, "%*s%*s%80s", sym) == 1)
            && (strcmp (sym, "------------") == 0)) {
          adding = 1;
      }
    }
  }
}

/** callback function for symbol_interate() */
static void write_one_symbol (symbol_t *sym, void* data) {
   FILE* f = data;
   fprintf(f, "//\t%-16s  %04X\n", sym->name, sym->addr);
}

void lc3_write_sym_table (FILE* sym_file) {
  if (sym_file && lc3_sym_tab) {
    fprintf(sym_file, "// Symbol table\n");
    fprintf(sym_file, "// Scope level 0:\n");
    fprintf(sym_file, "//\tSymbol Name       Page Address\n");
    fprintf(sym_file, "//\t----------------  ------------\n");
    symbol_iterate(lc3_sym_tab, write_one_symbol, sym_file);
  }
}

/** Map to allow printing "name" of opcode rather than number */
static char* opcode_names[] = {
  "OP_INVALID",
  "OP_BR",
  "OP_ADD",
  "OP_LD",
  "OP_ST",
  "OP_JSR_JSRR",
  "OP_AND",
  "OP_LDR",
  "OP_STR",
  "OP_RTI",
  "OP_NOT",
  "OP_LDI",
  "OP_STI",
  "OP_JMP_RET",
  "OP_RESERVED",
  "OP_LEA",
  "OP_TRAP",
  "OP_ORIG",
  "OP_END",
  "OP_BLKW",
  "OP_FILL",
  "OP_STRINGZ",
  "OP_GETC",
  "OP_OUT",
  "OP_PUTS",
  "OP_IN",
  "OP_PUTSP",
  "OP_HALT",
  "OP_GETS",
  "OP_ZERO",
  "OP_COPY",
  "OP_SUB"
};
						  
const char* lc3_get_opcode_name (opcode_t op) {
  return (((op >= OP_INVALID) && (op < NUM_OPCODES)) ? opcode_names[op +1] : NULL);
} 

/** convert operand type to a "name" */
const char* lc3_get_operand_name (operand_t operand) {
  switch (operand) {
    case OPN_DR:    return "DR or SR";
    case OPN_SR1:   return "SR1 or BaseR";
    case OPN_SR2:   return "SR2";
    case OPN_CC:    return "condition codes";
    case OPN_IMM5:  return "imm5";
    case OPN_OFF6:  return "offset6";
    case OPN_VEC8:  return "trapvect8";
    case OPN_PCO9:  return "label (or address for imm9)";
    case OPN_PCO11: return "label (or address for imm11)";
    case OPN_IMM16: return "val for .ORIG/.BLKW";
    case OPN_STR:   return "string literal";
    case OPN_FILL:  return "val for .FILL";
    default:        return "unknown operand";
  }
}

const char* lc3_get_format_name (format_t format) {
  switch (format) {
    case FMT_:     return "no operands";
    case FMT_RRR:  return "reg,reg,reg";
    case FMT_RRI5: return "reg,reg,imm5";
    case FMT_CL:   return "label for BR";
    case FMT_R1:   return "DR or SR";
    case FMT_R2:   return "SR1 or baseR";
    case FMT_I11:  return "label for JSR";
    case FMT_RL:   return "reg,label for LD/ST";
    case FMT_RRI6: return "reg,reg,offset6";
    case FMT_RR:   return "reg,reg";
    case FMT_V:    return "trapvect8";
    case FMT_A:    return " ascii char";
    case FMT_16:   return "16 bits for .ORIG/.BLKW";
    case FMT_S:    return "string literal";
    case FMT_FILL: return ".FILL value";
    default: return "unknown type";
  }
}
