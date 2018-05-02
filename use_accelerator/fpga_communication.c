#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include "vgg16.h"

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define ACCELERATOR_ADDRESS	0x43C00000UL
#define ACCELERATOR_RANGE	0x100000 //1MB
#define SIZE_OFFSET			0x00010
#define RESET_OFFSET		0x00014
#define KERNEL_OFFSET		0x00040
#define MATRIX_OFFSET		0x40000
#define OUT_R_OFFSET		0x80000

unsigned int dm( unsigned int target_addr){

	int fd = open("/dev/mem", O_RDWR|O_SYNC, S_IRUSR);
    volatile unsigned int *regs, *address ;
    unsigned long offset, lp_cnt;
    unsigned int value;

	if(fd == -1)
    {
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minnor=1)\n");
        return -1;
    }

	offset = 0;
	lp_cnt = 1;

	regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);
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
	
	int temp = close(fd);
    if(temp == -1)
    {
        printf("Unable to close /dev/mem.  Ensure it exists (major=1, miinor=1)\n");
        return -1;
    }
	munmap(regs, MAP_SIZE);
	return value;
}

int pm(unsigned int target_addr, unsigned int value){
	
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
    volatile unsigned int *regs, *address ;
	unsigned int offset = 0;
	int lp_cnt = 1;
	regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);

    if(fd == -1)
    {
        printf("Unable to open /dev/mem.  Ensure it exists (major=1, minnor=1)\n");
		return -1;
    }
	
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

    int temp = close(fd);
    if(temp == -1)
    {
        printf("Unable to close /dev/mem.  Ensure it exists (major=1, miinor=1)\n");
        return -1;
    }
	munmap(regs, MAP_SIZE);
	return 0;
}

float dm_float( unsigned int target_addr){

	int fd = open("/dev/mem", O_RDWR|O_SYNC, S_IRUSR);
    volatile float *regs, *address ;
    unsigned long offset, lp_cnt;
    float value;

	if(fd == -1)
    {
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minnor=1)\n");
        return -1;
    }

	offset = 0;
	lp_cnt = 1;

	regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);
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
	
	int temp = close(fd);
    if(temp == -1)
    {
        printf("Unable to close /dev/mem.  Ensure it exists (major=1, miinor=1)\n");
        return -1;
    }
	munmap(regs, MAP_SIZE);
	return value;
}

int pm_float(unsigned int target_addr, float value){
	
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
    volatile float *regs, *address ;
	unsigned int offset = 0;
	int lp_cnt = 1;
	regs = (float *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);

    if(fd == -1)
    {
        printf("Unable to open /dev/mem.  Ensure it exists (major=1, minnor=1)\n");
		return -1;
    }
	
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

    int temp = close(fd);
    if(temp == -1)
    {
        printf("Unable to close /dev/mem.  Ensure it exists (major=1, miinor=1)\n");
        return -1;
    }
	munmap(regs, MAP_SIZE);
	return 0;
}
// TODO: Should I create separate methods?
void fpga_set_matrix_kernel(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], int size) {
	int i, j;
	unsigned int matrix_address_in_fpga;
	// MATRIX
	for(i=0; i < (size+2); i++) {
		for(j=0; j < (size+2); j++) {
			matrix_address_in_fpga = (ACCELERATOR_ADDRESS + MATRIX_OFFSET) + (i * (SIZE+2) * sizeof(float) + j * sizeof(float));;
			pm_float(matrix_address_in_fpga, matrix[i][j]);
		}
	}
	// KERNEL
	matrix_address_in_fpga = ACCELERATOR_ADDRESS + KERNEL_OFFSET;
	for(i = 0; i < CONV_SIZE; i++) {
		for(j = 0; j < CONV_SIZE; j++) {
			pm_float(matrix_address_in_fpga, kernel[i][j]);
			matrix_address_in_fpga += sizeof(float);
		}
	}
}
void fpga_set_out_size(float out[SIZE+2][SIZE+2], int size) {
	int i, j;
	unsigned int matrix_address_in_fpga;
	// OUT_R
	for(i=0; i < (size+2); i++) {
		for(j=0; j < (size+2); j++) {
			matrix_address_in_fpga = (ACCELERATOR_ADDRESS + OUT_R_OFFSET) + (i * (SIZE+2) * sizeof(float) + j * sizeof(float));;
			pm_float(matrix_address_in_fpga, out[i][j]);
		}
	}
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
	for(i=0; i < (size+2); i++) {
		for(j=0; j < (size+2); j++) {
			matrix_address_in_fpga = (ACCELERATOR_ADDRESS + OUT_R_OFFSET) + (i * (SIZE+2) * sizeof(float) + j * sizeof(float));;
			out[i][j] = dm_float(matrix_address_in_fpga);
		}
	}	
}