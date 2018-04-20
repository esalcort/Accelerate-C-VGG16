#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vgg16.h"
#include <ctype.h>


// Weights and image block START
float image[3][SIZE][SIZE];
int cshape[13][4] = { 
	{ 64, 3, CONV_SIZE, CONV_SIZE },
	{ 64, 64, CONV_SIZE, CONV_SIZE },
	{ 128, 64, CONV_SIZE, CONV_SIZE },
	{ 128, 128, CONV_SIZE, CONV_SIZE },
	{ 256, 128, CONV_SIZE, CONV_SIZE },
	{ 256, 256, CONV_SIZE, CONV_SIZE },
	{ 256, 256, CONV_SIZE, CONV_SIZE },
	{ 512, 256, CONV_SIZE, CONV_SIZE },
	{ 512, 512, CONV_SIZE, CONV_SIZE },
	{ 512, 512, CONV_SIZE, CONV_SIZE },
	{ 512, 512, CONV_SIZE, CONV_SIZE },
	{ 512, 512, CONV_SIZE, CONV_SIZE },
	{ 512, 512, CONV_SIZE, CONV_SIZE }
};
float wc[13][512][512][CONV_SIZE][CONV_SIZE];
float **bc;
int dshape[3][2] = {
	{ 25088, 4096 },
	{ 4096, 4096 },
	{ 4096, 1000 }
};


// Blocks for intermediate convolutions
int mem_block_shape[3] = {512, SIZE, SIZE};
float mem_block1[512][SIZE][SIZE];
float mem_block2[512][SIZE][SIZE];

// Weights and image block END


void reset_mem_block(float mem[512][SIZE][SIZE]) {
	int i, j, k;
	for (i = 0; i < mem_block_shape[0]; i++) {
		for (j = 0; j < mem_block_shape[1]; j++) {
			for (k = 0; k < mem_block_shape[2]; k++) {
				mem[i][j][k] = 0.0;
			}
		}
	}
}

void init_memory() {
	int i, j, k, l;

	bc = malloc(13 * sizeof(float*));
	for (l = 0; l < 13; l++) {
		bc[l] = malloc(cshape[l][0] * sizeof(float));
	}
	reset_mem_block(mem_block1);
	reset_mem_block(mem_block2);
}


void free_memory() {
	int i, j, k, l;
	for (l = 0; l < 13; l++) {
		free(bc[l]);
	}
	free(bc);

}


void read_weights(char *in_file, int lvls) {
	float dval;
	int i, j, k, l, z;
	FILE *iin;
	int total_lvls_read = 0;

	iin = fopen(in_file, "r");
	if (iin == NULL) {
		printf("File %s absent\n", in_file);
		exit(1);
	}
	
	// Reading convolution weights (store them flipped from begining)
	z = 0;
	printf("Read conv block %d weights\n", z);
	for (i = 0; i < cshape[z][0]; i++) {
		for (j = 0; j < cshape[z][1]; j++) {
			for (k = 0; k < cshape[z][2]; k++) {
				for (l = 0; l < cshape[z][3]; l++) {
					fscanf(iin, "%f", &dval);
					wc[z][i][j][CONV_SIZE - k - 1][CONV_SIZE - l - 1] = dval;
				}
			}
		}
	}
	for (i = 0; i < cshape[z][0]; i++) {
		fscanf(iin, "%f", &dval);
		bc[z][i] = dval;
	}
	total_lvls_read += 1;

	fclose(iin);
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
	for (i = 0; i < SIZE; i++) {
		for (j = 0; j < SIZE; j++) {
			for (l = 0; l < 3; l++) {
				fscanf(iin, "%f", &dval);
				image[l][i][j] = dval;
			}
		}
	}
}

void normalize_image() {
	int i, j, l;
	float coef[3] = { 103.939, 116.779, 123.68 };

	for (l = 0; l < 3; l++) {
		for (i = 0; i < SIZE; i++) {
			for (j = 0; j < SIZE; j++) {
				image[l][i][j] -= coef[l];
			}
		}
	}
}

