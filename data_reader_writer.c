#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 40
#define MAX_WORDS 1000000;

typedef struct {
    char *word;
    int length;
} Word;

typedef struct {
    char *word;
    int id;
    int occ;
} UniqueWord;

int read_file(char *file_name, char **buffer);
int get_file_words(char *file_contents, Word ***word_list, int file_size);
void print_word_list(Word **list, int num_words);
int get_unique_words(Word **word_list, UniqueWord ***list, int num_words);
void print_unique_word_list(UniqueWord **list, int length);
void write_to_file(UniqueWord **list, int length, char *filename);
int load_unique_words(UniqueWord ***list, char *filename);

int main(int argc, char **argv){
    int max_words = MAX_WORDS;
    
    /* Filename, buffer for file contents. */
    char *file_name = "big.txt";
    char *file_contents;
    /* Read all file text into int buffer. */
    int file_size = read_file(file_name, &file_contents);

    /* Split the long list of ints into 2d array of ints by word. */
    Word **word_list;
    int num_words = get_file_words(file_contents, &word_list, file_size);

    // print_word_list(word_list, num_words);

    /* Make list of UniqueWords. */
    UniqueWord **words;
    int num_unique_words = get_unique_words(word_list, &words, num_words);
    
    print_unique_word_list(words, num_unique_words);

    // write_to_file(words, num_unique_words, "words.tsv");

    /* Does not work!!!!!!!! */
    // int num_unique_words = load_unique_words(&words, "words.tsv");

    // print_unique_word_list(words, num_unique_words);
    

    /* Free everything */
    free(file_contents);

    // for(int i = 0; i < max_words; i ++){
    //     free(word_list[i]);
    //     free(word_list[i]->word);    
    // }
    // free(word_list);

    // for(int i = 0; i < num_unique_words; i ++){
    //     free(words[i]->word);
    //     free(words[i]);
    // }
    // free(words);
    
    return 0;
}

int read_file(char *file_name, char **buffer){

    FILE *fp = fopen(file_name, "r");

    // Get file length.
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // malloc the buffers.
    char *char_temp_buffer = malloc(sizeof(char) * file_size);

    // Read characters into character buffer.
    fread(char_temp_buffer, 1, file_size, fp);

    // Close, free, assign and return.
    fclose(fp);
    // free(char_temp_buffer);
    *buffer = char_temp_buffer;
    return file_size;
}

int get_file_words(char *file_contents, Word ***word_list, int file_size){

    int num = MAX_WORDS;
    Word **temp_word_list = malloc(sizeof(Word*) * num);

    for(int i = 0; i < num; i ++){
        temp_word_list[i] = malloc(sizeof(Word));
        temp_word_list[i]->length = 0;
    }

    int start_index = 0;
    int end_index = 0;
    int length = 0;
    int word_index = 0;

    int i = 0;
    char c = file_contents[i];

    while(1){
        if(c == ' ' || c == '\n'){
            file_contents[i] = '\0';
            end_index = i;
            length = end_index - start_index;
            temp_word_list[word_index]->word = malloc(sizeof(char) * length);
            temp_word_list[word_index]->length = length;
            strcpy(temp_word_list[word_index]->word, file_contents + start_index);
            start_index = i + 1;
            word_index ++;
        }

        if(file_contents[i + 1]){
            c = file_contents[++i];
        }else{
            break;
        }
    }

    *word_list = temp_word_list;
    return word_index;
}

void print_word_list(Word **list, int length){

    for(int i = 0; i < length - 1; i ++){
        printf("\"%s\"\n", list[i]->word);
    }
}

int get_unique_words(Word **word_list, UniqueWord ***list, int num_words){

    UniqueWord **temp = (UniqueWord**)malloc(sizeof(UniqueWord*) * num_words);

    for(int i = 0; i < num_words; i ++){
        temp[i] = malloc(sizeof(UniqueWord));
        temp[i]->word = NULL;
        temp[i]->id = 0;
        temp[i]->occ = 0;
    }

    // Get the number of unique words.
    char *temp_word = malloc(sizeof(char) * word_list[0]->length);
    temp_word = word_list[0]->word;
    int temp_length = word_list[0]->length;
    int index = 0;
    int new = 1;
    int diff = 0;
    for(int i = 1; i <= num_words; i ++){
        if(i % 1000 == 0){
            printf("%0.2f%%\n", i * (double)100 / num_words);
        }
        for(int j = 0; j < num_words; j ++){
            if(temp[j]->word){
                for(int k = 0; k < temp_length; k ++){
                    if(temp[j]->word[k] == temp_word[k]){
                        diff ++;
                    }
                }
                if(diff == temp_length){
                    new = 0;
                    temp[j]->occ ++;
                }
            }
            diff = 0;
        }
        if(new){
            temp[index]->word = malloc(sizeof(char) * temp_length);
            temp[index]->word = temp_word;
            temp[index]->id = index;
            temp[index]->occ ++;
            index ++;
        }
        
        // free(temp_word);
        // temp_word = (int*)malloc(sizeof(int) * word_list[i]->length);
        temp_word = word_list[i]->word;

        new = 1;
    }

    // UniqueWord **real_temp = (UniqueWord**)malloc(sizeof(UniqueWord*) * index);
    // for(int i = 0; i < index; i ++){
    //     real_temp[i] = temp[i];
    // }

    *list = temp;
    return index;
}

void print_unique_word_list(UniqueWord **list, int length){
    printf("%d\n", length);
    for(int i = 0; i < length; i ++){
        printf("%d\n", i);
        printf("%d:\n", list[i]->id);
        printf("\tword: \"%s\"\n", list[i]->word);
        printf("\tocc: %d\n", list[i]->occ);
    }
}

void write_to_file(UniqueWord **list, int length, char *filename){
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "index\tword\toccurances\n");
    for(int i = 0; i < length; i ++){
        fprintf(fp, "%d\t%s\t%d\n", list[i]->id, list[i]->word, list[i]->occ);
    }

    fclose(fp);
}

int load_unique_words(UniqueWord ***list, char *filename){

    FILE *fp = fopen(filename, "r");

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *file_buffer = malloc(sizeof(char) * file_size);
    fread(file_buffer, 1, file_size, fp);

    fclose(fp);

    int count = 0;
    for(int i = 0; i < file_size; i ++){
        if(file_buffer[i] == '\n'){
            count ++;
        }
    }

    /* One for the first line, one for the last. */
    count = count - 2;

    UniqueWord **temp = malloc(sizeof(UniqueWord*) * count);
    for(int i = 0; i <= count; i ++){
        temp[i] = malloc(sizeof(UniqueWord));
        temp[i]->id = i;
        temp[i]->occ = 0;
    }

    int t1_index = 0;
    int t2_index = 0;
    int tcount = 0;
    int index = 0;
    for(int i = 21; i < file_size; i ++){
        if(file_buffer[i] == '\t'){
            tcount ++;
            if(tcount == 1){
                t1_index = i;
            }else if(tcount == 2){
                t2_index = i;

                temp[i]->word = malloc(sizeof(char) * t2_index - t1_index);
                strncpy(temp[i]->word, file_buffer + t1_index + 1,  t2_index - t1_index - 1);

                tcount = 0;
            }
        }
    }
    
    *list = temp;
    return count;
}
