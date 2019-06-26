#include <math.h>
#include "network.h"
#define INPUT_SIZE 127 // size of input vector
#define OUTPUT_SIZE 127 // size of output vector
#define LEARNING_RATE 1

// lstm takes input vector, output from last node, input state from last node, 
// need space to store the 
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


double sigmoid_derivative(double input){
	return input * (1 - input);
}

double tanh_derivative(double input){
	return 1. - (input) * (input);
}


void init_lstm_char(LSTM *lstm, int input_size, int output_size){

	
	lstm->forget_input = (Network *)malloc(sizeof(Network));
	lstm->forget_previous = (Network *)malloc(sizeof(Network));
	lstm->out_input = (Network *)malloc(sizeof(Network));
	lstm->out_previous = (Network *)malloc(sizeof(Network));
	lstm->in_input = (Network *)malloc(sizeof(Network));
	lstm->in_previous = (Network *)malloc(sizeof(Network));
	lstm->cell_input = (Network *)malloc(sizeof(Network));
	lstm->cell_previous = (Network *)malloc(sizeof(Network));

	load_network(lstm->forget_input, output_size, input_size, "weights/forget_in_weights.txt");
	load_network(lstm->forget_previous, output_size, output_size, "weights/forget_prev_weights.txt");
	load_network(lstm->out_input, output_size, input_size, "weights/out_in_weights.txt");
	load_network(lstm->out_previous, output_size, output_size, "weights/out_prev_weights.txt");
	load_network(lstm->in_input, output_size, input_size, "weights/in_in_weights.txt");
	load_network(lstm->in_previous, output_size, output_size, "weights/in_prev_weights.txt");
	load_network(lstm->cell_input, output_size, input_size, "weights/cell_in_weights.txt");
	load_network(lstm->cell_previous, output_size, output_size, "weights/cell_prev_weights.txt");
	
	lstm->cell_state = (double *)malloc(sizeof(double) * output_size);

	lstm->input_size = input_size;
	lstm->output_size = output_size;
}
	

void free_lstm(LSTM *lstm){
	
	free_network(lstm->forget_input);
	free_network(lstm->forget_previous);

	free_network(lstm->out_input);
	free_network(lstm->out_previous);
	
	free_network(lstm->in_input);
	free_network(lstm->in_previous);
	
	free_network(lstm->cell_input);
	free_network(lstm->cell_previous);
	
	free(lstm->cell_state);
	free(lstm);
}

