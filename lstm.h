#include <math.h>
#include"network.h"
#define INPUT_SIZE 127
#define OUTPUT_SIZE 127
#define LEARNING_RATE 1 
typedef struct {
	Network *forget_input;
	Network *forget_previous;	// sigmoid layer determining what to keep from previous cell state
	
	Network *out_input; // sigmoid layer for determining output with results from cell state
	Network *out_previous; // how much of cell state to use in output
	
	Network *in_input; // sigmoid -- determines activation ammount for each part of input
	Network *in_previous;

	Network *cell_input; // tanh activaiton layer -- evaluates the activation of each input
	Network *cell_previous;

	double *cell_state; // this is the vector of the state of the cell
	int input_size;
	int output_size;
} LSTM;

double sigmoid_derivative(double input);
double tanh_derivative(double input);
void init_lstm_char(LSTM *lstm, int input_size, int output_size);
void free_lstm(LSTM *lstm);
void train_lstm_char(LSTM *lstm, int iterations, int *input, int num_inputs);
void use_lstm_char(LSTM *lstm, int input, int *output, int output_size);
void save_lstm_weights(LSTM *lstm);

