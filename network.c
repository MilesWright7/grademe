// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <time.h>

// #define ETC 0.1
// #define ALPHA 0.5
// #define ETC_NEXT_CHAR 0.1
// #define ALPHA_NEXT_CHAR = 0.5

#include "network.h"

/* etc is how much change is made each iteration. .1 is good higher can cause traingin to
 * be impossible and lower can cause it to be needlessly slow.
 * ALPHA is how much of the previous change is used in the next one. must be between 0 and 1
 * and can also cause training to fail if the value is too extreme.
 *
 * Have to use -lm compile flag because of including math.h. 
 * Need to link to this library for some reason who knows why.
 */

// Helper functions ----------------

// Sigmoid working correctly 
double sigmoid(double in){
	return 1.0/(1.0 + exp((-1)*in));
}

double make_rand(double x0, double x1){
	// x0 is lower bound x1 is upper bound
	return x0 + (x1- x0)*rand()/((double)RAND_MAX);
}

int max_index(double *input, int size){
	double max = input[0];
	int index = 0;
	for(int i = 1; i < size; i++){
		// printf("max %f %d %f\n", max, i, input[i]);
		if(input[i] > max){
			max = input[i];
			index = i;
		}
	}
	return index;
}

//----------------------------
// network functions
// ---------------------------
void init_network(Network *net, int depth, int nodes){
	srand(time(0)); //set rand seed to be different every time
	net->layers = (double **)malloc(sizeof(double *) * depth);
	net->out_layer = (double *)malloc((sizeof(double) * depth) + 1);
	net->out_layer[depth] = make_rand(-1, 1);
	for(int i = 0; i < depth; i++){
		net->layers[i] = (double *)malloc((sizeof(double) * nodes) + 1);
		net->layers[i][0] = make_rand(-1, 1); // this is the layer's bias
		
		net->out_layer[i] = make_rand(-1, 1);
		
		for(int j = 1; j <= nodes; j++){
			net->layers[i][j] = make_rand(-1, 1);
		}
	}
	net->num_layers = depth;
	net->synapses = nodes;
}

void load_network(Network *net, int rows, int cols, char *file_name){

	FILE *f_in;
	f_in = fopen(file_name, "r");
	if(f_in == NULL){
		init_network(net, rows, cols);
		return;
	
	}
	
	int num_layers = 0;
	int synapses = 0;

	// Get number of layers and synapses.
	fscanf(f_in, "%d", &num_layers);
	fscanf(f_in, "%d", &synapses);

	if(num_layers == rows && synapses == cols){
		printf("founds the weights file\n");
	}else{
		init_network(net, rows, cols);

		return ;
	}

	// Set the number of layers and synapses within the network struct.
	net->num_layers = num_layers;
	net->synapses = synapses;

	// Allocate memory with the number of layers.
	net->layers = (double **)malloc(sizeof(double *) * num_layers);
	net->out_layer = (double *)malloc((sizeof(double) * (num_layers + 1)));

	net->out_layer[num_layers] = 0.0;

	int i, j;
	double num = 0.0;

	for(i = 0; i < num_layers; i ++){
		net->layers[i] = (double *)malloc((sizeof(double) * (synapses + 1)));
		// net->layers[i][synapses] = make_rand(-1, 1);
		for(j = 0; j <= synapses; j ++){
			// If there is a weights file, load the saved weights.
			fscanf(f_in, "%lf", &num);
			net->layers[i][j] = num;
		}
	}
	
	for(i = 0; i <= num_layers; i ++){
		fscanf(f_in, "%lf", &num);
		net->out_layer[i] = num;
	}
}

void free_network(Network *net){

	for(int i = 0; i < net->num_layers; i++){
		free(net->layers[i]);
	}
	free(net->layers);
	free(net->out_layer);
	free(net);
}

void print_network(Network *net){
	printf("This network has %d layers and %d nodes in each layer\n", net->num_layers, net->synapses);
	for(int i = 0; i < net->num_layers; i++){
		printf("Layer %d has weights:\n", i);
		for(int j = 0; j <= net->synapses; j++){
			printf("%0.2f ",net->layers[i][j]);
		}
		
		printf("\n");
	}
	printf("The output layer has weights:\n");
	for(int i = 0; i <= net->num_layers; i++){
		printf("%0.2f ", net->out_layer[i]);
	}
	printf("\n");
}

