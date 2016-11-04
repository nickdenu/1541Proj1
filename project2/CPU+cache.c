#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

// to keep cache statistics
unsigned int accesses = 0;
unsigned int read_accesses = 0;
unsigned int write_accesses = 0;
unsigned int L1hits = 0;
unsigned int L1misses = 0;
unsigned int L2hits = 0;
unsigned int L2misses = 0;

#include "cache.h" 

int main(int argc, char **argv)
{
    struct trace_item *tr_entry;
    size_t size;
    char *trace_file_name;
    int trace_view_on = 0;
    FILE * config_fd;

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
    if (argc == 3) trace_view_on = atoi(argv[2]) ;
    // TODO
    unsigned int L1size; 
    unsigned int bsize;
    unsigned int L1assoc;
    unsigned int L2size;
    unsigned int L2assoc;
    unsigned int L2_hit_latency;
    unsigned int mem_latency;
	
	config_fd = fopen("cache_config.txt", "r");
	fscanf(config_fd,"%u %u %u %u %u %u %u", &L1size, &bsize, &L1assoc, &L2size, &L2assoc, &L2_hit_latency, &mem_latency);

    fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

    trace_fd = fopen(trace_file_name, "rb");

    if (!trace_fd) {
        fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
        exit(0);
    }

    trace_init();
    struct cache_t *L1, *L2, *nextL;
    L1 = cache_create(L1size, bsize, L1assoc, 0); 
    nextL = NULL;   // the next level in the hierarchy -- NULL indicates main memory
    if (L2size != 0)
    {
        L2 = cache_create(L2size, bsize, L2assoc, L2_hit_latency);
        nextL = L2;
    }

    while(1) {
        size = trace_get_item(&tr_entry);

        if (!size) {       /* no more instructions (trace_items) to simulate */
            printf("+ Simulation terminates at cycle : %u\n", cycle_number);
            printf("+ Cache statistics \n");
            printf("++ Accesses: %d\n", accesses);
            printf("++ Read accesses: %d\n", read_accesses);
            printf("++ Write accesses: %d\n", write_accesses);
            printf("++ L1 hits: %d\n", L1hits);
            printf("++ L1 misses: %d\n", L1misses);
            printf("++ L2 hits: %d\n", L2hits);
            printf("++ L2 misses: %d\n", L2misses);
            break;
        }
        else{              /* parse the next instruction to simulate */
            cycle_number++;
            t_type = tr_entry->type;
            t_sReg_a = tr_entry->sReg_a;
            t_sReg_b = tr_entry->sReg_b;
            t_dReg = tr_entry->dReg;
            t_PC = tr_entry->PC;
            t_Addr = tr_entry->Addr;
        }  

        // SIMULATION OF A SINGLE CYCLE cpu IS TRIVIAL - EACH INSTRUCTION IS EXECUTED
        // IN ONE CYCLE, EXCEPT IF THERE IS A DATA CACHE MISS.

        switch(tr_entry->type) {
            case ti_NOP:
                if (trace_view_on) printf("[cycle %d] NOP:", cycle_number);
                break;
            case ti_RTYPE:
                if (trace_view_on) {
                    printf("[cycle %d] RTYPE:", cycle_number);
                    printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
                };
                break;
            case ti_ITYPE:
                if (trace_view_on){
                    printf("[cycle %d] ITYPE:", cycle_number);
                    printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
                };
                break;
            case ti_LOAD:
                if (trace_view_on){
                    printf("[cycle %d] LOAD:", cycle_number);
                    printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
                };
                accesses++;
                read_accesses++;
                cycle_number += cache_access(L1, tr_entry->Addr, 'r', cycle_number, nextL, 1, mem_latency, 0);
                break;
            case ti_STORE:
                if (trace_view_on){
                    printf("[cycle %d] STORE:", cycle_number);
                    printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
                };
                accesses++;
                write_accesses++;
                cycle_number += cache_access(L1, tr_entry->Addr, 'w', cycle_number, nextL, 1, mem_latency, 0);
                break;
            case ti_BRANCH:
                if (trace_view_on) {
                    printf("[cycle %d] BRANCH:", cycle_number);
                    printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
                };
                break;
            case ti_JTYPE:
                if (trace_view_on) {
                    printf("[cycle %d] JTYPE:", cycle_number);
                    printf(" (PC: %x)(addr: %x)\n", tr_entry->PC, tr_entry->Addr);
                };
                break;
            case ti_SPECIAL:
                if (trace_view_on) printf("[cycle %d] SPECIAL:", cycle_number);
                break;
            case ti_JRTYPE:
                if (trace_view_on) {
                    printf("[cycle %d] JRTYPE:", cycle_number);
                    printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
                };
                break;
        }

    }

    trace_uninit();

    exit(0);
}


