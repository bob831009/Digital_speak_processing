#include "hmm.h"
#include <math.h>
#include <stdio.h>

/*typedef struct{
   char *model_name;
   int state_num;					//number of state
   int observ_num;					//number of observation
   double initial[MAX_STATE];			//initial prob.
   double transition[MAX_STATE][MAX_STATE];	//transition prob.
   double observation[MAX_OBSERV][MAX_STATE];	//observation prob.
} HMM;
*/
int observe_to_index(char input){
	return (int)(input - 'A');
}

int main(int argc, char** argv){

	if(argc != 4){
		printf("ERR_format\n");
		exit(0);
	}
	HMM hmms[5];
	load_models(argv[1], hmms, 5);
	//dump_models( hmms, 5);
	FILE *input_fp = open_or_die(argv[2], "r");
	FILE *output_fp = fopen(argv[3], "w");
	FILE *answer_fp = fopen("testing_answer.txt", "r");
	FILE *acc_fp = fopen("acc.txt", "w");
	char tmp_string[MAX_SEQ];
	int Correct_num = 0;
	int Total_input_num = 0;
	while(fscanf(input_fp, "%s", tmp_string) != EOF){
		// printf("%s\n", tmp_string);
		Total_input_num++;
		int seq_len = strlen(tmp_string);
		int Max_model = 0;
		double Max_model_prob = 0;
		for(int model_index = 0; model_index < 5; model_index++){
			HMM hmm = hmms[model_index];
			double lamda[MAX_SEQ][MAX_STATE] = {0};
			for (int t = 0 ; t < seq_len ; t++){
				for (int i = 0 ; i < hmm.state_num ; i++){
					if(t == 0){
						lamda[t][i] = hmm.initial[i] * hmm.observation[observe_to_index(tmp_string[t])][i];
					}else{
						double tmp_max = 0;
						for(int j = 0 ; j < hmm.state_num ; j++){
							if(lamda[t - 1][j] * hmm.transition[j][i] > tmp_max)
								tmp_max = lamda[t - 1][j] * hmm.transition[j][i];
						}
						lamda[t][i] = tmp_max * hmm.observation[observe_to_index(tmp_string[t])][i];
					}
					// printf("%e ", lamda[t][i]);
				}
				// printf("\n");
			}
			double max_prob = 0;
			for(int i = 0 ; i < hmm.state_num ; i++){
				// printf("%e ", lamda[seq_len - 1][i]);
				if(lamda[seq_len - 1][i] > max_prob){
					max_prob = lamda[seq_len - 1][i];
					// printf("%e \n", max_prob);
				}
			}
			// printf("\n");
			// printf("%d %e\n", Max_model, Max_model_prob);
			// printf("%e \n", max_prob);
			if(max_prob > Max_model_prob){
				// printf("%d %e\n", Max_model, Max_model_prob);
				Max_model_prob = max_prob;
				Max_model = model_index;
			}
			// printf("%d %e\n", Max_model, Max_model_prob);
		}
		// printf("%d %e\n", Max_model, Max_model_prob);
		char tmp_ans_string[100];
		int tmp_ans = 0;
		//fscanf(answer_fp, "%s", tmp_ans_string);
		//sscanf(tmp_ans_string, "model_0%d.txt", &tmp_ans);
		fprintf(output_fp, "model_0%d.txt %e\n", Max_model + 1, Max_model_prob);
		// if(tmp_ans == Max_model+1)
		// 	Correct_num++;
	}
	//fprintf(acc_fp, "%lf\n", float(Correct_num)/Total_input_num);
	//printf("acc : %lf\n",  float(Correct_num)/Total_input_num);
	//printf("%f\n", log(1.5) ); // make sure the math library is included
	return 0;
}
