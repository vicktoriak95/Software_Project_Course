/*
 * spmat.c
 *
 *  Created on: May 2, 2020
 *      Author: User
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "spmat.h"

/* TODO - use only necessary includes in both files*/
/* TODO - comment everything*/
/* TODO - Check free & assert for all alloc*/
/* TODO - Check close for all open - and assert to both*/
/* TODO - Check time of run*/

/**** List implementation ****/

typedef struct {
    double value;
    int col_index;
    struct Node* next;
} Node;

Node* list_create_node(double value, int col_index){
	Node* node;
	node = (Node *)malloc(sizeof(Node));
	assert(node != NULL);

	node->value = value;
	node->col_index = col_index;
	node->next = NULL;

	return node;
}

void list_free_list(Node* list){
	if (list != NULL){
		list_free_list((Node*)list->next);
		free(list);
	}
}

void list_add_row(struct _spmat *A, const double *row, int i){
	Node* tail = NULL;
	Node* node = NULL;
	int j;

	/* Iterating over A values */
	for(j = 0; j < A->n; j++){
		if(row[j] != 0){
			/* Creating node for non zero value */
			node = list_create_node(row[j], j);

			/* List is empty - first node appended */
			if(tail == NULL){
				((Node** )A->private)[i] = node; /* TODO: UPDATE OF TALE?? */
			}
			/* List is not empty */
			else {
				tail->next = (struct Node*)node;
			}
			tail = node;
		}
	}
}

void list_free(struct _spmat *A){
	int i;

	for(i = 0; i< A->n; i++){
		list_free_list(((Node** )A->private)[i]);
	}
	free(A->private);
	free(A);
}

void list_mult(const struct _spmat *A, const double *v, double *result){
	int i;
	double dot_product;
	Node* head;

	/* Iterating over A rows */
	for(i = 0; i < A->n; i++){
		dot_product = 0;
		head = ((Node** )A->private)[i];
		while(head != NULL){
			dot_product += (head->value) * (v[head->col_index]);
			head = (Node*)head->next;
		}
		result[i] = dot_product;
	}
}

spmat* spmat_allocate_list(int n){
	spmat* A;
	Node** private;

	A = (spmat*)malloc(sizeof(spmat));
	assert(A != NULL);

	/* Using calloc to assure all pointers are NULL */
	private = (Node**)calloc(n, sizeof(Node*));
	assert(private != NULL);

	A->n = n;
	A->add_row = &list_add_row;
	A->free = &list_free;
	A->mult = &list_mult;
	A->private = private;

	return A;
}


/*** Array implementation ***/


typedef struct {
	int nnz_counter;
    double* values;
    int* col_index;
    int* row_ptr;
} array_mat;

void array_add_row(struct _spmat *A, const double *row, int i){
	int row_nnz = 0;
	int j;
	double* values = ((array_mat*)A->private)->values;
	int* col_index = ((array_mat*)A->private)->col_index;
	int* row_ptr = ((array_mat*)A->private)->row_ptr;
	int* nnz_counter = &(((array_mat*)A->private)->nnz_counter);

	for(j = 0; j < A->n; j++){
		if(row[j] != 0){
			/* update values */
			values[*nnz_counter] = row[j];
			/* update col_ind */
			col_index[*nnz_counter] = j;
			/* update nnz_counter */
			*nnz_counter += 1;

			row_nnz += 1;
		}
	}
	/* update row_ptr */
	row_ptr[i + 1] = row_ptr[i] + row_nnz;
}

void array_free(struct _spmat *A){
	free(((array_mat*)A->private)->values);
	free(((array_mat*)A->private)->col_index);
	free(((array_mat*)A->private)->row_ptr);
	free(A->private);
	free(A);
}

void array_mult(const struct _spmat *A, const double *v, double *result){
	int i;
	int values_index;
	double dot_product;
	double* values = ((array_mat*)A->private)->values;
	int* col_index = ((array_mat*)A->private)->col_index;
	int* row_ptr = ((array_mat*)A->private)->row_ptr;

	for(i = 0; i < A->n; i++){
		dot_product = 0;
		for(values_index = row_ptr[i]; values_index < row_ptr[i + 1]; values_index++){
			dot_product += values[values_index] * v[col_index[values_index]];
		}
		result[i] = dot_product;
	}
}


spmat* spmat_allocate_array(int n, int nnz){
	spmat* A;
	array_mat* private;
	double* values = NULL;
	int* col_index = NULL;
	int* row_ptr = NULL;

	A = (spmat*)malloc(sizeof(spmat));
	assert(A != NULL);

	values = (double*)malloc(nnz * sizeof(double));
	assert(values != NULL);
	col_index = (int*)malloc(nnz * sizeof(int));
	assert(col_index != NULL);
	row_ptr = (int*)malloc((n + 1) * sizeof(int));
	assert(row_ptr != NULL);

	private = (array_mat*)malloc(sizeof(array_mat));
	assert(private != NULL);
	private->nnz_counter = 0;
	private->values = values;
	private->col_index = col_index;
	private->row_ptr = row_ptr;
	private->row_ptr[0] = 0;

	A->n = n;
	A->add_row = &array_add_row;
	A->free = &array_free;
	A->mult = &array_mult;
	A->private = private;

	return A;
}

void test(){
	spmat* A;
	double matrix[4][4] = {{0,0,0,0},{0,0,1,5},{0,0,0,0},{4,0,2,0}};
	int i;
	double v[4] = {1,1,1,1};
	double result[4];

	/*A = spmat_allocate_list(4);*/
	A = spmat_allocate_array(4, 4);
	for(i = 0; i < 4; i++){
		A->add_row(A, matrix[i], i);
	}
	A->mult(A, v, result);
	A->free(A);

	printf("Vicki is the Best");
}
