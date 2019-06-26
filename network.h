#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ETC 0.1
#define ALPHA 0.5
#define ETC_NEXT_CHAR 0.3
#define ALPHA_NEXT_CHAR 0.3
#define TOTAL_CHARS 126
#define DISTANCE 100

typedef struct{
    int num_layers, synapses;
    double **layers;
    double *out_layer;
} Network;

double sigmoid(double in);
int max_index(double *input, int size);
double make_rand(double x0, double x1);
void init_network(Network *net, int depth, int nodes);
void load_network(Network *net, int rows, int cols, char *file_name);
void free_network(Network *net);
void print_network(Network *net);
void train_network(Network *net, int iterations, int num_inputs, double **input, double *target);
void train_network_next_char(Network *net, int iterations, int num_inputs, int *input);
void use_network(Network *net, int num_inputs, double **input, double *res);
void use_network_next_char(Network *net, int num_inputs, int *input, int *res, int total_res);
void save_weights(Network *n, char *file_name);
void read_training_data(double **test_input, double **input);

#endif
