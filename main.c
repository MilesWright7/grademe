#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "network.h"
#include "lstm.h"
#include "read_data.h"

int main(){

	// Data file.
	char *file_contents = NULL;
	int file_size = 0;
	read_file(&file_contents, &file_size);
	printf("%s\n", file_contents);

	int *input_vector = NULL;
	set_up_input_vector(file_contents, &input_vector, file_size);

	// char *output_text = NULL;
	// convert_output_vector(&output_text, input_vector, file_size);

	// printf("%s\n", output_text);
	
	LSTM *lstm = (LSTM *)malloc(sizeof(LSTM));

	init_lstm_char(lstm, INPUT_SIZE, OUTPUT_SIZE);
	int out_size = 500;
	int *output_vector = (int *)malloc(sizeof(int) * out_size);
	
	clock_t start, end;
	double total_time;
	start = clock();
	train_lstm_char(lstm, 1, input_vector, file_size);
	
	end = clock();
	total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("time taken %f\n", total_time);

	use_lstm_char(lstm, (int)'a', output_vector, out_size);


	char *output_text = NULL;
	convert_output_vector(&output_text, output_vector, out_size);

	// print output text 
	printf("\n--------------------\nGo getem computer!\n\n");
	printf("%s\n", output_text);

	printf("\n");

	// save_weights(net); // saving weights now happens inside of training periodically so if crashes 
	// we can atleast have some progress maybe ....
	free_lstm(lstm);
	free(file_contents);
	free(input_vector);
	free(output_text);
	free(output_vector);
	return 0;
}
