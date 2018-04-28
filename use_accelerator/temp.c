#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define ACCELERATOR_ADDRESS 0x43C00000UL
#define ACCELERATOR_RANGE   0x100000 //1MB
#define SIZE_OFFSET         0x00010
#define KERNEL_OFFSET       0x00040
#define MATRIX_OFFSET       0x40000
#define OUT_R_OFFSET        0x80000


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

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

int main(int argc, char const *argv[])
{
    unsigned int i,j;
    unsigned int matrix_address = ACCELERATOR_ADDRESS + MATRIX_OFFSET;
    for(i = 0; i < 226; i++) {
        for(j = 0; j < 226; j++) {
            pm_float(matrix_address, (float)(i + j/100.0));
            matrix_address += sizeof(float);
        }
    }
    matrix_address = ACCELERATOR_ADDRESS + MATRIX_OFFSET;
    for(i = 0; i < 10; i++) {
        for(j = 0; j < 10; j++) {
            matrix_address = (ACCELERATOR_ADDRESS + MATRIX_OFFSET) + (i * 226 * sizeof(float) + j * sizeof(float));
            printf("Address: 0x%x, Value: %f \n", matrix_address, dm_float(matrix_address));
            // matrix_address += sizeof(float);
        }
    }
    
    return 0;
}