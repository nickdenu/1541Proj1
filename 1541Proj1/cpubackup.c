/**************************************************************/
/* CS/COE 1541				 			
   just compile with gcc -o pipeline pipeline.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

#define ti_SQUASHED 10
#define PIPELINE_SIZE 8
#define REG_UNUSED 255
#define ADDR_UNUSED 0xFFFFFFFF


enum stage_name{
	s_IF1 = 0,
	s_IF2,
	s_ID,
	s_EX1,
	s_EX2,
	s_MEM1,
	s_MEM2,
	s_WB
};

int main(int argc, char **argv)
{
  struct trace_item *tr_entry;
  struct trace_item *tr_out_entry;
  struct trace_item **pipeline_array = (struct trace_item **)malloc(8 * sizeof(struct trace_item *));
  
  for(int i = 0 ; i < PIPELINE_SIZE ; i++){
	  pipeline_array[i] = (struct trace_item *)malloc(sizeof(struct trace_item));
	  pipeline_array[i]->type = ti_NOP;
	  pipeline_array[i]->sReg_a = REG_UNUSED;
	  pipeline_array[i]->sReg_b = REG_UNUSED;
	  pipeline_array[i]->dReg = REG_UNUSED;
	  pipeline_array[i]->PC = ADDR_UNUSED;
	  pipeline_array[i]->Addr = ADDR_UNUSED;
  }
  
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;
  int prediction_method = 0;
  
  unsigned char t_type = 0;
  unsigned char t_sReg_a= 0;
  unsigned char t_sReg_b= 0;
  unsigned char t_dReg= 0;
  unsigned int t_PC = 0;
  unsigned int t_Addr = 0;

  unsigned int cycle_number = 0;

  
  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc >= 3) trace_view_on = atoi(argv[2]) ;
  if (argc >= 4) prediction_method = atoi(argv[3]) ;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();
  int stalled = 0; //Set to false if you don't want new instruction
  int squashed = 0;
  int insertion_point = 0;
  int pipeline_array_count = 0;
  while(1) {
	if(squashed){
		insertion_point = s_EX2;
	}
	else{
		insertion_point = stalled;
	}
	for(int i = PIPELINE_SIZE - 1 ; i > insertion_point ; i--){
		if(i == PIPELINE_SIZE - 1){
			tr_out_entry = pipeline_array[PIPELINE_SIZE - 1];
		}	
		pipeline_array[i]->type = pipeline_array[i-1]->type;
		pipeline_array[i]->sReg_a = pipeline_array[i-1]->sReg_a;
		pipeline_array[i]->sReg_b = pipeline_array[i-1]->sReg_b;
		pipeline_array[i]->dReg = pipeline_array[i-1]->dReg;
		pipeline_array[i]->PC = pipeline_array[i-1]->PC;
		pipeline_array[i]->Addr = pipeline_array[i-1]->Addr;
	}	
	if(squashed){
		pipeline_array[insertion_point]->type = ti_SQUASHED;
		pipeline_array[insertion_point]->sReg_a = REG_UNUSED;
		pipeline_array[insertion_point]->sReg_b = REG_UNUSED;
		pipeline_array[insertion_point]->dReg = REG_UNUSED;
		pipeline_array[insertion_point]->PC = ADDR_UNUSED;
		pipeline_array[insertion_point]->Addr = ADDR_UNUSED;
		squashed--;
	}
	else if(stalled){
		pipeline_array[insertion_point]->type = ti_NOP;
		pipeline_array[insertion_point]->sReg_a = REG_UNUSED;
		pipeline_array[insertion_point]->sReg_b = REG_UNUSED;
		pipeline_array[insertion_point]->dReg = REG_UNUSED;
		pipeline_array[insertion_point]->PC = ADDR_UNUSED;
		pipeline_array[insertion_point]->Addr = ADDR_UNUSED;
	}
	
	if(!stalled && !squashed){
    size = trace_get_item(&tr_entry);
	
		if (!size) {       /* no more instructions (trace_items) to simulate */
		  printf("+ Simulation terminates at cycle : %u\n", cycle_number);
		  break;
		}
		else{              /* parse the next instruction to simulate */
		  cycle_number++;
		  pipeline_array_count++;
		  /*t_type = tr_entry->type;
		  t_sReg_a = tr_entry->sReg_a;
		  t_sReg_b = tr_entry->sReg_b;
		  t_dReg = tr_entry->dReg;
		  t_PC = tr_entry->PC;
		  t_Addr = tr_entry->Addr;*/
		  pipeline_array[0] = tr_entry;
		}  
	}
	else{
		stalled = 0;
		cycle_number++;
	}

