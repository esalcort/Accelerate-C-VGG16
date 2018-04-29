#include "vgg16.h"
#include <pthread.h>

// #define DEBUG_LOG
// #define DEBUG_LOG_ADDRESSES

float fpga_out[SIZE + 2][SIZE + 2] = {0.0};
float (*fpga_matrix)[SIZE+2];
float (*fpga_kernel)[CONV_SIZE];
int fpga_size;
unsigned char fpga_busy = 0;

pthread_t tid;

extern int numthreads;
/*
typedef struct 
{
	float (*matrix)[SIZE+2];
	float (*out)[SIZE+2];
	float (*kernel)[CONV_SIZE];
	int size;
} convolution_t;

convolution_t fpga_convolution;
*/
void reset_fpga(int size) {
	unsigned char i, j;
	for(i = 0; i < (size + 2); i++) {
		for(j = 0; j < (size + 2); j++) {
			fpga_out[i][j] = 0.0;
		}
	}
}
void join_fpga(float out[SIZE+2][SIZE+2], int size) {
	unsigned char i, j;
	// pthread_join(tid, NULL);
	for(i = 0; i < (size + 2); i++) {
		for(j = 0; j < (size + 2); j++) {
			out[i][j] += fpga_out[i][j];
		}
	}
}
// void *convolution_in_fpga(void* args) {
/*void convolution_in_fpga() {
	int i, j;
	float sum;
	// convolution_t * conv = (convolution_t *) args;
	for (i = 0; i < fpga_size; i++) {
		for (j = 0; j < fpga_size; j++) {
			sum = 	fpga_matrix[i][j] 			* 	fpga_kernel[0][0] +
					fpga_matrix[i + 1][j]		* 	fpga_kernel[1][0] +
					fpga_matrix[i + 2][j]		* 	fpga_kernel[2][0] +
					fpga_matrix[i][j + 1]		* 	fpga_kernel[0][1] +
					fpga_matrix[i + 1][j + 1]	*	fpga_kernel[1][1] +
					fpga_matrix[i + 2][j + 1]	* 	fpga_kernel[2][1] +
					fpga_matrix[i][j + 2] 		* 	fpga_kernel[0][2] +
					fpga_matrix[i + 1][j + 2] 	* 	fpga_kernel[1][2] +
					fpga_matrix[i + 2][j + 2] 	* 	fpga_kernel[2][2] ;
			fpga_out[i+1][j+1] += sum;
		}
	}
	fpga_busy = 0;
#ifdef DEBUG_LOG
	printf("convolution_in_fpga:\tDone tid=%d\n", tid);
#endif
}
*/
void convolution_in_fpga(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
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
	fpga_busy = 0;
}

void convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
	int i, j;
	float sum;

	if(!fpga_busy) {
		/*
		fpga_convolution.matrix = matrix;
		fpga_convolution.out = fpga_out;
		fpga_convolution.kernel = kernel;
		fpga_convolution.size = size;
		*/
	// 	fpga_matrix = matrix;
	// 	fpga_kernel = kernel;
	// 	fpga_size = size;
	// 	fpga_busy = 1; 
	// 	// pthread_create(&tid, NULL, convolution_in_fpga, NULL);
	// 	convolution_in_fpga();
		convolution_in_fpga(matrix, kernel, out, size);
	}
	else {
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
}

void convolution_2d(int shape_depth, float input[][SIZE + 2][SIZE + 2], float weights_kernel[][CONV_SIZE][CONV_SIZE],
					float output[SIZE + 2][SIZE + 2], int size)
{
	int j=0;
	// reset_fpga(size);
		for (j = 0; j < shape_depth; j++) {
			convolution_3_x_3(input[j], weights_kernel[j], output, size);
		}
	// join_fpga(output, size);
}