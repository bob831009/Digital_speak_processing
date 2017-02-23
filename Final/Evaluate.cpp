#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
using namespace std;


int main(int argc, char** argv){
    int ngram_order;
    string test_file1_dir, test_file2_dir;
    // parse argv
 	if(argc != 3){
 		printf("ERROR ARGV FORMAT!\n");
 		exit(0);
 	}else{
 		test_file1_dir = argv[1];
 		test_file2_dir = argv[2];
 	}

 	fstream f1_text;
 	fstream f2_text;
 	f1_text.open(test_file1_dir, fstream::in);
 	f2_text.open(test_file2_dir, fstream::in);

 	string f1_line, f2_line;
 	int line_error_num = 0;
 	int total_line_num = 0;
 	int term_error_num = 0;
 	int total_term_num = 0;
 	while(getline(f1_text, f1_line) && getline(f2_text, f2_line)){
 		
 		// handle word error
 		int Error_Condiction = 0;
 		for(int i = 0 ; i < f1_line.length();){
 			if(f1_line[i] < 0){
 				int term_end;
 				for(term_end = i; term_end < f1_line.length(); term_end+=2){
 					if(f1_line[term_end] > 0)
 						break;
 				}

 				string term1 = f1_line.substr(i, term_end-i);
 				string term2 = f2_line.substr(i, term_end-i);
 				total_term_num += 1;
 				if(term1.compare(term2) != 0){
 					term_error_num += 1;
 					Error_Condiction = 1;
 				}

 				i = term_end;
 			}else{
 				i++;
 			}
 		}

 		// handle line errors
 		total_line_num += 1;
 		if(Error_Condiction != 0){
 			line_error_num += 1;
 		}
 	}
 	printf("Sentence Correct Rate: %lf\n", 1-float(line_error_num)/total_line_num);
 	printf("Word Correct Rate: %lf\n", 1-float(term_error_num)/total_term_num);
}