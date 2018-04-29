#ifndef VGG16_H
#define VGG16_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 224
#define CONV_SIZE 3

void convolution_2d(int shape_depth, float input[][SIZE + 2][SIZE + 2], float weights_kernel[][CONV_SIZE][CONV_SIZE],
					float output[SIZE + 2][SIZE + 2], int size);

void fpga_set_arguments(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size);
void fpga_start();
void fpga_poll();
#endif