// SIMULATION OF A SINGLE CYCLE cpu IS TRIVIAL - EACH INSTRUCTION IS EXECUTED
// IN ONE CYCLE
	printf("[cycle %d] \n", cycle_number);
	for(int i = 0 ; i < PIPELINE_SIZE ; i++){
		switch(i){
			case s_IF1:
				printf("IF1 :\n");
				break;
			case s_IF2:
				printf("IF2 :\n");
				break;
			case s_ID:
				printf("ID :\n");
				break;
			case s_EX1:
				printf("EX1 :\n");
				break;
			case s_EX2:
				printf("EX2 :\n");
				break;
			case s_MEM1:
				printf("MEM1 :\n");
				break;
			case s_MEM2:
				printf("MEM2 :\n");
				break;
			case s_WB:
				printf("WB :\n");
				break;
		}
		switch(pipeline_array[i]->type) {
        case ti_NOP:
          printf("\tNOP:\n") ;
          break;
        case ti_RTYPE:
          printf("\tRTYPE:(PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", pipeline_array[i]->PC, pipeline_array[i]->sReg_a, pipeline_array[i]->sReg_b, pipeline_array[i]->dReg);
          break;
        case ti_ITYPE:
          printf("\tITYPE:(PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", pipeline_array[i]->PC, pipeline_array[i]->sReg_a, pipeline_array[i]->dReg, pipeline_array[i]->Addr);
          break;
        case ti_LOAD:    
          printf("\tLOAD:(PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", pipeline_array[i]->PC, pipeline_array[i]->sReg_a, pipeline_array[i]->dReg, pipeline_array[i]->Addr);
          break;
        case ti_STORE:    
          printf("\tSTORE: (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", pipeline_array[i]->PC, pipeline_array[i]->sReg_a, pipeline_array[i]->sReg_b, pipeline_array[i]->Addr);
          break;
        case ti_BRANCH:
          printf("\tBRANCH: (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", pipeline_array[i]->PC, pipeline_array[i]->sReg_a, pipeline_array[i]->sReg_b, pipeline_array[i]->Addr);
          break;
        case ti_JTYPE:
          printf("\tJTYPE: (PC: %x)(addr: %x)\n", pipeline_array[i]->PC,pipeline_array[i]->Addr);
          break;
        case ti_SPECIAL:
          printf("\tSPECIAL:\n") ;      	
          break;
        case ti_JRTYPE:
          printf("\tJRTYPE: (PC: %x) (sReg_a: %d)(addr: %x)\n", pipeline_array[i]->PC, pipeline_array[i]->dReg, pipeline_array[i]->Addr);
          break;
		case ti_SQUASHED:
		  printf("\tSQUASHED:\n");
      }
	}
	
	
    if (trace_view_on && (tr_out_entry != NULL)) {
		// print the executed instruction if trace_view_on=1 
      switch(tr_out_entry->type) {
        case ti_NOP:
          printf("[cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE:
          printf("[cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %x)(addr: %x)\n", tr_entry->PC,tr_entry->Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:\n",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
          break;
      }
    }
	
	if((pipeline_array[s_WB]->dReg != REG_UNUSED && pipeline_array[s_ID]->sReg_a != REG_UNUSED)
		|| (pipeline_array[s_EX1]->dReg == pipeline_array[s_ID]->sReg_a
		|| pipeline_array[s_EX1]->dReg == pipeline_array[s_ID]->sReg_b)
			&& pipeline_array[s_EX1]->dReg != REG_UNUSED
		|| (pipeline_array[s_EX2]->type == ti_LOAD 
			&& (pipeline_array[s_EX2]->dReg == pipeline_array[s_ID]->sReg_a 
				|| pipeline_array[s_EX2]->dReg == pipeline_array[s_ID]->sReg_b))
		|| (pipeline_array[s_MEM1]->type == ti_LOAD 
			&& (pipeline_array[s_MEM1]->dReg == pipeline_array[s_ID]->sReg_a 
				|| pipeline_array[s_MEM1]->dReg == pipeline_array[s_ID]->sReg_b))){
		stalled = s_EX1;
	}
	
	switch(prediction_method){
		case 0:
			if(pipeline_array[s_EX2]->type == ti_BRANCH && 
				pipeline_array[s_EX2]->Addr == pipeline_array[s_EX1]->PC){
				squashed = s_EX1 + 1;
			}
			break;
	}
	
  }

  trace_uninit();

  exit(0);
}

