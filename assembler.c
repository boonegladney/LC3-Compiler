/** @file assembler.c
 *  @brief You will modify this file and implement the assembler.h interface
 *  @details This is a implementation of the interface that you will write
 *  for the assignment. Although there are only four functions defined
 *  by the interface, it is likely that you will add many helper functions
 *  to perform specific tasks.
 *  <p>
 *  @author <b>your name here</b>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <assert.h>

#include "assembler.h"
#include "lc3.h"
#include "symbol.h"
#include "tokens.h"
#include "util.h"

/** Global variable containing the current line number in the source file*/
static int srcLineNum;

/** Global variable containing the current LC3 address */
static int currAddr;

// do not modify
line_info_t* asm_init_line_info (line_info_t* info) {
  if (! info)
    info = malloc(sizeof(line_info_t));

  info->next        = NULL;
  info->lineNum     = 0;
  info->address     = 0;
  info->opcode      = OP_INVALID;
  info->format      = NULL;
  info->DR          = -1;
  info->SR1         = -1;
  info->SR2         = -1;
  info->immediate   = 0;
  info->reference   = NULL;

  return info;
}

// do not modify
void asm_print_line_info (line_info_t* info) {
  if (info) {
    fprintf(stderr, "%3d: addr: x%04x op: %s DR:%3d, SR1:%3d SR2:%3d imm: %d ref: %s\n",
           info->lineNum, info->address, lc3_get_opcode_name(info->opcode), info->DR,
           info->SR1, info->SR2, info->immediate, info->reference);
  }
}

// do not modify
/* based on code from http://www.eskimo.com/~scs/cclass/int/sx11c.html */
void asm_error (const char* msg, ...) {
  numErrors++;
  va_list argp;
  fprintf(stderr, "ERROR %3d: ", srcLineNum);
  va_start(argp, msg);
  vfprintf(stderr, msg, argp);
  va_end(argp);
  fprintf(stderr, "\n");
}

/** @todo implement this function */
void asm_init (void) {
	srcLineNum = 0;
	currAddr = 0;
}

