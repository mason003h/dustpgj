#include <printk.h>
#include <mm/mmu.h>
#include <mm/kalloc.h>
#include <mm/kmalloc.h>
#include <mm/shm.h>
#include <hardware.h>
#include <kernel.h>
#include <system.h>
#include <dev/uart.h>
#include <proc.h>
#include <irq.h>
#include <kstring.h>
#include <timer.h>
#include <scheduler.h>
#include <dev/gpio.h>
#include <base16.h>
#include <dev/fb.h>
#include <dev/initrd.h>
#include <sramdisk.h>
#include <printk.h>

page_dir_entry_t* _kernel_vm;
uint32_t _phy_mem_size = 0;

void set_kernel_vm(page_dir_entry_t* vm) {
	memset(vm, 0, PAGE_DIR_SIZE);

	map_pages(vm, 0, 0, PAGE_SIZE, AP_RW_D);
	//map interrupt vector to high(virtual) mem
	map_pages(vm, INTERRUPT_VECTOR_BASE, 0, PAGE_SIZE, AP_RW_D);
	//map kernel image to high(virtual) mem
	map_pages(vm, KERNEL_BASE+PAGE_SIZE, PAGE_SIZE, V2P(_kernel_end), AP_RW_D);
	//map kernel page dir to high(virtual) mem
	map_pages(vm, (uint32_t)_kernel_vm, V2P(_kernel_vm), V2P(_kernel_vm)+PAGE_DIR_SIZE, AP_RW_D);
	//map MMIO to high(virtual) mem.
	map_pages(vm, MMIO_BASE, get_mmio_base_phy(), get_mmio_base_phy() + get_mmio_mem_size(), AP_RW_D);
	//map kernel memory trunk to high(virtual) mem.
	map_pages(vm, KMALLOC_BASE, V2P(KMALLOC_BASE), V2P(KMALLOC_BASE+KMALLOC_SIZE), AP_RW_D);
	
	if(_phy_mem_size == 0) {
		/*map some allocable memory for the pagetable alloc for rest momory mapping
		(coz we don't know the whole phymem size yet.*/
		map_pages(vm, 
			ALLOCATABLE_MEMORY_START,
			V2P(ALLOCATABLE_MEMORY_START),
			V2P(ALLOCATABLE_MEMORY_START) + INIT_RESERV_MEMORY_SIZE,
			AP_RW_D);
	}
	else {
		map_pages(vm, 
			ALLOCATABLE_MEMORY_START, 
			V2P(ALLOCATABLE_MEMORY_START),
			_phy_mem_size,
			AP_RW_D);
	}

	arch_set_kernel_vm(vm);
}

static void init_kernel_vm() {
	/*
	build free mems list only for kernel init.
	We can only use init memory part
	(from ALLOCATABLE_MEMORY_START to 'KERNEL_BASE + INIT_MEMORY_SIZE'),
	cause the boot program only mapped part of mem by _startup_page_dir(startup.c).
	Notice: This part of physical mem (0 to INIT_MEMORY_SIZE) works for init kernel page mapping
	*/
	kalloc_init(ALLOCATABLE_MEMORY_START,
			ALLOCATABLE_MEMORY_START + INIT_RESERV_MEMORY_SIZE);


	//align up to PAGE_DIR_SIZE (like 16KB in this case). 16KB memory after kernel be used for kernel page dir table 
	_kernel_vm = (page_dir_entry_t*)ALIGN_UP((uint32_t)_kernel_end, PAGE_DIR_SIZE);

	set_kernel_vm(_kernel_vm);
	
	//Use physical address of kernel virtual memory as the new virtual memory page dir table base.
	__set_translation_table_base(V2P((uint32_t)_kernel_vm));

	/*
	init kernel trunk memory malloc.
	*/
	km_init();
}

static void init_allocable_mem() {
	_phy_mem_size = get_phy_ram_size();

	map_pages(_kernel_vm,
		ALLOCATABLE_MEMORY_START, 
		V2P(ALLOCATABLE_MEMORY_START),
		_phy_mem_size,
		AP_RW_D);
}

static void load_init_proc() {
	const char* name = "init";

	printk("Loading the first process ... ");
	process_t *proc = proc_create(TYPE_PROC); //create first process
	if(proc == NULL) {
		printk("panic: init process create failed!\n");
		return;
	}
	
	int32_t size = 0;
	const char *p = read_initrd(name, &size);
	if(p == NULL) {
		printk("panic: init process load failed!\n");
		return;
	}
	else {
		proc_load(proc, p, size);
		strncpy(proc->cmd, name, CMD_MAX);
	}
	printk("ok.\n");
}

static void welcome() {
	printk("\n=================\n"
				"EwokOS (by Misa.Z)\n"
				"=================\n"
				"Kernel got ready(MMU and ProcMan).\n\n");
}

void kernel_entry() {
	init_kernel_vm();  /* Done mapping all mem */
	init_allocable_mem(); /*init the rest allocable memory VM*/
	welcome(); /*show welcome words*/
	hw_init(); /*hardware init*/
	shm_init(); /*init share memory*/
	gpio_init(); 
	fb_init(); /*framebuffer init*/
	uart_init(); /*init uart for debug*/
	ipc_init(); /*init internal process communiation*/

	/*decode initramdisk to high memory(kernel trunk memory).*/
	if(load_initrd() != 0)
		return;
	
	proc_init(); /*init process mananer*/
	load_init_proc(); /*load init process(first process)*/
	irq_init(); /*init irq interrupts*/
	timer_init(); /*init timer irq*/
	scheduler_init(); /*start scheduler*/

	while(1) {
		schedule();
	}
	close_initrd();
}