// real functions
// --------------------------
void train_network(Network *net, int iterations, int num_inputs, double **input, double *target){
	int rows = net->num_layers;
	int cols = net->synapses;
	double outputs[num_inputs];
	double sum; // boring way to store sums
	double delta_outer[num_inputs];
	double delta_outer_weight[rows + 1];
	double delta_inner[rows]; // way to store how erronious code is to use in back propagation
	double delta_inner_weight[rows][cols + 1]; // stores changes to be made to the weights
	double row_out[rows];
	double out;
	double error;
	double eta = ETC; // % change made each weight update -- larger values can be ok, so can smaller.
					   // about .1 is good. lower is ok but higher than .2 seems to get the thing stuck.
	double alpha = ALPHA; // how important the previous change was to this change
						// must be between [0,1]. too small and training is really slow, to fast and 
						// values can swing wildly and prevent training at all.

	// initialise all weighs to be 0 as the previous change is used in this algorithm to update later weights
	delta_outer_weight[rows] = 0.0;
	for(int j = 0; j < rows; j++){
		delta_outer_weight[j] = 0.0;
		for(int i = 0; i < (cols + 1); i++){

			delta_inner_weight[j][i] = 0.0;
		}
	}

	for(int i = 0; i < iterations; i++){
		
		error = 0.0; // can be used a stop point for how close we need to get to target outputs 
		
		for(int n = 0; n < num_inputs; n++){
			// in_vector = input[n]
			// in_target = target[n]

			for(int j = 0; j < rows; j++){
	
				sum = net->layers[j][net->synapses]; // sum initially = bias
				for(int k = 0; k < cols; k++){
					
					sum += input[n][k] * net->layers[j][k];
				}
				row_out[j] = sigmoid(sum); // sum is an array of outputs for each row.
				// row_out[j] is a value [0, 1] giving us the output of each row
			}
			
			out = net->out_layer[rows]; // outer layer bias
			for(int j = 0; j < rows; j++){
				out += row_out[j] * net->out_layer[j];
			}

			outputs[n] = sigmoid(out);
			error += 0.5 * (outputs[n] * target[n]) * (outputs[n] * target[n]);
			delta_outer[n] = (target[n] - outputs[n]) * outputs[n] * (1 - outputs[n]);

			
			for(int j = 0; j < rows; j++){
				sum = 0.0; // this is the sum of the errors in each row
				sum += net->out_layer[j] * delta_outer[n];
				delta_inner[j] = sum * row_out[j] * (1 - row_out[j]);
			}
			// updating weights for inner layers
			for(int j = 0; j < rows; j++){
				delta_inner_weight[j][cols] = eta * delta_inner[j] + alpha * delta_inner_weight[j][cols];
				net->layers[j][cols] += delta_inner_weight[j][cols];
				for(int k = 0; k < cols; k++){
					delta_inner_weight[j][k] = eta * delta_inner[j] + alpha * delta_inner_weight[j][k];
					net->layers[j][k] += delta_inner_weight[j][k];
				}
			
			}
			// updating weights for outer layer.
			delta_outer_weight[rows] = eta * delta_outer[n] + alpha * delta_outer_weight[rows];
			net->out_layer[rows] += delta_outer_weight[rows];
			for(int j = 0; j < rows; j++){
				delta_outer_weight[j] = eta * delta_outer[n] + alpha * delta_outer_weight[j];
				net->out_layer[j] += delta_outer_weight[j];
				
			}

			
		}
		

	}
}