void createFormat(format_t operands, line_info_t* lineInfo)
{
	lineInfo->format = malloc(sizeof(inst_format_t));
	switch(lineInfo->opcode)
	{
		  case OP_BR:
				lineInfo->format->name = "BR";
				lineInfo->format->operands = FMT_CL;
				lineInfo->format->prototype = 0;
		  case OP_AND:
				lineInfo->format->name = "AND";
				if(operands == FMT_RRR)
				{
					lineInfo->format->operands = FMT_RRR;
					lineInfo->format->prototype = 20480;
				}
				else if(operands ==FMT_RRI5)
				{
					lineInfo->format->operands = FMT_RRI5;
					lineInfo->format->prototype = 20512;
				}
				break;
		  case OP_ADD:
				lineInfo->format->name = "ADD";
				if(operands == FMT_RRR)
				{
					lineInfo->format->operands = FMT_RRR;
					lineInfo->format->prototype = 4096;
				}
				else if(operands ==FMT_RRI5)
				{
					lineInfo->format->operands = FMT_RRI5;
					lineInfo->format->prototype = 4128;
				}
				break;
		  case OP_STI:
				lineInfo->format->name = "STI";
				lineInfo->format->operands = FMT_RL;
				lineInfo->format->prototype = 45056;
				break;
		  case OP_LDI:
				lineInfo->format->name = "LDI";
				lineInfo->format->operands = FMT_RL;
				lineInfo->format->prototype = 40960;
				break;
		  case OP_LEA:
				lineInfo->format->name = "LEA";
				lineInfo->format->operands = FMT_RL;
				lineInfo->format->prototype = 57344;
				break;
		  case OP_LD:
				lineInfo->format->name = "LD";
				lineInfo->format->operands = FMT_RL;
				lineInfo->format->prototype = 8192;
				break;
		  case OP_ST:
				lineInfo->format->name = "ST";
				lineInfo->format->operands = FMT_RL;
				lineInfo->format->prototype = 12288;
				break;
		  case OP_JSR_JSRR:
				if(operands == FMT_I11)
				{
					lineInfo->format->name = "JSR";
					lineInfo->format->operands = FMT_I11;
					lineInfo->format->prototype = 18432;
				}
				if(operands == FMT_R2)
				{
					lineInfo->format->name = "JSRR";
					lineInfo->format->operands = FMT_R2;
					lineInfo->format->prototype = 16384;
				}
				break;
		  case OP_LDR:
				lineInfo->format->name = "LDR";
				lineInfo->format->operands = FMT_RRI6;
				lineInfo->format->prototype = 24576;
				break;
		  case OP_STR:
				lineInfo->format->name = "STR";
				lineInfo->format->operands = FMT_RRI6;
				lineInfo->format->prototype = 28672;
				break;
		  case OP_RTI:
				lineInfo->format->name = "RTI";
				lineInfo->format->operands = FMT_;
				lineInfo->format->prototype = 32768;
				break;
		  case OP_NOT:
				lineInfo->format->name = "NOT";
				lineInfo->format->operands = FMT_RR;
				lineInfo->format->prototype = 36927;
				break;
		  case OP_JMP_RET:
				if(operands == FMT_R2)
				{
					lineInfo->format->name = "JMP";
					lineInfo->format->operands = FMT_R2;
					lineInfo->format->prototype = 49152;
				}
				if(operands == FMT_)
				{
					lineInfo->format->name = "RET";
					lineInfo->format->operands = FMT_;
					lineInfo->format->prototype = 49600;
				}
				break;
		  case OP_TRAP:
				lineInfo->format->name = "TRAP";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61440;
				break;
		  case OP_ORIG:
				break;
		  case OP_END:
				break;
		  case OP_BLKW:
				break;
		  case OP_FILL:
				break;
		  case OP_GETC://
				lineInfo->format->name = "GETC";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61472;
				break;
		  case OP_OUT://
				lineInfo->format->name = "OUT";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61473;
				break;
		  case OP_PUTS://
				lineInfo->format->name = "PUTS";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61474;
				break;
		  case OP_IN://
				lineInfo->format->name = "IN";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61475;
				break;
		  case OP_PUTSP://
				lineInfo->format->name = "PUTSP";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61476;
				break;
		  case OP_HALT://
				lineInfo->format->name = "HALT";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61477;
				break;
		  case OP_GETS:
				lineInfo->format->name = "GETS";
				lineInfo->format->operands = FMT_V;
				lineInfo->format->prototype = 61478;
				break;
		  case OP_INVALID: //do not need to handle this and ops below.
		  case OP_RESERVED:
		  case OP_STRINGZ:
		  case OP_NEWLN:
		  case OP_ZERO:
		  case OP_COPY:
		  case OP_SETCC:
		  case OP_SUB:
		  case NUM_OPCODES:
				break;
	}
}
//only valid inputs for shouldBe is "comma", "reg", "regimm", and "null"
void operandCheck(const char* shouldBe, const char* isA)
{
	if(isA == NULL)
	{
		asm_error(ERR_MISSING_OPERAND);
	}
	else if(strcmp(shouldBe, "comma") == 0)
	{
		if(strcmp(isA, ",") != 0)
			asm_error(ERR_EXPECTED_COMMA, isA);
	}
	else if(strcmp(shouldBe, "reg") == 0)
	{
		if(util_get_reg(isA) == -1)
			asm_error(ERR_EXPECTED_REG, isA);
	}
	else if(strcmp(shouldBe, "regimm") == 0)
	{
		int error = 0;
		if((util_get_reg(isA) == -1) && lc3_get_int(isA, &error) == false)
			asm_error(ERR_EXPECT_REG_IMM, isA);
	}
}

int get_offset(int currAddress, int offsetAddress, int size, char* label)
{
	int offset = 0;
	currAddress++;
	offset = (offsetAddress - currAddress);
	if(size == 9)
	{
		if((offset > 256) || (offset < -257))
			asm_error(ERR_BAD_PCOFFSET, label);
	}
	else if(size == 11)
	{
		if((offset > 1024) || (offset < -1025))
			asm_error(ERR_BAD_PCOFFSET, label);
	}
	return offset;
}