void train_lstm_char(LSTM *lstm, int iterations, int *input, int num_inputs){
	
	double *prev_output = (double *)malloc(sizeof(double) * lstm->output_size);
	int batch_size = 500;
	double **prev_cell_state = (double **)malloc(sizeof(double *) * batch_size);
	double **forget_activation = (double **)malloc(sizeof(double *) * batch_size);
	double **send_in = (double **)malloc(sizeof(double *) * batch_size);
	double **send_out = (double **)malloc(sizeof(double *) * batch_size);
	double **send_cell = (double **)malloc(sizeof(double *) * batch_size);
	double **output = (double **)malloc(sizeof(double *) * batch_size);

	double sum;
	int i, j, k, l, n, error, row, col, cur_input, next_input;
	for(i = 0; i < batch_size; i++){
		prev_cell_state[i] = (double *)malloc(sizeof(double) *lstm->output_size);
		forget_activation[i] = (double *)malloc(sizeof(double) *lstm->output_size);
		send_in[i] = (double *)malloc(sizeof(double) *lstm->output_size);
		send_out[i] = (double *)malloc(sizeof(double) *lstm->output_size);
		send_cell[i] = (double *)malloc(sizeof(double) *lstm->output_size);
		output[i] = (double *)malloc(sizeof(double) *lstm->output_size);
		
	}

	for(l = 0; l < iterations; l++){
		
		// initialize cell state to 0
		for(n = 0; n < lstm->output_size; n++){
			
			lstm->cell_state[n] = 0;
			
		}
		if(l % 100 == 0){
			if(l != 0){
				save_lstm_weights(lstm);
			}
		}

		printf("%0.2f%%\n", (double)100 * l / iterations);
		
		
		for(n = 0; n < num_inputs - 1; n++){
			cur_input = input[n];

			for(row = 0; row < lstm->output_size; row++){

				// set previous cell state 
				prev_cell_state[n % batch_size][row] = lstm->cell_state[row];
				
				// forget gate activation
				//sum = lstm->forget_input->layers[row][lstm->input_size];
				sum = lstm->forget_input->layers[row][cur_input];
				for(col = 0; col < lstm->output_size; col++){
					sum += lstm->forget_previous->layers[row][col] * prev_output[col];
				}
				forget_activation[n % batch_size][row] = sigmoid(sum);
					
				// how much to weight each input to cell state/ output
				//sum = lstm->in_input->layers[row][lstm->input_size];
				sum = lstm->in_input->layers[row][cur_input];
				for(col = 0; col < lstm->output_size; col++){
					sum += lstm->in_previous->layers[row][col] * prev_output[col];
				}
				send_in[n % batch_size][row] = sigmoid(sum);

				// what is being sent out to calculate the output of the lstm cell
				//sum = lstm->out_input->layers[row][lstm->input_size];
				sum = lstm->out_input->layers[row][cur_input];
				for(col = 0; col < lstm->output_size; col++){
					sum += lstm->out_previous->layers[row][col] * prev_output[col];
				}
				send_out[n % batch_size][row] = sigmoid(sum);

				// values to send to cell state
				//sum = lstm->cell_input->layers[row][lstm->input_size];
				sum = lstm->cell_input->layers[row][cur_input];
				for(col = 0; col < lstm->output_size; col++){
					sum += lstm->cell_previous->layers[row][col] * prev_output[col];
				}
				send_cell[n % batch_size][row] = tanh(sum);

				// update cell state
				lstm->cell_state[row] = forget_activation[n % batch_size][row] * prev_cell_state[n % batch_size][row] + send_in[n % batch_size][row] * send_cell[n % batch_size][row];
				output[n % batch_size][row] = send_out[n % batch_size][row] * tanh(lstm->cell_state[row]);

					
			}

			// ###############################
			// error stuff and backpropagaion
			// ###############################

			// sigmoid derivitave is (output) * (1 - output)
			// tanh derivitave is 1 - (output)(output)
				
			// each time the batch is full we go ahead and let the errorrs go
			if(n % batch_size == (batch_size - 1)){

				double d_forget[lstm->output_size];
				double d_input[lstm->output_size];
				double d_cell[lstm->output_size];
				double d_output[lstm->output_size];
				double d_state[lstm->output_size];
				double diff_cell[lstm->output_size];
				double diff_h[lstm->output_size];
					
				int index_pred = batch_size - 1;
				int index_real = n;

				for(i = 0; i < lstm->output_size; i++){
					diff_cell[i] = 0;
				}

				while(index_pred >= 0){
					
					for(i = 0; i < lstm->output_size; i++){
						if(i == input[index_real + 1]){
							diff_h[i] += 2 * (output[index_pred][i] - 1);
						}
						else {
							diff_h[i] += 2 * (output[index_pred][i] - 0);
						}
					
					}
					
					for(i = 0; i < lstm->output_size; i++){
						d_state[i] = send_out[index_pred][i] * diff_h[i] + diff_cell[i];
						d_output[i] = prev_cell_state[index_pred][i] * diff_h[i];
					    d_input[i] = send_cell[index_pred][i] * d_state[i];
						d_cell[i] = send_in[index_pred][i] * d_state[i];
							
						//first cell didnt have a previous cell state
						d_forget[i] = 0;
						if(i != 0){
							d_forget[i] = prev_cell_state[index_pred][i - 1] * d_state[i];	
						}

						d_input[i] = d_input[i] * sigmoid_derivative(send_in[index_pred][i]);
						d_forget[i] = d_forget[i] * sigmoid_derivative(forget_activation[index_pred][i]);
						d_output[i] = d_output[i] * sigmoid_derivative(send_out[index_pred][i]);
						d_cell[i] = d_cell[i] * tanh_derivative(send_cell[index_pred][i]);

					}

					for(row = 0; row < lstm->output_size; row++){
						
						// bias terms
						lstm->forget_input->layers[row][lstm->output_size] += LEARNING_RATE * d_forget[row];
						lstm->in_input->layers[row][lstm->output_size] += LEARNING_RATE * d_input[row];
						lstm->out_input->layers[row][lstm->output_size] += LEARNING_RATE * d_input[row];
						lstm->cell_input->layers[row][lstm->output_size] += LEARNING_RATE * d_input[row];

						// input matricies
						lstm->forget_input->layers[row][col] += LEARNING_RATE * d_forget[row] * input[index_real];
						lstm->in_input->layers[row][col] += LEARNING_RATE * d_input[row] * input[index_real];
						lstm->out_input->layers[row][col] += LEARNING_RATE * d_output[row] * input[index_real];
						lstm->cell_input->layers[row][col] += LEARNING_RATE * d_cell[row] * input[index_real];

						// matricies dealing in the past life
						if(index_real != 0){	
							lstm->forget_previous->layers[row][col] += LEARNING_RATE * d_forget[row] * input[index_real - 1];
							lstm->in_previous->layers[row][col] += LEARNING_RATE * d_input[row] * input[index_real - 1];
							lstm->out_previous->layers[row][col] += LEARNING_RATE * d_output[row] * input[index_real - 1];
							lstm->cell_previous->layers[row][col] += LEARNING_RATE * d_cell[row] * input[index_real - 1];
						}

					}

					// keep the error going 
					for(i = 0; i < lstm->output_size; i++){
						diff_cell[i] = forget_activation[index_pred][i] * d_state[i];
						diff_h[i] = 0;
					}
						
					// go to next cycle
					index_pred --;
					index_real --;

				}

			}
	
		
	
		
		}

	
	}	

	// free every thing

	for(i = 0; i < batch_size; i++){
		free(forget_activation[i]);
		free(send_in[i]);
		free(send_out[i]);
		free(send_cell[i]);
		free(output[i]);
		free(prev_cell_state[i]);
	}
	save_lstm_weights(lstm);
	free(forget_activation);
	free(send_in);
	free(send_out);
	free(send_cell);
	free(output);
	free(prev_output);
	free(prev_cell_state);
}

