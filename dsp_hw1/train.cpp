#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "hmm.h"

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
int main (int argc, char *argv[]){
	
	if(argc != 5){
		printf("Err_input\n");
		exit(0);
	}
    int iteration = atoi(argv[1]);
	char* InitModel_path = argv[2];
	char* SeqModel_path = argv[3];
	char* Model_path = argv[4];
	
	HMM my_HMM;
	
	loadHMM(&my_HMM, InitModel_path);
	FILE *output_fp = open_or_die(Model_path, "w");
	while(iteration > 0){
		char tmp_string[MAX_SEQ];
		FILE *input_fp = open_or_die(SeqModel_path, "r");
		double Total_gamma[MAX_SEQ][MAX_STATE] = {};
		double Total_epsilon[MAX_SEQ][MAX_STATE][MAX_STATE] = {};
		double Total_gamma_obser[MAX_OBSERV][MAX_STATE] = {};
		int Total_sample_num = 0;
		int seq_len = 0;
		while(fscanf(input_fp, "%s", tmp_string) != EOF){
			Total_sample_num++;
			seq_len = strlen(tmp_string);
			
			// alpha
			double alpha[MAX_SEQ][MAX_STATE] = {};
			for(int i = 0; i < seq_len ; i++){
				for(int j = 0; j < my_HMM.state_num ; j++){
					if(i == 0){
						alpha[i][j] = my_HMM.initial[j] * my_HMM.observation[observe_to_index(tmp_string[i])][j];
					}else{
						double total_tmp = 0;
						for (int k = 0; k < my_HMM.state_num ; k++){
							total_tmp += alpha[i - 1][k] * my_HMM.transition[k][j];
						}
						alpha[i][j] = total_tmp * my_HMM.observation[observe_to_index(tmp_string[i])][j];
					}
					//printf("%lf ",  alpha[i][j]);
				}
				//printf("\n");
			}
			
			//beta
			double beta[MAX_SEQ][MAX_STATE] = {};
			for (int i = seq_len - 1 ; i >= 0 ; i--){
				for(int j = 0 ; j < my_HMM.state_num ; j++){
					if(i == seq_len - 1){
						beta[i][j] = 1;
					}else{
						double total_tmp = 0;
						for(int k = 0 ; k < my_HMM.state_num ; k++){
							total_tmp += my_HMM.transition[j][k] * my_HMM.observation[observe_to_index(tmp_string[i + 1])][k] * beta[i + 1][k];
						}
						beta[i][j] = total_tmp;
					}
					//printf("%lf ",  beta[i][j]);
				}
				//printf("\n");
			}
			
			// gamma
			double gamma[MAX_SEQ][MAX_STATE] = {};
			for (int t = 0 ; t < seq_len ; t++){
				double total_tmp = 0;
				for (int i = 0 ; i < my_HMM.state_num ; i++)
					total_tmp += alpha[t][i] * beta[t][i];
				for (int i = 0 ; i < my_HMM.state_num ; i++){
					gamma[t][i] = alpha[t][i] * beta[t][i] / total_tmp;
					Total_gamma[t][i] += gamma[t][i];
					Total_gamma_obser[observe_to_index(tmp_string[t])][i] += gamma[t][i];
					//printf("%lf ", Total_gamma[t][i]);
				}
				//printf("\n");
			}
			
			// epsilon
			double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE] = {};
			for (int t = 0; t < seq_len - 1 ; t++){
				
				double total_tmp = 0;
				for (int i = 0 ; i < my_HMM.state_num ; i++){
					for (int j = 0 ; j < my_HMM.state_num ; j++){
						total_tmp += alpha[t][i] * my_HMM.transition[i][j] * my_HMM.observation[observe_to_index(tmp_string[t + 1])][j] * beta[t + 1][j];
					}
				}
				
				for (int i = 0 ; i < my_HMM.state_num ; i++){
					for (int j = 0 ; j < my_HMM.state_num ; j++){
						epsilon[t][i][j] = alpha[t][i]*my_HMM.transition[i][j]*my_HMM.observation[observe_to_index(tmp_string[t + 1])][j] * beta[t + 1][j] / total_tmp;
						Total_epsilon[t][i][j] += epsilon[t][i][j];
					}
				}
			}
					
		}
		HMM new_model;
		new_model.model_name = my_HMM.model_name;
		new_model.state_num = my_HMM.state_num;
		new_model.observ_num = my_HMM.observ_num;
		for (int i = 0 ; i < my_HMM.state_num ; i++){
			new_model.initial[i] = Total_gamma[0][i] / Total_sample_num;
		}
		for (int i = 0 ; i < my_HMM.state_num ; i++){
			double tmp_gamma_sum = 0;
			for (int t = 0 ; t < seq_len - 1 ; t++){
				tmp_gamma_sum += Total_gamma[t][i];
			}
			for (int j = 0 ; j < my_HMM.state_num ; j++){
				double tmp_epsilon_sum = 0;
				for (int t = 0; t < seq_len - 1; t++){
					tmp_epsilon_sum += Total_epsilon[t][i][j];
				}
				new_model.transition[i][j] = tmp_epsilon_sum / tmp_gamma_sum;
			}
		}

		for(int k = 0 ; k < my_HMM.observ_num ; k++){
			for(int j = 0 ; j < my_HMM.state_num ; j++){
				double tmp_total = 0;
				for(int t = 0 ; t < seq_len ; t++){
					tmp_total += Total_gamma[t][j];
				}
				new_model.observation[k][j] = Total_gamma_obser[k][j] / tmp_total;
			}
		}
		my_HMM = new_model;
		iteration--;
		fclose(input_fp);
	}
	dumpHMM(output_fp, &my_HMM);
	// dumpHMM(stderr, &my_HMM);
	fclose(output_fp);
}