/** @todo implement this function */
line_info_t* asm_pass_one (const char* asm_file_name,
                           const char* sym_file_name) {	
	//error handling 'booleans'
	int hasReadOrigin = 0;
	int hasReadEnd = 0;
	//open asm file for readying and report any errors and initialize symbol table
	lc3_sym_tab = symbol_init(SYM_TAB_SIZE, 0); // needs subsequent call to term to free memory
	FILE *file = fopen(asm_file_name, "r");
	if(file == NULL)
	{
		asm_error(ERR_OPEN_READ, asm_file_name);
		return NULL;
	}
	
	//initialize head of linked list and copy it into general node
	line_info_t* lineInfoHead = NULL;
	line_info_t* lineInfoPrev = NULL;
	line_info_t* lineInfo;
	
	//begin loop to read in lines
	
	char line[MAX_LINE_LENGTH];
	while(fgets(line, MAX_LINE_LENGTH, file)!=NULL)
	{
		srcLineNum++;
		//initialize lineInfo
		lineInfo = asm_init_line_info(NULL);
		lineInfo->lineNum = srcLineNum;
		lineInfo->address = currAddr;
		
		//tokenize line and store first line(return value of function) in a const char*)
		tokens_init(); //needs a subsequent call to tokens_term()
		const char* token = tokenize_lc3_line(line);
		if(token == NULL)
		{
			free(lineInfo);
			continue;
		}
		//const char* label = NULL;
		
		//if token is a label, add it to the symbol table (need to initialize symTab in asm_init)
		opcode_t opcode = util_get_opcode(token);
		
		if(opcode == -1) // if it's not an opcode, then its a label
		{
			const char* label = NULL;
			label = token;
			if(util_is_valid_label(token) == 0)
				asm_error(ERR_BAD_LABEL, label);
			if(symbol_add(lc3_sym_tab, label, currAddr) == 0)
				asm_error(ERR_DUPLICATE_LABEL, label);
			//label = token;
			token = next_token();
			if(token == NULL)
			{
				free(lineInfo);
				continue;
			}
			opcode = util_get_opcode(token);
		}
		
		//now, either the line had no label, or the label was stored in symTab and we have moved on to the opcode
		switch(opcode){
			//switches on whatever 'opcode' is stored in opcode and based on which
			//'opcode' is in the line it stores the information based on the remainder of the line
			//(i.e. the operands)
			case OP_INVALID :      /**< invalid opcode                                       */
					asm_error(ERR_MISSING_OP);
					break;
			case OP_BR :           /**< PC = PCi + PCoffset9 if condition is met             */
					lineInfo->opcode = opcode;
					createFormat(FMT_CL, lineInfo);
					int count = 0;
					if(token[2] == 0)
					{
						lineInfo->DR = 7;
						token = next_token();//this should be our label
						lineInfo->reference = strdup(token);
					}
					else if(token[3] == 0)
					{
						if(token[2] == 'n' || token[2] == 'N')
						{
							lineInfo->DR = 4;
							token = next_token();//this should be our label
							lineInfo->reference = strdup(token);
						}
						else if(token[2] == 'z' || token[2] == 'Z')
						{
							lineInfo->DR = 2;
							token = next_token();//this should be our label
						lineInfo->reference = strdup(token);
						}
						else if(token[2] == 'p' || token[2] == 'P')
						{
							lineInfo->DR = 1;
							token = next_token();//this should be our label
						lineInfo->reference = strdup(token);
						}
						else
						{
							//throw error?
						}
					}
					else if(token[4] == 0)
					{
						if(token[2] == 'n' || token[2] == 'N')
						{
							count = count + 4;
						}
						else if(token[2] == 'z' || token[2] == 'Z')
						{
							count = count + 2;
						}
						else if(token[2] == 'p' || token[2] == 'P')
						{
							count = count + 1;
						}
						if(token[3] == 'n' || token[3] == 'N')
						{
							count = count + 4;
						}
						else if(token[3] == 'z' || token[3] == 'Z')
						{
							count = count + 2;
						}
						else if(token[3] == 'p' || token[3] == 'P')
						{
							count = count + 1;
						}
						lineInfo->DR = count;
						token = next_token();//this should be our label
						lineInfo->reference = strdup(token);
						
					}
					else
					{
						lineInfo->DR = 7;
						token = next_token();//this should be our label
						lineInfo->reference = strdup(token);
					}
					currAddr++;
					break;
			case OP_AND :          /**< DR = SR1 & SR2 or DR = SR1 & imm5                    */
					//SAME AS ADD SO WE CONTINUE TO ADD BLOCK
			case OP_ADD :          /**< DR = SR1 + SR2 or DR = SR1 + imm5                    */
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					//need to read in DR and SR1 and (SR2 OR an immediate)
					token = next_token(); // this should be a register DR
					lineInfo->DR = util_get_reg(token);
					token = next_token(); operandCheck("comma", token);//this should be a comma
					token = next_token(); operandCheck("reg", token);//this should be a register SR1
					lineInfo->SR1 = util_get_reg(token);
					token = next_token(); operandCheck("comma", token);//this should be a comma
					token = next_token(); operandCheck("regimm", token);//this should be either a register SR2 or an immediate
					if(util_get_reg(token)!= -1)//its a register
					{
						lineInfo->SR2 = util_get_reg(token);
						createFormat(FMT_RRR, lineInfo);
					}
					else if(lc3_get_int(token, &(lineInfo->immediate)))//its an immediate
					{
						createFormat(FMT_RRI5, lineInfo);
					}
					else
					{
						//throw error
					}
					currAddr++;
					break;
			case OP_STI :          /**< mem[mem[PCi + offset9]] = SR                         */
					//SAME AS ST BLOCK SO WE CONTINUE ON TO ST BLOCK
			case OP_LDI :          /**< DR = mem[mem[PCi + PCoffset9]]                       */
					//SAME AS ST BLOCK SO WE CONTINUE ON TO ST BLOCK
			case OP_LEA :          /**< DR = PCi + PCoffset9                                 */
					//SAME AS ST BLOCK SO WE CONTINUE ON TO ST BLOCK
			case OP_LD :           /**< DR = mem[PCi + PCoffset9]                            */
					//SAME AS ST BLOCK SO WE CONTINUE ON TO ST BLOCK
			case OP_ST :           /**< mem[PCi + PCoffset9] = SR                            */
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					createFormat(FMT_RL, lineInfo);
					//need to read in SR and PCoffset9
					token = next_token(); operandCheck("reg", token);//this should be a register SR
					lineInfo->DR = util_get_reg(token);
					token = next_token(); operandCheck("comma", token);//this hould be a comma
					token = next_token(); //this should be label that gives pcoffset
					lineInfo->reference = strdup(token);
					token = next_token(); //this should be NULL
					currAddr++;
					break;
			case OP_JSR_JSRR :     /**< R7 = PCi and (PC = SR or PC = PCi + PCoffest9)       */
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					//need to read in label if JSR or BaseR into SR1 if JSRR
					token = next_token(); //this should be a baseR or a label
					if(util_get_reg(token) != -1)
					{
						operandCheck("reg", token);
						lineInfo->SR1 = util_get_reg(token);
						createFormat(FMT_R2, lineInfo);
					}
					else
					{
						lineInfo->reference = strdup(token);
						createFormat(FMT_I11, lineInfo);
					}
					currAddr++;
					break;						
			case OP_LDR :          /**< DR = mem[BaseR + offset6]                            */
					//UNFINISHED
					//lineNum already initialized to correct value
					//THIS IS THE SAME AS STR SO THIS CONTINUE ONTO THE STR BLOCK INSTEAD
			case OP_STR :          /**< mem[BaseR + offset6] = SR                            */
					//UNFINISHED
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					createFormat(FMT_RRI6, lineInfo);
					//need to read in DR and BaseR and an offset 6 (immediate)
					token = next_token(); operandCheck("reg", token);//should be a register DR
					lineInfo->DR = util_get_reg(token);
					token = next_token(); operandCheck("comma", token);//should be a comma
					token = next_token(); operandCheck("reg", token);//should be baseR
					lineInfo->SR1 = util_get_reg(token);
					token = next_token(); operandCheck("comma", token);//should be a comma
					token = next_token(); //should be an offset 6 immediate
					lc3_get_int(token, &(lineInfo->immediate));
					currAddr++;
					break;
						
			case OP_RTI :          /**< PC = R7, exit supervisor mode                        */
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					createFormat(FMT_, lineInfo);
					//doesn't take any operands
					token = next_token(); //this should be NULL
					currAddr++;
					break;
			case OP_NOT :          /**< DR = ~SR1                                            */
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					createFormat(FMT_RR, lineInfo);
					//need to read in DR and SR1
					token = next_token(); operandCheck("reg", token);
					lineInfo->DR = util_get_reg(token);
					token = next_token(); operandCheck("comma", token);//this should be a comma
					token = next_token(); operandCheck("reg", token);//this should be the SR
					lineInfo->SR1 = util_get_reg(token);
					currAddr++;
					break;
			case OP_JMP_RET :      /**< PC = R7 (RET) or PC = Rx (JMP Rx)                    */
					//lineNum already initialized to correct value
					//address already initialized to correct value
					lineInfo->opcode = opcode; //initialize opcode
					//if there is a register operand we need to store it
					token = next_token();
					if(token!=NULL)
					{
						operandCheck("reg", token);
						lineInfo->SR1 = util_get_reg(token);
						createFormat(FMT_R2, lineInfo);
					}
					else
						createFormat(FMT_, lineInfo);
					currAddr++;
					break;
			case OP_RESERVED :     /**< Currently not used                                   */
					break;						
			case OP_TRAP :         /**< R7 = PCi, PC = mem[mem[trap]], enter supervisor mode */
					lineInfo->opcode = opcode; //initialize opcode
					createFormat(FMT_V, lineInfo);
					token = next_token();
					lc3_get_int(token, &(lineInfo->immediate));
					currAddr++;
					break;
			case OP_ORIG :         /**< memory location where code is loaded                 */
					hasReadOrigin++;
					lineInfo->opcode = opcode;
					token = next_token(); //should be a hex address
					lc3_get_int(token, &currAddr);
					lc3_get_int(token, &(lineInfo->immediate));
					break;
			case OP_END :          /**< end of propgram - only comments may follow           */
					hasReadEnd++;
					break;
			case OP_BLKW :         /**< allocate N words of storage initialized with 0       */
					lineInfo->opcode = opcode; //initialize opcode
					//need to store number of words of storage needed in immediate and increment currAddr by that number
					token = next_token();
					lc3_get_int(token, &(lineInfo->immediate));
					currAddr = currAddr + lineInfo->immediate;
			case OP_FILL :         /**< allocate 1 word of storage initialed with operand    */
					lineInfo->opcode = opcode; //initialize opcode
					//need to get operand which is going to be stored in immediate as a number
					token = next_token();
					lc3_get_int(token, &(lineInfo->immediate));
					currAddr++;
					break;
			case OP_STRINGZ :      /**< allocate N+1 words of storage initialized with
										string and null terminator (1 char per word)     */
					break;
			case OP_GETC :         /**< Read character from keyboard, no echo      (trap x20)*/
			case OP_OUT :          /**< Write one character                        (trap x21)*/
			case OP_PUTS :         /**< Write null terminated string               (trap x22)*/
			case OP_IN :           /**< Print prompt and read/echo character       (trap x23)*/
			case OP_PUTSP :        /**< Write packed null terminated string        (trap x24)*/
			case OP_HALT :         /**< Halt execution                             (trap x25)*/
			case OP_GETS :         /**< Read string from keyboard, store in memory (trap x26)*/
					lineInfo->opcode = opcode; //initialize opcode
					createFormat(FMT_V, lineInfo);
					currAddr++;
					break;
			default:
					break;
		}
		tokens_term();
		if(opcode == OP_END)
		{

			free(lineInfo);
			break;
		}
		if(lineInfoHead == NULL)
		{
			lineInfoHead = lineInfo;
		}
		else if(lineInfoPrev == NULL)
		{
			lineInfoPrev = lineInfo;
			lineInfoHead->next = lineInfo;
		}
		else
		{
			lineInfoPrev->next = lineInfo;
			lineInfoPrev = lineInfo;
		}
	}
	//open sym file for writing
	FILE *symfile = fopen(sym_file_name, "w");
	if(symfile == NULL)
	{
		asm_error(ERR_OPEN_WRITE, sym_file_name);
	}
	//write symfile
	lc3_write_sym_table(symfile);
	fclose(file);
	fclose(symfile);
	//error handling
	if(hasReadOrigin == 0)
		asm_error(ERR_NO_ORIG);
	if(hasReadOrigin > 1)
		asm_error(ERR_MULTIPLE_ORIG);
	if(hasReadEnd == 0)
		asm_error(ERR_NO_END);
	if(lineInfoHead->opcode != OP_ORIG)
		asm_error(ERR_ORIG_NOT_1ST);
	return lineInfoHead;
	
	
	
	//return NULL;
}