void get_VGG16_predict(int only_convolution) {
	int i, j;
	int level, cur_size;

	// Init intermediate memory
	reset_mem_block(mem_block1);

	// Layer 1 (Convolution 3 -> 64)
	level = 0;
	cur_size = SIZE;
	for (i = 0; i < cshape[level][0]; i++) {
		for (j = 0; j < cshape[level][1]; j++) {
			convolution_3_x_3(image[j], wc[level][i][j], mem_block1[i], cur_size);
		}
	}
	
	return;
}

void orig_convolution_3_x_3(float matrix[SIZE][SIZE], float kernel[CONV_SIZE][CONV_SIZE], float out[SIZE][SIZE], int size) {
	int i, j;
	float sum;
	float zeropad[SIZE + 2][SIZE + 2] = { 0.0 };

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			zeropad[i + 1][j + 1] = matrix[i][j];
		}
	}

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			sum = zeropad[i][j] * kernel[0][0] +
				zeropad[i + 1][j] * kernel[1][0] +
				zeropad[i + 2][j] * kernel[2][0] +
				zeropad[i][j + 1] * kernel[0][1] +
				zeropad[i + 1][j + 1] * kernel[1][1] +
				zeropad[i + 2][j + 1] * kernel[2][1] +
				zeropad[i][j + 2] * kernel[0][2] +
				zeropad[i + 1][j + 2] * kernel[1][2] +
				zeropad[i + 2][j + 2] * kernel[2][2];
			out[i][j] += sum;
		}
	}
	
}

int compare_get_VGG16_predict(int only_convolution) {
	int i, j;
	int level, cur_size;
	int error = 0;

	// Init intermediate memory
	reset_mem_block(mem_block2);

	// Layer 1 (Convolution 3 -> 64)
	level = 0;
	cur_size = SIZE;
	for (i = 0; i < cshape[level][0]; i++) {
		for (j = 0; j < cshape[level][1]; j++) {
			orig_convolution_3_x_3(image[j], wc[level][i][j], mem_block2[i], cur_size);
		}
	}
	int r, c;
	for (i = 0; i < cshape[level][0]; i++) {
		for (r = 0; r < cur_size; r++) {
			for (c = 0; c < cur_size; c++) {
				if (mem_block1[i][r][c] != mem_block2[i][r][c]) {
					error++;
				}
			}
		}
	}
	
	return error;
}


char *trimwhitespace(char *str)
{
	char *end;

	// Trim leading space
	while (isspace((unsigned char)*str)) str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}


int main(int argc, char *argv[]) {
	FILE *file_list, *results;
	char buf[1024];
#ifndef _WIN32
	struct timeval timeStart, timeEnd;
#else
	time_t timeStart, timeEnd;
#endif
	double deltaTime;
	char *weights_file;
	char *image_list_file;
	char *output_file;
	int lvls = -1;
	int only_convolution = 0;

	// if (argc != 4 && argc != 5) {
	// 	printf("Usage: <program.exe> <weights file> <images list file> <output file> <only_convolution [optional]>\n");
	// 	return 0;
	// }
//	weights_file = argv[1];
	weights_file = "vgg16_weights.txt";
//	image_list_file = argv[2];
	image_list_file = "filelist.txt";
//	output_file = argv[3];
	output_file = "output.txt";
	if (argc == 5) {
		lvls = 13;
		only_convolution = 1;
	}

	init_memory();
	file_list = fopen(image_list_file, "r");
	if (file_list == NULL) {
		printf("Check file list location: %s", image_list_file);
		return 1;
	}
	results = fopen(output_file, "w");
	if (results == NULL) {
		printf("Couldn't open file for writing: %s", output_file);
		return 1;
	}

	read_weights(weights_file, lvls);
	printf("Reading weights: %.3lf sec\n", deltaTime);
	int err_cnt;
	fgets(buf, 1024, file_list);
	if (strlen(buf) != 0) {
		printf("%d\n", strlen(buf));
		read_image(trimwhitespace(buf));
		normalize_image();
		// dump_image();
		get_VGG16_predict(only_convolution);
		err_cnt = compare_get_VGG16_predict(only_convolution);
	}
	free_memory();
	fclose(file_list);

	int ret_val;
	if (err_cnt == 0) {
        printf("*** TEST PASSED ***\n");
        ret_val = 0;
    } else {
        printf("!!! TEST FAILED - %d mismatches detected !!!\n", err_cnt);
        ret_val = -1;
    }
	return ret_val;
}
