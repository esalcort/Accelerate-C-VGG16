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
#define KERNEL_OFFSET		0x00040
#define MATRIX_OFFSET		0x40000
#define OUT_R_OFFSET		0x80000

#define LOG_DM


int pm_float_addr(unsigned int target_addr, float *value, int size){
	
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
    volatile float *regs, *address ;
	unsigned int offset = 0;
	int lp_cnt = size;
	regs = (float *)mmap(NULL, sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);

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

    int temp = close(fd);
    if(temp == -1)
    {
        printf("Unable to close /dev/mem.  Ensure it exists (major=1, miinor=1)\n");
        return -1;
    }
	munmap(regs, MAP_SIZE);
	return 0;
}

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
	munmap(NULL, MAP_SIZE);
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
	munmap(NULL, MAP_SIZE);
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
	munmap(NULL, MAP_SIZE);
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
	munmap(NULL, MAP_SIZE);
	return 0;
}

int main(int argc, char *argv[]) {
    float kernel_static[3][3] = {{1.001, 13.88, 26.04}, {35.298, 0.005, 1.104}, {0.009, 95.007, 3.006}};

	int matrix_address_in_fpga = ACCELERATOR_ADDRESS + KERNEL_OFFSET;
	printf("pms\n");
	pm_float_addr(matrix_address_in_fpga, &kernel_static[0][0], sizeof(kernel_static)/sizeof(float));
    int i;
    int offset = 0;
    printf("dms\n");
	for (i = 0; i < sizeof(kernel_static)/sizeof(float); i++) {
		dm_float(matrix_address_in_fpga + offset);
		offset += 4;
	}

	printf("\n\n single pm/dm\n");
	pm_float(matrix_address_in_fpga, kernel_static[1][1]);
	dm_float(matrix_address_in_fpga);
	pm(matrix_address_in_fpga, kernel_static[1][1]);
	dm(matrix_address_in_fpga);
	return 0;
}