/** @todo implement this function */
void asm_pass_two (const char* obj_file_name, line_info_t* list) {
	FILE *objectFile = fopen(obj_file_name, "w");
	if(objectFile == NULL)
	{
		asm_error(ERR_OPEN_WRITE, obj_file_name);
	}
	lc3_set_obj_file_mode(obj_file_name);
	while(list != NULL)	
	{
		//print line_info linked list
		asm_print_line_info(list);
		//open file for writing
		
		int word = 0;
		if((list->opcode != OP_ORIG) &&
			(list->opcode != OP_FILL) &&
			(list->opcode != OP_BLKW))
		{
			word = list->format->prototype; //places protoype in word
		}
		int pc_offset = 0;
		int DR = 0;
		int SR1 = 0;
		int SR2 = 0;
		int imm_mask_5 = 31;
		int offset_mask_6 = 63;
		int offset_mask_9 = 511;
		int trapvect_mask_8 = 255;
		int offset_mask_11 = 2047;
		//switch statement to handle any possible opcode
		switch(list->opcode)
		{
			case OP_INVALID :      /**< invalid opcode                                       */
					break;
			case OP_BR :           /**< PC = PCi + PCoffset9 if condition is met             */
					//offset9
					if(symbol_find_by_name(lc3_sym_tab, list->reference) == NULL)
					{
						asm_error(ERR_MISSING_LABEL, list->reference);
					}
					else
					{
					lc3_get_address(list->reference, &pc_offset); //gets address of BR reference
					pc_offset = get_offset(list->address, pc_offset, 9, list->reference); //calculates pc_offset
					pc_offset = (pc_offset & offset_mask_9); //masks offset to fit into word
					word = (pc_offset | word); //masks offset field into word
					}
					//conditional
					int conditional = list->DR;
					conditional = (conditional << 9);
					word = (word | conditional);
					//write word
					break;
			case OP_AND :          /**< DR = SR1 & SR2 or DR = SR1 & imm5                    */
					//same as ADD so we continue to ADD block
			case OP_ADD :          /**< DR = SR1 + SR2 or DR = SR1 + imm5                    */
					
					//DR
					DR = list->DR;
					DR = (DR<<9);
					word = (word | DR);
					//SR1
					SR1 = list->SR1;
					SR1 = (SR1<<6);
					word = (word | SR1);
					//imm5 or SR2
					if(list->format->operands == FMT_RRI5) //its in format FMT_RRI5 and need to handle imm5
					{
						imm_mask_5 = (imm_mask_5 & list->immediate);
						word = (word | imm_mask_5);
					}
					else //its in format FMT_RRR and need to handle SR2
					{
						SR2 = list->SR2;
						word = (word | SR2);
					}
					break;
					
			case OP_STI :          /**< mem[mem[PCi + offset9]] = SR                         */
			case OP_LDI :          /**< DR = mem[mem[PCi + PCoffset9]]                       */
			case OP_LEA :          /**< DR = PCi + PCoffset9                                 */
			case OP_LD :           /**< DR = mem[PCi + PCoffset9]                            */
			case OP_ST :           /**< mem[PCi + PCoffset9] = SR                            */
					
					//SR/DR
					DR = list->DR;
					DR = (DR<<9);
					word = (word | DR);
					//pcoffset 9
					if(symbol_find_by_name(lc3_sym_tab, list->reference) == NULL)
					{
						asm_error(ERR_MISSING_LABEL, list->reference);
					}
					else
					{
					lc3_get_address(list->reference, &pc_offset); //gets adress of BR reference
					pc_offset = get_offset(list->address, pc_offset, 9, list->reference); //calculates pc_offset
					pc_offset = (pc_offset & offset_mask_9); //masks offset to fit into word
					word = (pc_offset | word); //masks offset field into word
					}
					//
					break;
					
			case OP_JSR_JSRR :     /**< R7 = PCi and (PC = SR or PC = PCi + PCoffest9)       */
					
					if(list->format->operands == FMT_R2) //SR1
					{
						SR1 = list->SR1;
						SR1 = (SR1<<6);
						word = (word | SR1);
					}
					else //PCoffset11
					{
						if(symbol_find_by_name(lc3_sym_tab, list->reference) == NULL)
						{
							asm_error(ERR_MISSING_LABEL, list->reference);
						}
						else
						{
						lc3_get_address(list->reference, &pc_offset); //gets adress of BR reference
						pc_offset = get_offset(list->address, pc_offset, 11, list->reference); //calculates pc_offset
						pc_offset = (pc_offset & offset_mask_11); //masks offset to fit into word
						word = (pc_offset | word); //masks offset field into word
						}
					}
					//
					break;
					
			case OP_LDR :          /**< DR = mem[BaseR + offset6]                            */
			case OP_STR :          /**< mem[BaseR + offset6] = SR                            */
					
					//DR
					DR = list->DR;
					DR = (DR<<9);
					word = (word | DR);
					//baseR/SR1
					SR1 = list->SR1;
					SR1 = (SR1<<6);
					word = (word | SR1);
					//offset6
					offset_mask_6 = (offset_mask_6 & list->immediate);
					word = (word | offset_mask_6);
					//
					break;
					
			case OP_RTI :          /**< PC = R7, exit supervisor mode                        */
					//nothing to be done
					break;
			case OP_NOT :          /**< DR = ~SR1                                            */
					
					//DR
					DR = list->DR;
					DR = (DR<<9);
					word = (word | DR);
					//SR/SR1
					SR1 = list->SR1;
					SR1 = (SR1<<6);
					word = (word | SR1);
					//
					break;
					
			case OP_JMP_RET :      /**< PC = R7 (RET) or PC = Rx (JMP Rx)                    */
					
					if(list->format->operands == FMT_R2)
					{
						//baseR/SR1
						SR1 = list->SR1;
						SR1 = (SR1<<6);
						word = (word | SR1);
					}
					//
					break;
					
			case OP_RESERVED :     /**< Currently not used                                   */
					break;
			case OP_TRAP :         /**< R7 = PCi, PC = mem[mem[trap]], enter supervisor mode */
					
					//trapvect8
					trapvect_mask_8 = (trapvect_mask_8 & list->immediate);
					word = (word | trapvect_mask_8);
					//
					break;
					
			case OP_ORIG :         /**< memory location where code is loaded                 */
					word = list->immediate;
					break;
			case OP_END :          /**< end of propgram - only comments may follow           */
					break;
			case OP_BLKW :         /**< allocate N words of storage initialized with 0       */
					for(int i = 1; i < list->immediate; i++)
					{
						lc3_write_LC3_word(objectFile, word); //write to .hex file
					}
					break;
			case OP_FILL :         /**< allocate 1 word of storage initialed with operand    */
					word = list->immediate;
					break;
			case OP_STRINGZ :      /**< allocate N+1 words of storage initialized with       
											string and null terminator (1 char per word)     */
					break;
			case OP_GETC :         /**< Read character from keyboard, no echo      (trap x20)*/
			case OP_OUT :          /**< Write one character                        (trap x21)*/
			case OP_PUTS :         /**< Write null terminated string               (trap x22)*/
			case OP_IN :           /**< Print prompt and read/echo character       (trap x23)*/
			case OP_PUTSP :        /**< Write packed null terminated string        (trap x24)*/
			case OP_HALT :         /**< Halt execution                             (trap x25)*/
			case OP_GETS :         /**< Read string from keyboard, store in memory (trap x26)*/
					break;
			default:
					break;
		}
		lc3_write_LC3_word(objectFile, word); //write to .hex file
		//get next element in list
		list = list->next;
	}
	fclose(objectFile);
}

/** @todo implement this function */
void asm_term (line_info_t* list) {
	while(list != NULL)
	{
		line_info_t* next = list->next;
		if(list->reference!=NULL)
			free(list->reference);
		free(list->format);
		free(list);
		list = next;
	}
	symbol_term(lc3_sym_tab);
}
