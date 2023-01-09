/**
 * virtmem.c 
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16
#define PAGES 1024
#define PAGE_MASK 1023

#define PAGE_SIZE 1024
#define OFFSET_BITS 10
#define OFFSET_MASK 1023

#define page_frame 256 // As stated in the pdf

#define MEMORY_SIZE page_frame * PAGE_SIZE // changing accordingly 

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

struct tlbentry {
  unsigned int logical;
  unsigned int physical;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];

signed int main_memory[MEMORY_SIZE];

// Pointer to memory mapped backing file
signed int *backing;

int max(int a, int b)
{
  if (a > b)
    return a;
  return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned int logical_page) {
    /* TODO */
      struct tlbentry tlb1;
  for (int i = 0; i < TLB_SIZE; i++) {
    tlb1 = tlb[i];
    if (tlb1.logical == logical_page) {
      return tlb1.physical;
    }
  }
    return -1;
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned int logical, unsigned int physical) {
    /* TODO */
   struct tlbentry tlb1;
  memset(&tlb1, 0, sizeof(tlb1));
  // add values to tlb1 and add to the array
  tlb1.logical = logical;
  tlb1.physical = physical;
  tlb[tlbindex % TLB_SIZE] = tlb1;
  tlbindex++;
}

int policy_select(int argc, const char *argv[]){

if (argc != 5)
{
    printf("Incorrect amount of arguments");
    return -1;
}

if (strcmp(argv[3], "-p") != 0)
{
    printf("missing flag");
    return -1;
}

int p = atoi(argv[4]);

return p;
}

int main(int argc, const char *argv[])
{
 
 int p = policy_select(argc, argv); // initializing the policy number
  
  const char *backing_filename = argv[1]; 
  int backing_fd = open(backing_filename, O_RDONLY);
  backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0); 
  
  const char *input_filename = argv[2];
  FILE *input_fp = fopen(input_filename, "r");
  
  // Fill page table entries with -1 for initially empty table.
  int i;
  for (i = 0; i < PAGES; i++) {
    pagetable[i] = -1;
  }
  
  // Character buffer for reading lines of input file.
  char buffer[BUFFER_SIZE];
  
  // Data we need to keep track of to compute stats at end.
  int total_addresses = 0;
  int tlb_hits = 0;
  int page_faults = 0;
  
  // Number of the next unallocated physical page in main memory
  unsigned int free_page = 0;
  
  while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL) {
    total_addresses++;
    int logical_address = atoi(buffer);

    /* TODO 
    / Calculate the page offset and logical page number from logical_address */
    int offset = logical_address & OFFSET_MASK;
    int logical_page = (logical_address >> OFFSET_BITS) & PAGE_MASK; 
    ///////
    
    int physical_page = search_tlb(logical_page);
    // TLB hit
    if (physical_page != -1) {
      tlb_hits++;
      // TLB miss
    } else {
      physical_page = pagetable[logical_page];
      
      // Page fault
      if (physical_page == -1) {
          /* TODO */
          page_faults++;

          if (p == 0)
          {
            /* FIFO */
             // increase page_faults
          
          // assign physical page to the free page 
          // increase free_page
          free_page++;
          physical_page = (free_page) % (page_frame);
          // bring the page from backing to the main memory
          memcpy(main_memory + physical_page* PAGE_SIZE, backing + logical_page * PAGE_SIZE, PAGE_SIZE);
          // update page table
          pagetable[logical_page] = physical_page;
            
          } else if (p == 1)
          {
            /* LRU */
          }
          
          
      }

      add_to_tlb(logical_page, physical_page);
    }
    
    int physical_address = (physical_page << OFFSET_BITS) | offset;
    signed char value = main_memory[physical_page * PAGE_SIZE + offset];
    
    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, value);
  }
  
  printf("Number of Translated Addresses = %d\n", total_addresses);
  printf("Page Faults = %d\n", page_faults);
  printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hits);
  printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));
  
  return 0;
}