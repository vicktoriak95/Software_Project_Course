/*
 * cov.c
 *
 *  Created on: 8 באפר׳ 2020
 *      Author: User
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SPBufferset.h"


double** allocatDoubleMatrix(int row_size, int col_size){
	int i = 0;
	double **input_matrix = NULL;

	/* Allocating array of pointers to rows */
	input_matrix = (double **)malloc(row_size * sizeof(double *));
	assert(input_matrix != NULL);

	/* Allocating array of doubles for each row */
	for (i=0; i<row_size; i++){
		input_matrix[i] = (double *)malloc(col_size * sizeof(double));
		assert(input_matrix[i] != NULL);
	}

	return input_matrix;
}

void standardize_input(double** input_matrix, int row_size, int col_size){
	double sum = 0;
	double avg = 0;
	int i, j;

	for(i=0; i<row_size; i++){
		sum = 0;
		/* Calculating mean of row i */
		for(j=0; j<col_size; j++){
			sum += input_matrix[i][j];
		}
		avg = sum / col_size;
		/* Subtracting mean of row i */
		for(j=0; j<col_size; j++){
			input_matrix[i][j] -= avg;
		}
	}
}

double dot_product(double* vector1, double* vector2, int vector_size){
	double sum = 0;
	int i;

	for(i=0; i<vector_size; i++){
		sum += vector1[i] * vector2[i];
	}

	return sum;
}

void write_covariance_matrix(double** input_matrix, int row_size, int col_size, FILE* output_file){
	int result_write = 0;
	double* output_row;
	int i, j;
	double* vector1 = NULL;
	double* vector2 = NULL;

	/* Writing row size twice*/
	result_write = fwrite(&row_size, sizeof(int), 1, output_file);
	assert(result_write == 1);
	result_write = fwrite(&row_size, sizeof(int), 1, output_file);
	assert(result_write == 1);

	output_row = (double *)malloc(row_size * sizeof(double));

	/* Computing output_row and writing to output_file */
	for(i=0; i<row_size; i++){
		vector1 = input_matrix[i];
		for(j=0; j<row_size; j++){
			vector2 = input_matrix[j];
			output_row[j] = dot_product(vector1, vector2, col_size);
		}
		result_write = fwrite(output_row, sizeof(double), row_size, output_file);
		assert(result_write = row_size);
	}

	free(output_row);
}

int main(int argc, char* argv[]){
	FILE* input_file = NULL;
	FILE* output_file = NULL;
	double **input_matrix = NULL;
	int col_size = 0;
	int row_size = 0;
	int i;
	int read_result = 0;
	int close_result = 0;
	clock_t start, end;

	assert(argc == 3);

	start = clock();

	/* Opening input file */
	input_file = fopen(argv[1], "r");
	assert(input_file != NULL);

	/* Reading col and row size */
	read_result = fread(&col_size, sizeof(int), 1, input_file);
	assert(read_result == 1);
	read_result = fread(&row_size, sizeof(int), 1, input_file);
	assert(read_result == 1);

	/* Allocating input matrix */
	input_matrix = allocatDoubleMatrix(row_size, col_size);

	/* Reading rows of input matrix */
	for(i=0; i<row_size; i++){
		read_result = fread(input_matrix[i], sizeof(double), col_size, input_file);
		assert(read_result = col_size);
	}

	close_result = fclose(input_file);
	assert(close_result == 0);

	/* Standardize the input matrix */
	standardize_input(input_matrix, row_size, col_size);

	/*Opening output file */
	output_file = fopen(argv[2], "w");
	assert(output_file != NULL);
	write_covariance_matrix(input_matrix, row_size, col_size, output_file);
	close_result = fclose(output_file);
	assert(close_result == 0);

	/* Free input matrix */
	for(i=0; i<row_size; i++){
		free(input_matrix[i]);
	}
	free(input_matrix);

	end = clock();
	printf("Execution took %f seconds\n", ((double)(end-start) / CLOCKS_PER_SEC));

	return 0;
}