void train_network_next_char(Network *net, int iterations, int num_inputs, int *input){
	// input will be of form [index char 1, index char 2, ....] total of 97 chars
	// num inputs will be the total chars in the whole input
	int rows = net->num_layers; 
	int cols = net->synapses; // must be 97
	int total_chars = TOTAL_CHARS; // should match rows or net->num_layers
	int index; // maybe unused?
	int target;  // value of the target character
	
	int distance = DISTANCE; // the number of previous characters used in the creation of the next character
    int i, j, k, l, n;	
	double sum; // boring way to store sums
	double delta_row[total_chars];
	double delta_all_row[rows]; 
	double delta_row_weight[rows][cols + 1]; 
	double row_out[total_chars];
	double out[total_chars];
	double error;
	double eta = ETC_NEXT_CHAR; // % change made each weight update -- larger values can be ok, so can smaller.
					   // about .1 is good. lower is ok but higher than .2 seems to get the thing stuck.
	double alpha = ALPHA_NEXT_CHAR; // how important the previous change was to this change
						// must be between [0,1]. too small and training is really slow, to fast and 
						// values can swing wildly and prevent training at all.

	// initialise all weighs to be 0 as the previous change is used in this algorithm to update later weights
	for(j = 0; j < rows; j++){
		for(i = 0; i < (cols + 1); i++){

			delta_row_weight[j][i] = 0.0;
		}
	}


	for(i = 0; i < iterations; i++){
		
		error = 0.0; // can be used a stop point for how close we need to get to target outputs 
		
		
		// loop through each input	
		for(n = 0; n < num_inputs - distance; n++){
			// in_matrix = input[n]
			// in_target_vector = target[n]
			target = input[n + distance];  

			for(j = 0; j < rows; j++){
				sum = 0.0;
				sum = net->layers[j][cols]; // sum initially = bias
				// go through each of the upcomming letters and weight them so the letters close to the 
				// character about to be predicted have more weight and those further have less weight
			
				// current method used to weight = linear (kinda). can experiment with different things later.

				for(l = 0; l < distance; l++){
					// each run should add the value of net-layers[j][input[n + l]]
					sum += ((l + 1) * net->layers[j][input[n + l]]) / (distance + 1);

				}
				row_out[j % total_chars] += sum; // old --- sigmoid(sum); // sum is an array of outputs for each row.
			}
			
			for(k = 0; k < total_chars; k++){
				out[k] = sigmoid(row_out[k]);
			   	// out is now a vector of probablities of each letter in their position
				if(k == target){
					delta_row[k] = 1 - out[k];

				} else {
					delta_row[k] = 0 - out[k];

				}
				error += 0.5 * delta_row[k] * delta_row[k];
			}
			// delta row now contains the desired change in answer for each character's probablility

		    //error += 0.5 * (out - target[n]) * (out[n] - target[n]); // error is not the stop for this funciotn yet
			// delta_outer[n] = (target[n] - outputs[n]) * outputs[n] * (1 - outputs[n]); // outer layer not used in this function
			
			for(j = 0; j < rows; j++){
				k = j % total_chars;
				delta_all_row[j] = delta_row[k];
			}
		
			// updating weights for inner layers
			for(j = 0; j < rows; j++){
				delta_row_weight[j][cols] = eta * delta_all_row[j] + alpha * delta_row_weight[j][cols];
				net->layers[j][cols] += delta_row_weight[j][cols]/(distance + 1);
				for(l = 0; l < distance; l++){
					index = input[n + l];
					delta_row_weight[j][index] = eta * delta_all_row[j] + alpha * delta_row_weight[j][index];
					net->layers[j][k] += ((l+1) * delta_row_weight[j][k]) / (distance + 1); // equation made my miles to weight the changes to the weights. ha
				}
			}
		

				
		}	
		printf("error for iteration %d is %f\n", i, error);

		// periodically save the weights
		if(i % 100 == 0 && i < 100){
			save_weights(net, "weights.txt");
	
		}
		
	}
	
	save_weights(net, "weights.txt");
}

void use_network_next_char(Network *net, int num_inputs, int *input, int *res, int total_res){
	// uses the network to propogate res with the outputs given by the network for the inputs
	int rows = net->num_layers;
	int cols = net->synapses;
	int total_chars = TOTAL_CHARS;
	int distance = DISTANCE;
	double sum;
	int index;
	//double out;
	double row_out[total_chars];
	
	// starting with exactly the last (distance) elements of our input we make characters	
	for(int n = (num_inputs - (distance + 1)); n < num_inputs; n++){
		
		// clean row_out   haha
		for(int p = 0; p < total_chars; p++){
			row_out[p] = 0.0;
		}
		
		for(int j = 0; j < rows; j++){

			sum = net->layers[j][cols];
			for(int l = 0; l < distance; l++){
				if((l + n) < num_inputs){
					index = input[n + l];
					sum += ((l + 1) * net->layers[j][index]) / (distance + 1);

				} else { // we need to take chars from our results to keep making chars
					index = res[(l + n) - num_inputs];
					sum += ((l + 1) * net->layers[j][index]) / (distance + 1);
				}

			}
			row_out[j % total_chars] += sigmoid(sum);
		}
		res[(distance + 1 + n) - num_inputs] = max_index(row_out, total_chars);
	}

	// using only generated characters make some more.
	for(int n = 0; n < total_res - distance; n++){
		
		// clean row_out   haha
		for(int p = 0; p < total_chars; p++){
			row_out[p] = 0.0;
		}
		
		for(int j = 0; j < rows; j++){

			sum = net->layers[j][cols];
			for(int l = 0; l < distance; l++){
				index = res[n];
				sum += ((l + 1) * net->layers[j][index]) / (distance + 1);

			}

			row_out[j % total_chars] += sigmoid(sum);
		}
		res[n + distance] = max_index(row_out, total_chars);
	}

	
	
}	

