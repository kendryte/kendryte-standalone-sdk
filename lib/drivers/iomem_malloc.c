#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "iomem_malloc.h"

#define IOMEM_BLOCK_SIZE 512

typedef struct _iomem_malloc_t
{
    void (*init)();
    uint8_t (*perused)();
    uint8_t *membase;
    uint32_t memsize;
    uint32_t memtblsize;
    uint16_t *memmap;
    uint8_t  memrdy;
} iomem_malloc_t;

static void iomem_init();
static uint8_t iomem_perused();

iomem_malloc_t malloc_cortol = 
{
    iomem_init,
    iomem_perused,
    NULL,
    0,
    0,
    NULL,
    0
};

static void iomem_set(void *s, uint8_t c, uint32_t num)
{
    uint8_t *xs = s;
    while(num--)
        *xs++=c;
}

static void iomem_init()
{
    extern unsigned int _eiodata;
    malloc_cortol.membase = (uint8_t *)((uintptr_t)&_eiodata);
    malloc_cortol.memsize = 0x40600000 - (uintptr_t)&_eiodata;
    malloc_cortol.memtblsize = malloc_cortol.memsize / IOMEM_BLOCK_SIZE;
    malloc_cortol.memmap = (uint16_t *)malloc(malloc_cortol.memtblsize * 2);
    printf("&_eiodata = %p \n", &_eiodata);
    iomem_set(malloc_cortol.memmap, 0, malloc_cortol.memtblsize * 2);
    iomem_set(malloc_cortol.membase, 0, malloc_cortol.memsize);
    malloc_cortol.memrdy = 1;
}

static uint8_t iomem_perused()
{
    uint32_t used=0;
    uint32_t i;
    for(i = 0;i < malloc_cortol.memtblsize; i++)
    {
        if(malloc_cortol.memmap[i])
            used++;
    }
    return (used * 100) / (malloc_cortol.memtblsize);
}

static uint32_t k_malloc(uint32_t size)
{
    signed long offset = 0;
    uint32_t xmemb;
    uint32_t kmemb = 0;
    uint32_t i;
    if(!malloc_cortol.memrdy)
        malloc_cortol.init();
    if(size==0)
        return 0XFFFFFFFF;
    xmemb=size / IOMEM_BLOCK_SIZE;
    if(size % IOMEM_BLOCK_SIZE)
        xmemb++;
    for(offset=malloc_cortol.memtblsize-1; offset>=0; offset--)
    {
        if(!malloc_cortol.memmap[offset])
            kmemb++;
        else 
            kmemb=0;
        if(kmemb==xmemb)
        {
            for(i=0; i<xmemb; i++)
            {
                malloc_cortol.memmap[offset+i] = xmemb;
            }
            return (offset * IOMEM_BLOCK_SIZE);
        }
    }
    return 0XFFFFFFFF;
}

static uint8_t k_free(uint32_t offset)
{
    int i;
    if(!malloc_cortol.memrdy)
    {
        malloc_cortol.init();
        return 1;
    }  
    if(offset < malloc_cortol.memsize)
    {  
        int index=offset / IOMEM_BLOCK_SIZE;
        int nmemb=malloc_cortol.memmap[index];
        for(i=0; i<nmemb; i++)
        {  
            malloc_cortol.memmap[index+i] = 0;
        }  
        return 0;
    }
    else 
        return 2;
}  

void iomem_free(void *paddr)
{
    uint32_t offset;
    if(paddr == NULL)
        return;
    offset=(uintptr_t)paddr - (uintptr_t)malloc_cortol.membase;
    k_free(offset);
}

void *iomem_malloc(uint32_t size)
{
    uint32_t offset;
    offset=k_malloc(size);
    if(offset == 0XFFFFFFFF)
        return NULL;
    else 
        return (void*)((uintptr_t)malloc_cortol.membase + offset);
}

