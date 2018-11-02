#include "CodeSource.h"
#include "CodeObject.h"
#include "CFG.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include "Operation_impl.h"
using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

int main(int argc, char **argv) {
    
	  FILE *f1, *f2, *f3, *f4;
	  if(argc != 4) {
		    printf("format: test [hexcode_FILE] [opcode_FILE] [binary_FILE] [output_FILE]");
				fflush(stdout);
				return 0;
		}
		else {
				f1 = fopen(argv[1], "r");
				f2 = fopen(argv[2], "r");
				f3 = fopen(argv[3], "r");
		}
	
		f4 = fopen("test_result.txt", "w");

		char number[20], buffer[100], name[20], binary[100];
		uint8_t hexnum[4];
		uint32_t hex, temp;
		int third_level_count = 0, count = 0, total = 0;
	 	int opcode_n;
		while(fscanf(f1, "%x", &temp) != EOF) {
				hex = temp;
				if(!fscanf(f2, "%s", name)) {
						printf("error in name file\n");
						return 0;
				}

				//skip the name
				if(!fgets(binary, 100, f3)) {
						printf("error in binary file\n");
						return 0;
				}
				if(!fgets(buffer, 100, f3)) {
						printf("error in binary file\n");
						return 0;
				}
				
				opcode_n = 0;
				for(int i = 0; i < 6; i++) {
						opcode_n *= 2;
						//printf("%c %d\n", binary[i], opcode_n);
						opcode_n += binary[i] - '0';
				}
				
				for(char *c = name; *c != '\0'; c++) {
						if(*c == '[' || *c == '.') {
								*c = '\0';
								break;
						}
				}
				hexnum[0] = temp % 256;
				temp /= 256;
				hexnum[1] = temp % 256;
				temp /= 256;
				hexnum[2] = temp % 256;
				temp /= 256;
				hexnum[3] = temp % 256;
				temp /= 256;
				// Construct an instruction decoder for power instruction
    		InstructionDecoder dec((void*)hexnum, sizeof(hexnum) , Arch_ppc64);
    		// Decode one instruction
    		Instruction insn = dec.decode();
    		// Print the instruction
				char decoded[200];
			 	strcpy(decoded, insn.format().c_str());
				
				for(char *c = decoded; *c != '\0'; c++) {
						if(*c == ' ' || *c == '.') {
								*c = '\0';
								break;
						}
				}

				total++;
				int type = 0;
				//printf("%s\n", decoded);	
				if(strcmp(decoded, name) != 0) {
						//printf("%s not implemented (correctly), output is: %s\n", name, decoded);
						fprintf(f4, "%x ", hex);
						fprintf(f4, "%13s %13s  opcode:%2d    %s", name, decoded, opcode_n, binary);
						
						if(binary[11] != '.')
								type = 1;

						for(int i = 13; i < 18; i++) {
								if(binary[i] != '0' && binary[i] != '1') {
									 type = 1;
									 break;
								}
						}
						if(type == 0){
								third_level_count++;
								fprintf(f4, "type: third level opcode\n");
						}
						else
								fprintf(f4, "\n");
						
						count++;
				}

		}
		printf("total %d tested, %d not implemented, third level: %d, detailed result in test_result.txt\n", total, count, third_level_count);	
		fprintf(f4, "total %d tested, %d not implemented, third level: %d\n", total, count, third_level_count);	
		return 0;
}