void use_lstm_char(LSTM *lstm, int input, int *output, int output_size){
	
	double *forget_activation = (double *)malloc(sizeof(double) * lstm->output_size);
	double *send_in = (double *)malloc(sizeof(double) * lstm->output_size);
	double *send_out = (double *)malloc(sizeof(double) * lstm->output_size);
	double *send_cell = (double *)malloc(sizeof(double) * lstm->output_size);
	double *output_row = (double *)malloc(sizeof(double) * lstm->output_size);
	double *prev_output = (double *)malloc(sizeof(double) * lstm->output_size);
	double *prev_cell_state = (double *)malloc(sizeof(double) * lstm->output_size);

	double sum;
	int n, row, col, index, cur_input; 

	for(n = 0; n < output_size; n++){
		if( n == 0){
			cur_input = input;
		}
		else {
				
			cur_input = output[n - 1];
		}

		for(row = 0; row < lstm->output_size; row++){

			// set previous cell state 
			prev_cell_state[row] = lstm->cell_state[row];
				
			// forget gate activation
			//sum = lstm->forget_input->layers[row][lstm->input_size];
			sum = lstm->forget_input->layers[row][cur_input];
			for(col = 0; col < lstm->output_size; col++){
				sum += lstm->forget_previous->layers[row][col] * prev_output[col];
			}
			forget_activation[row] = sigmoid(sum);
					
			// how much to weight each input to cell state/ output
			//sum = lstm->in_input->layers[row][lstm->input_size];
			sum = lstm->in_input->layers[row][cur_input];
			for(col = 0; col < lstm->output_size; col++){
				sum += lstm->in_previous->layers[row][col] * prev_output[col];
			}
			send_in[row] = sigmoid(sum);

			// what is being sent out to calculate the output of the lstm cell
			//sum = lstm->out_input->layers[row][lstm->input_size];
			sum = lstm->out_input->layers[row][cur_input];
			for(col = 0; col < lstm->output_size; col++){
				sum += lstm->out_previous->layers[row][col] * prev_output[col];
			}
			send_out[row] = sigmoid(sum);

			// values to send to cell state
			//sum = lstm->cell_input->layers[row][lstm->input_size];
			sum = lstm->cell_input->layers[row][cur_input];
			for(col = 0; col < lstm->output_size; col++){
				sum += lstm->cell_previous->layers[row][col] * prev_output[col];
			}
			send_cell[row] = tanh(sum);

			// update cell state
			lstm->cell_state[row] = forget_activation[row] * prev_cell_state[row] + send_in[row] * send_cell[row];
			output_row[row] = send_out[row] * tanh(lstm->cell_state[row]);
		}

		index = max_index(output_row, lstm->output_size);
		output[n] = index;
	}
	
	free(forget_activation);
	free(send_in);
	free(send_out);
	free(send_cell);
	free(output_row);
	free(prev_output);
	free(prev_cell_state);
}

void save_lstm_weights(LSTM *lstm){

	save_weights(lstm->forget_input, "weights/forget_in_weights.txt");
	save_weights(lstm->forget_previous, "weights/forget_prev_weights.txt");
	save_weights(lstm->out_input, "weights/out_in_weights.txt");
	save_weights(lstm->out_previous, "weights/out_prev_weights.txt");
	save_weights(lstm->in_input, "weights/in_in_weights.txt");
	save_weights(lstm->in_previous, "weights/in_prev_weights.txt");
	save_weights(lstm->cell_input, "weights/cell_in_weights.txt");
	save_weights(lstm->cell_previous, "weights/cell_prev_weights.txt");
}
/*
int main(){
	// to test this gamer moment.
	LSTM *lstm = (LSTM *)malloc(sizeof(LSTM));
	
	init_lstm_char(lstm, INPUT_SIZE, OUTPUT_SIZE);
	
	//save_lstm_weights(lstm);
	free_lstm(lstm);
}
*/
