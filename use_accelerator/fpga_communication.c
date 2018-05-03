#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include "vgg16.h"

#define MAP_SIZE 0x2000UL
#define MAP_MASK (MAP_SIZE - 1)

#define ACCELERATOR_ADDRESS	0x43C00000UL
#define ACCELERATOR_RANGE	0x100000 //1MB
#define SIZE_OFFSET			0x00010
#define RESET_OFFSET		0x00014
#define KERNEL_OFFSET		0x00040
#define MATRIX_OFFSET		0x40000
#define OUT_R_OFFSET		0x80000

int fpga_fd;

unsigned int dm( unsigned int target_addr){

    volatile unsigned int *regs, *address ;
    unsigned long offset, lp_cnt;
    unsigned int value;


	offset = 0;
	lp_cnt = 1;

	regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, target_addr & ~MAP_MASK);
    while( lp_cnt) {
    	#ifdef LOG_DM
        printf("0x%.4x" , (target_addr+offset));
        #endif

		address = regs + (((target_addr+ offset) & MAP_MASK)>>2);
		value = *address;
		#ifdef LOG_DM
        printf(" = 0x%.8x\n", value);              // display register value
        #endif
        lp_cnt -= 1;
        offset  += 4; // WORD alligned
    } // End while loop	
	
	munmap(regs, MAP_SIZE);
	return value;
}

int pm(unsigned int target_addr, unsigned int value){
	
    volatile unsigned int *regs, *address ;
	unsigned int offset = 0;
	int lp_cnt = 1;
	regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, target_addr & ~MAP_MASK);
	
	while (lp_cnt) {
		#ifdef LOG_DM
        printf("0x%.8x" , (target_addr + offset));
        #endif
        address = regs + (((target_addr+ offset) & MAP_MASK)>>2);
        *address = value;
		// perform write command
		#ifdef LOG_DM
        printf(" = 0x%.8x\n", *address);            // display register valuue
        #endif
		lp_cnt -= 1;
        offset  += 4; //
		// WORD alligned
    } // End of while loop

	munmap(regs, MAP_SIZE);
	return 0;
}

int pmdm_open(void){
	
	fpga_fd = open("/dev/mem", O_RDWR|O_SYNC);

    if(fpga_fd == -1)
    {
        printf("Unable to open /dev/mem.  Ensure it exists (major=1, minnor=1)\n");
		return -1;
    }
	
	return 0;
}

int pmdm_close(void){

    int temp = close(fpga_fd);
    if(temp == -1)
    {
        printf("Unable to close /dev/mem.  Ensure it exists (major=1, miinor=1)\n");
        return -1;
    }
	return 0;
}


int pm_float_row(unsigned int target_addr, float *value, int size){	
    volatile float *regs, *address ;
	unsigned int offset = target_addr & MAP_MASK;
	unsigned int base_addr = target_addr & ~MAP_MASK;
	int lp_cnt = size;
	regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);

	if ((size * sizeof(float)) > (MAP_SIZE - offset)) {
		// printf("Target Address = %.8x, Base Address = 0x%.8x, Offset = 0x%.8x, Size = %d\n", target_addr, base_addr, offset, size);
		lp_cnt = (MAP_SIZE - offset) / sizeof(float);
		// printf("lp_cnt = %d, MAP_SIZE - offset = %d\n",lp_cnt, MAP_SIZE - offset );
		while (lp_cnt) {
			#ifdef LOG_DM
	        printf("0x%.8x" , (base_addr + offset));
	        #endif
	        address = regs + (((base_addr+ offset) & MAP_MASK)>>2);
	        *address = *value;
			// perform write command
			#ifdef LOG_DM
	        printf(" = 0x%.8x\n", *address);            // display register valuue
	        #endif
			lp_cnt -= 1;
	        offset  += 4;
			value++;
			//
			// WORD alligned
	    } // End of while loop
		munmap(regs, MAP_SIZE);
		base_addr += MAP_SIZE;
		regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);
		lp_cnt = size - ((MAP_SIZE - offset) / sizeof(float));
		offset = 0;
		// printf("Target Address = %.8x, Base Address = 0x%.8x, Offset = 0x%.8x, Size = %d\n", target_addr, base_addr, offset, size);
		// printf("lp_cnt = %d\n", lp_cnt);

	}
	while (lp_cnt) {
		#ifdef LOG_DM
        printf("0x%.8x" , (base_addr + offset));
        #endif
        address = regs + (((base_addr + offset) & MAP_MASK)>>2);
        *address = *value;
		// perform write command
		#ifdef LOG_DM
        printf(" = 0x%.8x\n", *address);            // display register valuue
        #endif
		lp_cnt -= 1;
        offset  += 4;
		value++;
		//
		// WORD alligned
    } // End of while loop

	munmap(regs, MAP_SIZE);
	return 0;
}

