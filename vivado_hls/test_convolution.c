/* Test Vivado HLS */

#include "vgg16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZES_TO_TEST	5

float in_matrix[SIZE+2][SIZE+2];
float out_matrix[SIZE+2][SIZE+2];
float dut_out_matrix[SIZE+2][SIZE+2];
float kernel_static[CONV_SIZE][CONV_SIZE] = {{1.001, 13.88, 26.04}, {35.298, 0.005, 1.104}, {0.009, 95.007, 3.006}};
int test_sizes[] = {224, 112, 56, 28, 14};

void orig_convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
	int i, j;
	float sum;

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
		convolution_3_x_3(in_matrix, kernel_static, dut_out_matrix, size);
		for(j = 1; j <= size; j++) {
			for (k = 1; k <= size; k++)
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
	}
	return error;
}
