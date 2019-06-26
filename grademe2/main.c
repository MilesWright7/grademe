#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 40

typedef struct {
    char *word;
    int id;
    int occ;
} UniqueWord;

void read_file(char *file_name, char **buffer, int *file_size);
int count_words(char *file_contents);
int get_file_words(char *file_contents, char ***word_list, int num_words);
int get_unique_words(char **word_list, UniqueWord ***list, int num_words);
void print_unique_word_list(UniqueWord **list, int length);
void write_to_file(UniqueWord **list, int length, char *filename);
int load_unique_words(UniqueWord ***list, char *filename);
void free_unique_word_list(UniqueWord **list, int length);

int main(int argc, char **argv){

    char *file_name = "big.txt";
    char *file_contents;
    int file_size;

    read_file(file_name, &file_contents, &file_size);
    // Get rough count of words in file.
    int num_words = count_words(file_contents);

    // // Make a list of the words and get the real count of words.
    char **word_list;
    num_words = get_file_words(file_contents, &word_list, num_words);

    // printf("number of words: %d\n", num_words);

    // // Make list of unique word structs from the list of all the words.
    UniqueWord **words;
    int num_unique_words = get_unique_words(word_list, &words, num_words);

    print_unique_word_list(words, num_unique_words);

    // int max = 0;
    // int index = 0;
    // for(int i = 0; i < num_unique_words; i ++){
    //     if(max < words[i]->occ){
    //         max = words[i]->occ;
    //         index = i;
    //     }
    // }

    // printf("%s: %d\n", words[index]->word, max);

    // UniqueWord **words;
    // int num_unique_words = load_unique_words(&words, "words.tsv");
    // free_unique_word_list(words, num_unique_words);
    
    // print_unique_word_list(words, num_unique_words);
    write_to_file(words, num_unique_words, "input.txt");
    
    return 0;
}

void read_file(char *file_name, char **buffer, int *file_size){
    
    int temp_file_size = 0;

    FILE *fp = fopen(file_name, "r");
    fseek(fp, 0, SEEK_END);
    temp_file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *temp_buffer = (char *)malloc(sizeof(char) * temp_file_size + 1);
    fread(temp_buffer, 1, temp_file_size, fp);

    *buffer = temp_buffer;
    *file_size = temp_file_size;
}

int count_words(char *file_contents){

    int count = 1;
    while(*file_contents){
        if(*file_contents == ' ' || *file_contents == '\n'){
            count ++;
        }
        file_contents ++;
    }

    return count;
}

int get_file_words(char *file_contents, char ***word_list, int num_words){

    int length = strlen(file_contents);

    char **temp_word_list = (char**)malloc(sizeof(char*) * num_words);

    int new = 0;
    int new_index = 0;
    int word_index = 0;

    int i, val;
    char c;
    for(i = 0; i <= length; i ++){
        c = *(file_contents + i);
        val = (int)c;
        if(val >= 33 && val <= 126){
            if(new == 0){
                new = 1;
                new_index = i;
            }
        }else{
            if(new == 1){
                temp_word_list[word_index] = (char*)malloc(sizeof(char) * (i - new_index) + 1);
                strncpy(temp_word_list[word_index], file_contents + new_index, i - new_index);
                new = 0;
                word_index ++;
            }
        }
    }

    *word_list = temp_word_list;
    return word_index;
}

int get_unique_words(char **word_list, UniqueWord ***list, int num_words){

    // allocat memory for all the structs.
    UniqueWord **temp_list = (UniqueWord**)malloc(sizeof(UniqueWord*) * num_words);

    for(int i = 0; i < num_words; i ++){
        temp_list[i] = (UniqueWord*)malloc(sizeof(UniqueWord));
        temp_list[i]->word = (char*)malloc(sizeof(char) * MAX_WORD_LENGTH);
        temp_list[i]->id = 0;
        temp_list[i]->occ = 0;
    }

    int i, j;
    int id = 0;
    int new = 1;
    char *temp_word;
    for(i = 0; i < num_words; i ++){
        if(i % 1000 == 0){
            printf("%0.2f%%\n", i * (double)100 / num_words);
        }
        temp_word = word_list[i];
        for(j = 0; j < num_words; j ++){
            if(temp_list[i]->word != ""){
                int diff = strcmp(temp_word, temp_list[j]->word);
                // printf("comparing \"%s\" and \"%s\": %d\n", temp_word, temp_list[j]->word, diff);
                if(diff == 0){
                    // printf("NOT NEW!\n");
                    temp_list[j]->occ ++;
                    new = 0;
                    break;
                }
            }
        }
        if(new){
            
            temp_list[id]->word = (char*)malloc(sizeof(char) * strlen(temp_word));
            temp_list[id]->word = temp_word;
            temp_list[id]->id = id;
            temp_list[id]->occ ++;
            id ++;
        }

        new = 1;
    }

    UniqueWord **real_temp_list = (UniqueWord**)malloc(sizeof(UniqueWord*) * id);
    for(int i = 0; i < id; i ++){
        real_temp_list[i] = temp_list[i];
    }


    *list = real_temp_list;
    free(temp_list);
    return id;
}

void print_unique_word_list(UniqueWord **list, int length){
    for(int i = 0; i < length; i ++){
        printf("%d:\n", list[i]->id);
        printf("\tword: \"%s\"\n", list[i]->word);
        printf("\tocc: %d\n", list[i]->occ);
    } 
}

void write_to_file(UniqueWord **list, int length, char *filename){
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "index, word, occurances\n");
    for(int i = 0; i < length; i ++){
        fprintf(fp, "%d\t%s\t%d\n", list[i]->id, list[i]->word, list[i]->occ);
    }

    fclose(fp);
}

int load_unique_words(UniqueWord ***list, char *filename){

    FILE *fp = fopen(filename, "r");

    char c;
    fscanf(fp, "%c", &c);
    int count = -2;
    while(!feof(fp)){
        fscanf(fp, "%c", &c);
        if(c == '\n'){
            count ++;
        }
    }

    printf("length: %d\n", count);

    // Reset file pointer to top.
    fseek(fp, 0, SEEK_SET);
    

    char temp_word[MAX_WORD_LENGTH];
    fgets(temp_word, MAX_WORD_LENGTH, fp);

    UniqueWord **temp_list = (UniqueWord**)malloc(sizeof(UniqueWord*) * count);
    for(int i = 0; i < count; i ++){
        temp_list[i] = (UniqueWord*)malloc(sizeof(UniqueWord));
        
        fscanf(fp, "%d", &temp_list[i]->id);
        
        fscanf(fp, "%s", temp_word);
        temp_list[i]->word = (char*)malloc(sizeof(char) * strlen(temp_word));
        strcpy(temp_list[i]->word, temp_word);
        fscanf(fp, "%d", &temp_list[i]->occ);
    }
    
    fclose(fp);
    *list = temp_list;
    return count;
}

void free_unique_word_list(UniqueWord **list, int length){
    for(int i = 0; i < length; i ++){
        
        free(list[i]->word);
        free(list[i]);
    
    }
    free(list);
}