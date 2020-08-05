/*
 * main.c
 *
 *  Created on: May 2, 2020
 *      Author: User
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "spmat.h"
/*
 * TODO
#define epsilon 0.00001
*/
void create_random_vector(double * vector, int vector_size){
	int i;
	for(i=0; i<vector_size; i++){
		vector[i] = (double)rand();
	}
}

void read_initial_vector(FILE* input_vector, double* vector, int vector_size){
	int read_result = 0;

	read_result = fread(vector, sizeof(double), vector_size, input_vector);
	assert(read_result = vector_size);
}

int count_nnz_values(FILE* input_mat, int n){
	int nnz = 0;
	int i, j;
	int read_result = 0;
	int seek_result = 0;
	double* row;

	row = (double*)malloc(n * sizeof(double));

	/* Reading rows of input matrix and counting non zero values */
	for(i = 0; i < n; i ++){
		read_result = fread(row, sizeof(double), n, input_mat);
		assert(read_result = n);
		for(j = 0; j < n; j++){
			if(row[j] != 0){
				nnz += 1;
			}
		}
	}
	free(row);
	/* Return file position to start of matrix */
	seek_result = fseek(input_mat, 2 * sizeof(int), SEEK_SET);
	assert(seek_result == 0);
	return nnz;
}

void read_matrix_rows(spmat* sp_mat, FILE* input_matrix, int n){
	double* row;
	int read_result;
	int i;

	/* TODO - maybe open file and seek 2 int instead of counting on it */
	row = (double*)malloc(n * sizeof(double));
	assert(row != NULL);
	for(i = 0; i < n; i++){
		read_result = fread(row, sizeof(double), n, input_matrix);
		assert(read_result = n);
		sp_mat->add_row(sp_mat, row, i);
	}
	free(row);
}

int close_vectors(double* vector_a, double* vector_b, double epsilon, int n){
	int i;
	for (i = 0; i < n; i++){
		if(fabs(vector_a[i] - vector_b[i]) > epsilon){
			return -1;
		}
	}
	return 0;
}

double dot_product(double* vector1, double* vector2, int vector_size){
	double sum = 0;
	int i;

	for(i=0; i<vector_size; i++){
		sum += vector1[i] * vector2[i];
	}

	return sum;
}

double* power_iteration(double* b0, spmat* sp_mat, double epsilon, int n){
	double* b_prev;
	double* b_next;
	double* temp;
	double norm = 0;
	int i;

	b_prev = b0;
	b_next = (double*)malloc(n * sizeof(double));
	assert(b_next != NULL);
	while(close_vectors(b_prev, b_next, epsilon, n) != 0){
		sp_mat->mult(sp_mat, b_prev, b_next);
		norm = sqrt(dot_product(b_next, b_next, n));
		for(i = 0; i < n; i++){
			b_next[i] /= norm;
		}
		temp = b_prev;
		b_prev = b_next;
		b_next = temp;
	}
	free(b_next);
	return b_prev;
}

int main(int argc, char* argv[]){
	FILE* input_matrix = NULL;
	FILE* output_file = NULL;
	FILE* input_b0 = NULL;
	spmat* sp_mat;
	double* b0;
	double* eigen_vector;
	int n = 0;
	int nnz = 0;
	int read_result = 0;
	int write_result = 0;
	int close_result = 0;
	char* implementation;
	int argv_offset = 0;
	int col_size;
	clock_t start, end;

	start = clock();

	/* Asserting num of args */
	assert(argc == 4 || argc == 5);

	/* Opening input matrix file */
	input_matrix = fopen(argv[1], "r");
	assert(input_matrix != NULL);
	/* Reading matrix size twice */
	read_result = fread(&n, sizeof(int), 1, input_matrix);
	assert(read_result == 1);
	read_result = fread(&n, sizeof(int), 1, input_matrix);
	assert(read_result == 1);

	/* Reading or randomizing b0 */

	b0 = (double*)malloc(n * sizeof(double));
	assert(b0 != NULL);
	/* If b0 is passed as argument */
	if (argc == 5){
		argv_offset = 1;
		input_b0 = fopen(argv[2], "r");
		assert(input_b0 != NULL);
		read_initial_vector(input_b0, b0, n);
		close_result = fclose(input_b0);
		assert(close_result == 0);
	} /* Else randomizing b0 */
	else {
		create_random_vector(b0, n);
	}

	/* Reading output file, implementation */
	output_file = fopen(argv[2 + argv_offset], "w");
	assert(input_matrix != NULL);
	implementation = argv[3 + argv_offset];
	assert((strcmp(implementation,"-array") == 0) | (strcmp(implementation,"-list") == 0));

	/* Creating sparse matrix according to implementation */
	if(strcmp(implementation,"-array") == 0){
		nnz = count_nnz_values(input_matrix, n);
		sp_mat = spmat_allocate_array(n, nnz);
	}
	else{
		sp_mat = spmat_allocate_list(n);
	}

	/* Reading rows of input matrix to sparse matrix*/
	read_matrix_rows(sp_mat, input_matrix, n);
	fclose(input_matrix);

	eigen_vector = power_iteration(b0, sp_mat, 0.00001, n);

	/* Writing col and row size */
	col_size = 1;
	write_result = fwrite(&col_size, sizeof(int), 1, output_file);
	assert(write_result == 1);
	write_result = fwrite(&n, sizeof(int), 1, output_file);
	assert(write_result == 1);

	/* Writing eigen vector to file */
	write_result = fwrite(eigen_vector, sizeof(double), n, output_file);
	assert(write_result == n);
	fclose(output_file);

	sp_mat->free(sp_mat);
	free(eigen_vector);

	end = clock();
	printf("Execution took %f seconds\n", ((double)(end-start) / CLOCKS_PER_SEC));

	return 0;
}
