#ifndef _READ_DATA_H_
#define _READ_DATA_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_file(char **buffer, int *file_size){

    char *file_path = "big.txt";

    int temp_file_size = 0;

    FILE *fp = fopen(file_path, "r");
    fseek(fp, 0, SEEK_END);
    temp_file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *temp = (char *)malloc(sizeof(char) * temp_file_size + 1);
    int i = 0;
    fread(temp, 1, temp_file_size, fp);

    *buffer = temp;
    *file_size = temp_file_size;
}

void set_up_input_vector(char *file_contents, int **vector, int size){
    int *tv = (int *)malloc(sizeof(int) * size);
    int i, b;
    char a;
    for(i = 0; i < size; i ++){
        a = file_contents[i];
        b = (int)a;
        tv[i] = b;   
    }
    *vector = tv;
}

void convert_output_vector(char **output_text, int *vector, int size){
    char *temp_text = (char *)malloc(sizeof(char) * size + 1);
    
    int i, a;
    char b;
    for(i = 0; i < size; i ++){
        a = vector[i];
        if(a > 9){
            b = (char)a;
            temp_text[i] = b;
        }else{
            temp_text[i] = 'z';
        }
        
    }
    temp_text[size] = '\0';
    *output_text = temp_text;
}

#endif
