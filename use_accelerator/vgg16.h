#ifndef VGG16_H
#define VGG16_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 224
#define CONV_SIZE 3

// External function prototypes
void convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size);
void reset_fpga(int size);
void join_fpga(float out[SIZE+2][SIZE+2], int size);
void convolution_2d(int shape_depth, float input[][SIZE + 2][SIZE + 2], float weights_kernel[][CONV_SIZE][CONV_SIZE],
					float output[SIZE + 2][SIZE + 2], int size);

#endif