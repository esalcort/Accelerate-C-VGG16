#ifndef VGG16_H
#define VGG16_H

#include <math.h>

#define SIZE 224
#define CONV_SIZE 3

// External function prototypes
void convolution_3_x_3(float matrix[SIZE+2][SIZE+2], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE+2][SIZE+2], int size);

#endif