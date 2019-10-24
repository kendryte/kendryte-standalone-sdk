#ifndef _IOMEM_MALLOC_H
#define _IOMEM_MALLOC_H

void iomem_free(void *paddr) ;
void *iomem_malloc(uint32_t size);
uint32_t iomem_unused();

#endif