// needs completely revamped ASDJKFASDF'"A"FA"FA"FD"AF"A"F"
void use_network(Network *net, int num_inputs, double **input, double *res){
	// uses the network to propogate res with the outputs given by the network for the inputs
	int rows = net->num_layers;
	int cols = net->synapses;
	double sum;
	double out;
	double row_out[rows];
	for(int n = 0; n < num_inputs; n++){
		// in_vector = input[n]
		// in_target = target[n]

		for(int j = 0; j < rows; j++){
	
			sum = net->layers[j][cols]; // sum initially = bias
			for(int k = 0; k < cols; k++){
					
				sum += input[n][k] * net->layers[j][k];
			}
			row_out[j] = sigmoid(sum); // sum is an array of outputs for each row.
			// row_out[j] is a value [0, 1] giving us the output of each row
		}
			
		out = net->out_layer[rows];
		for(int j = 0; j < rows; j++){
			out += row_out[j] * net->out_layer[j];
		}
		res[n] = sigmoid(out);
	}
	
}	

// Save weights to a file.
void save_weights(Network *n, char *file_name){
	printf("saving the weights!\n");
	int i, j;

	FILE *weights_file;
	weights_file = fopen(file_name, "w");
	

	fprintf(weights_file, "%d %d\n", n->num_layers, n->synapses);

	for(i = 0; i < n->num_layers; i ++){
		for(j = 0; j <= n->synapses; j ++){
			fprintf(weights_file, "%f ", n->layers[i][j]);
		}
		fprintf(weights_file, "\n");
	}
	for(i = 0; i <= n->num_layers; i++){
		fprintf(weights_file, "%f ",n->out_layer[i]);
	}
	fprintf(weights_file, "\n");
	fclose(weights_file);
	printf("weights sucessfully saved!\n");
}

void read_training_data(double **test_input, double **input){

	FILE *fp = fopen("training_data.txt", "r");

	int i, j;
	for(i = 0; i < 8; i ++){
		for(j = 0; j < 2; j ++){
			fscanf(fp, "%lf", &test_input[i][j]);
		}
	}

	fclose(fp);
	
}




// --------------------------
// main * to be deleted later once everything in here works
/*
int main(){
	* testing sigmoid 
	double input = 123.1;
	double out;
	out = sigmoid(input);
	printf("sigmoid value is %f\n", out);
    */ 	
	// Network *net;
	// init_network(net, 10, 10);
	// print_network(net);

	// save_weights(net);
	/*
	double **test_input = (double **)malloc(sizeof(double*) * 4);
	for(int i = 0; i < 4; i++){
		test_input[i] = (double *)malloc(sizeof(double) * 2);
	}
	test_input[0][0] = 1;
	test_input[0][1] = 0;
	test_input[1][0] = 0;
	test_input[1][1] = 1;
	test_input[2][0] = 0;
	test_input[2][1] = 0;
	test_input[3][0] = 1;
	test_input[3][1] = 1;
	

	double test_answers[] = {1, 1, 0, 0};

	double **input = (double **)malloc(sizeof(double*) * 4);
	for(int i = 0; i < 4; i++){
		input[i] = (double *)malloc(sizeof(double) * 2);
	}
	input[0][0] = 0;
	input[0][1] = 1;
	input[1][0] = 1;
	input[1][1] = 1;
	input[2][0] = 0;
	input[2][1] = 0;
	input[3][0] = 1;
	input[3][1] = 0;
	double desired_res[] = {1, 0, 0, 1};
	double res[4];


	load_network(net, 300, 2); // seems to work best with 300 layers. 1000 too much and 10 too few.

	use_network(net, 4, input, res);
	for(int i = 0; i < 4; i++){
		printf("desired result: %0.2f\tresult from net: %0.2f\n", desired_res[i], res[i]);
	}

	printf("!!!!!!!!!!!!!\n");

	// print_network(net);
	// void train_network(Network *net, int iterations, int num_inputs, double **input, double *target){
	train_network(net, 10000, 4, test_input, test_answers);
	// void use_network(Network *net, int num_inputs, double ** input, double *res){
	use_network(net, 4, input, res);
	for(int i = 0; i < 4; i++){
		printf("desired result: %0.2f\tresult from net: %0.2f\n", desired_res[i], res[i]);
	}
    *
	save_weights(net);
 	free_network(net);
}
*/
