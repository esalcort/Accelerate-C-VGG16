#include "vgg16.h"

void convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size) {
#pragma HLS INTERFACE s_axilite port=out bundle=ctrl_bus
#pragma HLS INTERFACE s_axilite port=matrix bundle=ctrl_bus
#pragma HLS INTERFACE s_axilite port=kernel bundle=ctrl_bus
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl_bus
//#pragma HLS INTERFACE bram port=out //Creates separate block for this variable
//#pragma HLS INTERFACE bram port=matrix //Creates separate block for this variable

//#pragma HLS ARRAY_PARTITION variable=kernel complete dim=0 //No performance gains when loops are not unrolled

	int i, j;
	float sum;

	for (i = 0; i < size; i++) {
#pragma HLS UNROLL factor=7 // Unrolling child loop seems to have give less latency
#pragma HLS LOOP_TRIPCOUNT min=14 max=224
// #pragma HLS PIPELINE // Do not use when child loop is unrolled
		for (j = 0; j < size; j++) {
#pragma HLS PIPELINE
#pragma HLS UNROLL factor=7 // Useless when parent loop is Pipelined
#pragma HLS LOOP_TRIPCOUNT min=14 max=224
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
