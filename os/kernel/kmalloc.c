#include <stddef.h>
#include <stdint.h>

#include "kmalloc.h"

#include "multiboot.h"
#include "kmem.h"
#include "kterm.h"
#include "kstd.h"

//malloc data
//uint8_t* bitmap;
BitmapStatus* bitmap; 
void*    arena;
size_t total_blocks;
size_t max_blocks;

static const uint8_t factor = 8;
static const size_t MAX_HEAP_SIZE = 0x800; //(2^10) * (2^4); //16k

void k_malloc_init(multiboot_info_t* mbd)
{
    //_mbd = mbd;
    
    if(!(mbd->flags & MULTIBOOT_INFO_MEMORY))
    {
	//error
	k_print("[ERROR][k_malloc_init] multiboot flag 0 not set\n");
	return;
    }
    
    //initalise bitmap
    total_blocks = 0;
    max_blocks = MAX_HEAP_SIZE * factor;
    bitmap = (BitmapStatus*)mem_region_start;
    arena  = bitmap + max_blocks;

    for(size_t i = 0; i < max_blocks; i++)
	bitmap[i] = Free;
}

void* kmalloc(size_t size)
{
    size_t base  = 0;
    size_t accum = 0;
    
    for(size_t i = 0; i < max_blocks; i++)
    {
	if(bitmap[i] == Free && accum == 0)
	{
	    base = i;
	    accum++;
	}
	else if (bitmap[i] == Used)
	{
	    accum = 0;
	}
	
	if((accum * factor) >= size)
	{
	    for(size_t j = 0; j < (accum); j++)
		bitmap[base + j] = Used;
		
	    k_print("Alloced ");
	    k_print_hex((size_t)arena + (i * base) * factor);
	    k_print(":");
	    k_print_hex(size);
	    k_print("\n");
	    
	    //total_blocks++;
	    return (void*)(arena + (base * factor));
	}
    }
}

void kfree(void* addr)
{
    size_t bitmap_addr = (size_t)(addr) / factor;
    bitmap[bitmap_addr] = Free;

    k_print("freed ");
    k_print_hex((size_t)addr);
    k_print("\n");

}