int pm_square_matrix(unsigned int target_addr, float matrix[SIZE + 2][SIZE + 2], int size){	

    volatile float *regs, *address ;
	unsigned int offset;
	unsigned int base_addr = target_addr & ~MAP_MASK;
	regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);
	unsigned int i, j, page_count, actual_page;

    actual_page = page_count = 0;
    for(i = 0; i < size; i++) {
    	for (j = 0; j < size; j++) {
    		offset = i * (SIZE + 2) * sizeof(float) + j * sizeof(float);
    		actual_page = offset / MAP_SIZE;
    		// if ( !((i == 0) && (j == 0)) && ((offset & MAP_MASK) == 0)) {
    		if (page_count < actual_page) {
    			// This is not the first element and offset is multiple of MAP_SIZE,
    			// need to create a new page
    			page_count++;
    			munmap(regs, MAP_SIZE);
				base_addr += MAP_SIZE;
				regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);
    		}
    		address = regs + (( offset & MAP_MASK)>>2);
	        *address = matrix[i][j];
    	}
    }

	munmap(regs, MAP_SIZE);
	return 0;
}

int dm_square_matrix(unsigned int target_addr, float matrix[SIZE + 2][SIZE + 2], int size){	

    volatile float *regs, *address ;
	unsigned int offset;
	unsigned int base_addr = target_addr & ~MAP_MASK;
	regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);
	unsigned int i, j, page_count, actual_page;

    actual_page = page_count = 0;
    for(i = 0; i < size; i++) {
    	for (j = 0; j < size; j++) {
    		offset = i * (SIZE + 2) * sizeof(float) + j * sizeof(float);
    		actual_page = offset / MAP_SIZE;
    		// if ( !((i == 0) && (j == 0)) && ((offset & MAP_MASK) == 0)) {
    		if (page_count < actual_page) {
    			// This is not the first element and offset is multiple of MAP_SIZE,
    			// need to create a new page
    			page_count++;
    			munmap(regs, MAP_SIZE);
				base_addr += MAP_SIZE;
				regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);
    		}
    		address = regs + (( offset & MAP_MASK)>>2);
	        matrix[i][j] = *address; 
    	}
    }

	munmap(regs, MAP_SIZE);
	return 0;
}

// TODO: Should I create separate methods?
void fpga_set_matrix(float matrix[SIZE+2][SIZE+2], int size) {
	pm_square_matrix(ACCELERATOR_ADDRESS + MATRIX_OFFSET, matrix, size+2);
}

void fpga_set_kernel(float kernel[CONV_SIZE][CONV_SIZE]) {

    volatile float *regs, *address ;
	unsigned int offset = KERNEL_OFFSET;
	unsigned int base_addr = ACCELERATOR_ADDRESS;
	unsigned int i, j;

	regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_fd, base_addr);
    for(i = 0; i < 3; i++) {
    	for (j = 0; j < 3; j++) {
    		address = regs + (( offset & MAP_MASK)>>2);
	        *address = kernel[i][j];
	        offset += 4;
    	}
    }

	munmap(regs, MAP_SIZE);
	return 0;
}

void fpga_set_out_size(float out[SIZE+2][SIZE+2], int size) {
	int i, j;
	unsigned int matrix_address_in_fpga;
	// OUT_R	
	// for(i=0; i < (size+2); i++) {
	// 		matrix_address_in_fpga = (ACCELERATOR_ADDRESS + OUT_R_OFFSET) + (i * (SIZE+2) * sizeof(float));
	// 		pm_float_row(matrix_address_in_fpga, out[i], (size+2));
	// }
	pm_square_matrix(ACCELERATOR_ADDRESS + OUT_R_OFFSET, out, size+2);
	// SIZE
	pm(ACCELERATOR_ADDRESS + SIZE_OFFSET, size);
}
void fpga_start() {
	pm(ACCELERATOR_ADDRESS, 1);
}
void fpga_poll() {
	unsigned int control_signals = dm(ACCELERATOR_ADDRESS);
	while ((control_signals & 2) == 0) {
		control_signals = dm(ACCELERATOR_ADDRESS);
	}
}
void fpga_read_out_r(float out[SIZE+2][SIZE+2], int size) {
	int i, j;
	unsigned int matrix_address_in_fpga;
	// OUT_R
	dm_square_matrix(ACCELERATOR_ADDRESS + OUT_R_OFFSET, out, size+2);
	
}