#include <iostream>
#include <fstream>
using namespace std;


// void readfile(filename){
char * memblock;
// }

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


int filesize(string inputfile){
// obtaining file size
    long begin,end;
    ifstream myfile ("input.txt");
    begin = myfile.tellg();
    myfile.seekg (0, ios::end);
    end = myfile.tellg();
    myfile.close();
    cout << "size is: " << (end-begin) << " bytes.\n";
    return 0;

}




void matrixfiller(int *** thematrix, string fileName){
    ifstream infile(fileName);
    // char *file_name = "words.tsv";
    // char *file_contents;
    // int file_size;
    // cout << "File Name is: " << file_name << endl;;
    // ifstream file ("word.tsv.bin", ios::in|ios::binary|ios::ate);
    // int size = file.tellg();
    // memblock = new char [size];
    // file.seekg (0, ios::beg);
    // file.read (memblock, size);
    // file.close();
    // cout << "the complete file content is in memory";
   
   //Getting the number of lines

    int numlines = 0;
    int mostoccurrance = 0;
    string line;
    string word;
    int index, occurance; 
    int **temp = (int **) malloc(sizeof(int *) * numlines);
    for (int i = 0; i < numlines; i++){
        temp[i] = (int *) malloc(sizeof(int) * numlines);
    }
    
    while(getline(infile, line)){
        string word;
        int occurance;
        infile >> index >> word >> occurance;
        if (occurance > mostoccurrance){
            mostoccurrance = occurance;
        }
        temp[numlines][numlines] = occurance;
        // cout << "INDEX is " << index << endl;
        cout << word << endl;
        // cout << "INT is " << occurance << endl;
        // cout << "A is " << index << endl;
        // cout << "B is " << word << endl;
        // cout << "B is " << occurance << endl;
        numlines++;

    }
    cout << "Number of lines is " << numlines << endl;
    cout << "Most occurance is " << mostoccurrance << endl;
    
    
    // while(getline(infile, line)){
    //     cout << "True?" << endl;

    //     cout << line << endl;

    //     infile >> index >> word >> occurance;
    //     cout << "DYLANS WORK" << occurance << endl;
    //     ++linenumber;
    //     cout << "Hello" << endl;
    //     // cout << "Line number is " << linenumber << endl;
        

    // }
 
    *thematrix = temp;
}


void print_matrix(int **matrix, int size)
{
    assert(matrix);

    printf("---- Print Matrix ----\n");
    printf("This matrix is %d x %d\n", size, size);
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("--------\n\n");

}

int main () {

    int **matrix;
    matrixfiller(&matrix, "input.txt");
    print_matrix(matrix, 10);
    
    // cout << file_contents[1] << endl;
    // cout << file_contents << endl;
    return 0;
}