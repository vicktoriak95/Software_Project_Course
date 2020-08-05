/*
 * eigen.c
 *
 *  Created on: 9 באפר׳ 2020
 *      Author: User
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "SPBufferset.h"

#define epsilon 0.00001

void print_vector(double* vector, int vector_size){
	int i;
	for(i=0; i<vector_size; i++){
		printf("%f ", vector[i]);
	}
	printf("\n\n");
}

void create_random_vector(double * vector, int vector_size){
	int i;
	for(i=0; i<vector_size; i++){
		vector[i] = (double)rand();
	}
}

double create_next_vector(FILE* input_file, double* current_vector, double* next_vector, double* input_row, int matrix_dim){
	double dot_product = 0;
	double sum_of_powers = 0;
	double norm = 0;
	int read_result = 0;
	int seek_result = 0;
	int i,j;
	for(i=0; i<matrix_dim; i++){ /* Iterating over rows of A */
		/* Read input_row */
		read_result = fread(input_row, sizeof(double), matrix_dim, input_file);
		assert(read_result == matrix_dim);

		/* Calculating dot_product of current_vector and input_row */
		dot_product = 0;
		for(j=0; j<matrix_dim; j++){
			dot_product += input_row[j] * current_vector[j];
		}
		next_vector[i] = dot_product;

		/* Using dot_product to cacl norm */
		sum_of_powers += dot_product * dot_product;
	}

	/* Return file position to start of matrix */
	seek_result = fseek(input_file, 2 * sizeof(int), SEEK_SET);
	assert(seek_result == 0);

	norm = sqrt(sum_of_powers);
	return norm;
}

int main(int argc, char* argv[]){
	FILE* input_file = NULL;
	FILE* output_file = NULL;
	double* current_vector = NULL;
	double* next_vector = NULL;
	double* input_row = NULL;
	double* temp = NULL;
	int read_result = 0;
	int write_result = 0;
	int col_size = 0;
	int row_size = 0;
	int norm = 0;
	int vectors_close_enough = 0;
	int i;
	clock_t start, end;

	assert(argc == 3);
	start = clock();

	srand(time(NULL));

	/* Opening input file */
	input_file = fopen(argv[1], "r");
	assert(input_file != NULL);


	/* Reading col and row size */
	read_result = fread(&col_size, sizeof(int), 1, input_file);
	assert(read_result == 1);
	read_result = fread(&row_size, sizeof(int), 1, input_file);
	assert(read_result == 1);

	current_vector = (double *)malloc(col_size * sizeof(double));
	next_vector = (double *)malloc(col_size * sizeof(double));
	input_row = (double *)malloc(col_size * sizeof(double));

	/* Creating random vector b0 */
	create_random_vector(current_vector, col_size);

	while (vectors_close_enough == 0){ /* Diff between vectors is bigger than epsilon */
		/* Calculating norm and next_vector */
		norm = create_next_vector(input_file, current_vector, next_vector, input_row, col_size);
		/* Dividing next_vector by norm, calc the diff between current_vector and next_vector */
		vectors_close_enough = 1;
		for(i=0; i<row_size; i++){
			next_vector[i] = next_vector[i] / norm;
			if(fabs(next_vector[i] - current_vector[i]) > epsilon){
				vectors_close_enough = 0;
			}
		}
		/* Assigning current_vector to be next_vector */
		temp = current_vector;
		current_vector = next_vector;
		next_vector = temp;
	}
	/* Closing input_file */
	fclose(input_file);

	/* Opening output_file */
	output_file = fopen(argv[2], "w");
	assert(input_file != NULL);

	/* Writing col and row size */
	col_size = 1;
	write_result = fwrite(&col_size, sizeof(int), 1, output_file);
	assert(write_result == 1);
	write_result = fwrite(&row_size, sizeof(int), 1, output_file);
	assert(write_result == 1);

	/* Writing result vector to file */
	write_result = fwrite(current_vector, sizeof(double), row_size, output_file);
	assert(write_result == row_size);

	/* Closing input_file and free memory */
	fclose(output_file);
	free(current_vector);
	free(next_vector);
	free(input_row);

	end = clock();
	printf("Execution took %f seconds\n", ((double)(end-start) / CLOCKS_PER_SEC));

	return 0;

}
