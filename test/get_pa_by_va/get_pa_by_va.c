 
 
#define _FILE_OFFSET_BITS 64
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/file.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#ifdef RTE_EAL_NUMA_AWARE_HUGEPAGES
#include <numa.h>
#include <numaif.h>
#endif
 
 
#define phys_addr_t	uint64_t
static bool phys_addrs_available = true;
typedef uint64_t rte_iova_t;
#define RTE_BAD_IOVA ((rte_iova_t)-1)
#define RTE_LOG(l, t, fmt...) printf(fmt)
#define PFN_MASK_SIZE	8
 
enum rte_iova_mode {
	RTE_IOVA_DC = 0,	/* Don't care mode */
	RTE_IOVA_PA = (1 << 0), /* DMA using physical address */
	RTE_IOVA_VA = (1 << 1)  /* DMA using virtual address */
};
 
/*
 * Get physical address of any mapped virtual address in the current process.
 */
phys_addr_t rte_mem_virt2phy(const void *virtaddr)
{
	int fd, retval;
	uint64_t page, physaddr;
	unsigned long virt_pfn;
	int page_size;
	off_t offset;
 
	/* Cannot parse /proc/self/pagemap, no need to log errors everywhere */
	if (!phys_addrs_available)
		return RTE_BAD_IOVA;
 
	/* standard page size */
	page_size = getpagesize();
 
	fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		RTE_LOG(ERR, EAL, "%s(): cannot open /proc/self/pagemap: %s\n",
			__func__, strerror(errno));
		return RTE_BAD_IOVA;
	}
 
	virt_pfn = (unsigned long)virtaddr / page_size;
	offset = sizeof(uint64_t) * virt_pfn;
	if (lseek(fd, offset, SEEK_SET) == (off_t) -1) {
		RTE_LOG(ERR, EAL, "%s(): seek error in /proc/self/pagemap: %s\n",
				__func__, strerror(errno));
		close(fd);
		return RTE_BAD_IOVA;
	}
 
	retval = read(fd, &page, PFN_MASK_SIZE);
	close(fd);
	if (retval < 0) {
		RTE_LOG(ERR, EAL, "%s(): cannot read /proc/self/pagemap: %s\n",
				__func__, strerror(errno));
		return RTE_BAD_IOVA;
	} else if (retval != PFN_MASK_SIZE) {
		RTE_LOG(ERR, EAL, "%s(): read %d bytes from /proc/self/pagemap "
				"but expected %d:\n",
				__func__, retval, PFN_MASK_SIZE);
		return RTE_BAD_IOVA;
	}
 
	/*
	 * the pfn (page frame number) are bits 0-54 (see
	 * pagemap.txt in linux Documentation)
	 */
	if ((page & 0x7fffffffffffffULL) == 0)
		return RTE_BAD_IOVA;
 
	physaddr = ((page & 0x7fffffffffffffULL) * page_size)
		+ ((unsigned long)virtaddr % page_size);
 
	RTE_LOG(ERR, EAL, "phyaddr %p\n", (void*)physaddr);
 
	return physaddr;
}
 
rte_iova_t rte_mem_virt2iova(const void *virtaddr)
{
	return rte_mem_virt2phy(virtaddr);
}
 
int main()
{
	int *addr1 = malloc(10);
 
	printf("addr1 = %p\n", addr1);
	rte_iova_t rte_iova = rte_mem_virt2iova(addr1);
 
	printf("addr1 = %p, 0x%x\n", addr1, (void*)rte_iova);
 
	return 0;
}
