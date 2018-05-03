/* Test Vivado HLS */

#include <string.h>
#include <sys/time.h>

#include "vgg16.h"

#define SIZE 224
#define CONV_SIZE 3

#define SIZES_TO_TEST	5
#define IN_FRAMES_TO_TEST	3

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


float in_matrix[IN_FRAMES_TO_TEST][SIZE+2][SIZE+2];
float out_matrix[SIZE+2][SIZE+2];
float dut_out_matrix[SIZE+2][SIZE+2];
float kernel_static[CONV_SIZE][CONV_SIZE] = {{1.001, 13.88, 26.04}, {35.298, 0.005, 1.104}, {0.009, 95.007, 3.006}};
int test_sizes[] = {224, 28, 14, 112, 56};

struct timeval fpga_start_time, fpga_end_time, fpga_all_start, fpga_all_end, sw_start, sw_end;

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

void fpga_convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
	
	gettimeofday(&fpga_all_start, NULL);
	unsigned char i, j;
	// FILL MATRIX
	// printf("Copy matrix\n");
	fpga_set_matrix(matrix, size);

	// SET KERNEL
	// printf("Copy kernel\n");
	fpga_set_kernel(kernel);

	// START ACCELLERATOR
	// printf("Set ap_start bit\n");
	gettimeofday(&fpga_start_time, NULL);
	fpga_start();

	// POLL FOR DONE
	fpga_poll();
	gettimeofday(&fpga_end_time, NULL);
	gettimeofday(&fpga_all_end, NULL);
}

void read_image(char *in_file) {
	int i, j, l;
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
			for (l = 0; l < IN_FRAMES_TO_TEST; l++) {
				fscanf(iin, "%f", &dval);
				in_matrix[l][i][j] = dval;
			}
		}
	}
	fclose(iin);
}

int main(int argc, char *argv[])
{
	/* code */
	unsigned char i, j, k, l;
	unsigned int error = 0;
	int size;
	read_image(argv[1]);
	pmdm_open();
	for(i = 0; i < SIZES_TO_TEST; i++) {
		size = test_sizes[i];
		fpga_set_out_size(dut_out_matrix, size);
		for (l = 0; l < IN_FRAMES_TO_TEST; l++) {
			printf("Test Image Width: %d, Frame #: %d\n", size, l);
			// Perform reference code
			orig_convolution_3_x_3(in_matrix[l], kernel_static, out_matrix, size);
			// Perform dut code
			// fpga_set_size(size);
			fpga_convolution_3_x_3(in_matrix[l], kernel_static, dut_out_matrix, size);
			printf("FPGA TIME:\t%d\n", (fpga_start_time.tv_sec - fpga_end_time.tv_sec) * 1000000 + (fpga_start_time.tv_usec - fpga_end_time.tv_usec));
			printf("FPGA ALL:\t%d\n", (fpga_all_start.tv_sec - fpga_all_end.tv_sec) * 1000000 + (fpga_all_start.tv_usec - fpga_all_end.tv_usec));
			printf("SW TIME:\t%d\n", (sw_start.tv_sec - sw_end.tv_sec) * 1000000 + (sw_start.tv_usec - sw_end.tv_usec));
		}
		// COPY OUTPUT MATRIX
		fpga_read_out_r(dut_out_matrix, size);
		for(j = 0; j <= (size+1); j++) {
			for (k = 0; k <= (size+1); k++)
			if (out_matrix[j][k] != dut_out_matrix[j][k]) {
				// Error
				printf("Error at row %d and column %d\n", j, k);
				printf("Expected value %f actual value %f\n", out_matrix[j][k], dut_out_matrix[j][k]);
				error++;
			}
		}
	}
	pmdm_close();
	if (error > 0) {
		printf("ERROR: TEST FAILED\n");
	}
	else {
		printf("DONE: TEST PASSED\n");
		
	}
	return error;
}


//(end.tv_sec - start.tv_sec) * 1000000
                                    //+ (end.tv_usec - start.tv_usec);