/* Test Vivado HLS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define SIZE 224
#define CONV_SIZE 3
#define SIZES_TO_TEST	1

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define ACCELERATOR_ADDRESS	0x43C00000UL
#define ACCELERATOR_RANGE	0x100000 //1MB
#define SIZE_OFFSET			0x00010
#define KERNEL_OFFSET		0x00040
#define MATRIX_OFFSET		0x40000
#define OUT_R_OFFSET		0x80000

#define FPGA_MATRIX_READ	0
#define FPGA_MATRIX_WRITE	1


float in_matrix[SIZE+2][SIZE+2];
float out_matrix[SIZE+2][SIZE+2];
float dut_out_matrix[SIZE+2][SIZE+2];
float kernel_static[CONV_SIZE][CONV_SIZE] = {{1.001, 13.88, 26.04}, {35.298, 0.005, 1.104}, {0.009, 95.007, 3.006}};
int test_sizes[] = {224, 28, 14, 112, 56};

struct timeval fpga_start, fpga_end, fpga_all_start, fpga_all_end, sw_start, sw_end;

void orig_convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
	gettimeofday(&sw_start, NULL);
	int i, j;
	float sum;

	// printf("\tOriginal Convolution\n");

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			sum = matrix[i][j] * kernel[0][0] +
				matrix[i + 1][j] * kernel[1][0] +
				matrix[i + 2][j] * kernel[2][0] +
				matrix[i][j + 1] * kernel[0][1] +
				matrix[i + 1][j + 1] * kernel[1][1] +
				matrix[i + 2][j + 1] * kernel[2][1] +
				matrix[i][j + 2] * kernel[0][2] +
				matrix[i + 1][j + 2] * kernel[1][2] +
				matrix[i + 2][j + 2] * kernel[2][2];
			out[i+1][j+1] += sum;
		}
	}
	gettimeofday(&sw_end, NULL);
	
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

void fpga_convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
	gettimeofday(&fpga_all_start, NULL);
	unsigned char i, j;
	unsigned int matrix_address_in_fpga;
	// FILL MATRIX
	// printf("Copy matrix\n");
	// fpga_matrix_transfer(matrix, ACCELERATOR_ADDRESS + MATRIX_OFFSET, FPGA_MATRIX_WRITE);
	matrix_address_in_fpga = ACCELERATOR_ADDRESS + MATRIX_OFFSET;
	for(i=0; i < (size+2); i++) {
		for(j=0; j < (size+2); j++) {
			matrix_address_in_fpga = (ACCELERATOR_ADDRESS + MATRIX_OFFSET) + (i * (SIZE+2) * sizeof(float) + j * sizeof(float));;
			pm_float(matrix_address_in_fpga, matrix[i][j]);
		}
	}
	
	// SET OUT MATRIX
	// printf("Copy OUT\n");
	// fpga_matrix_transfer(out, ACCELERATOR_ADDRESS + OUT_R_OFFSET, FPGA_MATRIX_WRITE);
	matrix_address_in_fpga = ACCELERATOR_ADDRESS + OUT_R_OFFSET;
	for(i=0; i < (size+2); i++) {
		for(j=0; j < (size+2); j++) {
			// printf("out[%d][%d]=0x%x ", i, j, out[i][j] );
			matrix_address_in_fpga = (ACCELERATOR_ADDRESS + OUT_R_OFFSET) + (i * (SIZE+2) * sizeof(float) + j * sizeof(float));;
			pm_float(matrix_address_in_fpga, out[i][j]);
		}
		// printf("\n\n");
	}

	// SET OTHER VARIABLES AND START ACCELERATOR (They are all in the same page)

    // SET SIZE
	// printf("Copy size\n");
	//*(fpga_address + SIZE_OFFSET) = size;
	pm(ACCELERATOR_ADDRESS + SIZE_OFFSET, size);

	// SET KERNEL
	// printf("Copy kernel\n");
	//memcpy(fpga_address + KERNEL_OFFSET, kernel, (CONV_SIZE)*(CONV_SIZE)*sizeof(float));
	matrix_address_in_fpga = ACCELERATOR_ADDRESS + KERNEL_OFFSET;
	for(i = 0; i < CONV_SIZE; i++) {
		for(j = 0; j < CONV_SIZE; j++) {
			pm_float(matrix_address_in_fpga, kernel[i][j]);
			matrix_address_in_fpga += sizeof(float);
		}
	}

	// START ACCELLERATOR
	// printf("Set ap_start bit\n");
	// *fpga_address = 1;
	gettimeofday(&fpga_start, NULL);
	pm(ACCELERATOR_ADDRESS, 1);

	// POLL FOR DONE
	unsigned int control_signals = dm(ACCELERATOR_ADDRESS);
	while ((control_signals & 2) == 0) {
		// printf("Poll for Done. Control Signals = 0x%x\n", control_signals);
		control_signals = dm(ACCELERATOR_ADDRESS);
	}
	gettimeofday(&fpga_end, NULL);

	// CLOSE FILES
	// COPY OUTPUT MATRIX
	// fpga_matrix_transfer(out, ACCELERATOR_ADDRESS + OUT_R_OFFSET, FPGA_MATRIX_READ);
	matrix_address_in_fpga = ACCELERATOR_ADDRESS + OUT_R_OFFSET;
	for(i=0; i < (size+2); i++) {
		for(j=0; j < (size+2); j++) {
			matrix_address_in_fpga = (ACCELERATOR_ADDRESS + OUT_R_OFFSET) + (i * (SIZE+2) * sizeof(float) + j * sizeof(float));
			out[i][j]=dm_float(matrix_address_in_fpga);
			// printf("Address: 0x%x. Value: %f, Row: %d, Column: %d\n", matrix_address_in_fpga, out[i][j], i, j );
		}
	}
	gettimeofday(&fpga_all_end, NULL);
}

void read_image(char *in_file) {
	int i, j;
	FILE *iin;
	float dval;

	iin = fopen(in_file, "r");
	if (iin == NULL) {
		printf("File %s absent\n", in_file);
		exit(1);
	}

	/* Reading image */
	for (i = 1; i <= SIZE; i++) {
		for (j = 1; j <= SIZE; j++) {
			fscanf(iin, "%f", &dval);
			in_matrix[i][j] = dval;
		}
	}
	fclose(iin);
}

int main(int argc, char *argv[])
{
	/* code */
	unsigned char i, j, k;
	unsigned int error = 0;
	int size;
	read_image(argv[1]);
	for(i = 0; i < SIZES_TO_TEST; i++) {
		size = test_sizes[i];
		printf("Test Image Width: %d\n", size);
		// Perform reference code
		orig_convolution_3_x_3(in_matrix, kernel_static, out_matrix, size);
		// Perform dut code
		fpga_convolution_3_x_3(in_matrix, kernel_static, dut_out_matrix, size);
		for(j = 0; j <= (size+1); j++) {
			for (k = 0; k <= (size+1); k++)
			if (out_matrix[j][k] != dut_out_matrix[j][k]) {
				// Error
				printf("Error at row %d and column %d\n", j, k);
				printf("Expected value %f actual value %f\n", out_matrix[j][k], dut_out_matrix[j][k]);
				error++;
			}
		}
		// Change input matrix
		memcpy(in_matrix, out_matrix, (SIZE+2)*(SIZE+2)*sizeof(float));
		printf("Copied %d bytes \n", (SIZE+2)*(SIZE+2)*sizeof(float) );
	}
	if (error > 0) {
		printf("ERROR: TEST FAILED\n");
	}
	else {
		printf("DONE: TEST PASSED\n");
		printf("FPGA TIME:\t%d\n", (fpga_start.tv_sec - fpga_end.tv_sec) * 1000000 + (fpga_start.tv_usec - fpga_end.tv_usec));
		printf("FPGA ALL:\t%d\n", (fpga_all_start.tv_sec - fpga_all_end.tv_sec) * 1000000 + (fpga_all_start.tv_usec - fpga_all_end.tv_usec));
		printf("SW TIME:\t%d\n", (sw_start.tv_sec - sw_end.tv_sec) * 1000000 + (sw_start.tv_usec - sw_end.tv_usec));
	}
	return error;
}


//(end.tv_sec - start.tv_sec) * 1000000
                                    //+ (end.tv_usec - start.tv_usec);