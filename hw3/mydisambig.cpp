#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include "Ngram.h"

using namespace std;

bool check_exist(const char *w1, Vocab &voc){
    VocabIndex wid = voc.getIndex(w1);
    if(wid == Vocab_None){
        return false;
    }else{
        return true;
    }
}

void Create_Map(string file_name, map<string, vector<string> > &My_map, Vocab &voc){
    string input_line;
    fstream fin_map;
    fin_map.open(file_name, fstream::in);
    while(getline(fin_map, input_line)){
        string key = input_line.substr(0, 2);
        string map_part = input_line.substr(2, input_line.length()-2);

        for (int i = 0; i < map_part.length(); i++){
            if(map_part[i] < 0){
                string map_to_word = map_part.substr(i, 2);
                My_map[key].push_back(map_to_word);
                i++;
            }
        }
    }
    My_map["<s>"].push_back("<s>");
    My_map["</s>"].push_back("</s>");

    return;
}
// Get P(W2 | W1) -- bigram
double getBigramProb(const char *w1, const char *w2, Vocab &voc, Ngram &lm)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None){  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);
    }

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

// Get P(W3 | W1, W2) -- trigram
double getTrigramProb(const char *w1, const char *w2, const char *w3, Vocab &voc, Ngram &lm) 
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);
    VocabIndex wid3 = voc.getIndex(w3);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);
    if(wid3 == Vocab_None)  //OOV
        wid3 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid2, wid1, Vocab_None };
    return lm.wordProb( wid3, context );
}
struct Node{
    double prob;
    string word;
    int pre_index;
};
void Viterbit(vector<string> Words, map<string, vector<string> > &My_map, Vocab &voc, Ngram &lm){
    vector< vector<Node> > Viterbit_map;

    for(int t = 0; t < Words.size(); t++){
        if(t == 0){
            string start_word = "<s>";
            Node start_node;
            vector<Node> start_node_vector;

            start_node.prob = 0;
            start_node.word = start_word;
            start_node.pre_index = 0;
            start_node_vector.push_back(start_node);
            Viterbit_map.push_back(start_node_vector);
            continue;
        }
        string word = Words[t];
        string word_pre = Words[t-1];
        if(My_map.find(word) == My_map.end()){
            My_map[word].push_back(word);
        }
        vector<string> word_to_map_now = My_map[word];
        vector<string> word_to_map_pre = My_map[word_pre];
        vector<Node> Nodes_now;
        vector<Node> Node_pre = Viterbit_map[t-1];

        for (int i = 0; i < word_to_map_now.size(); i++){
            string possi_word = word_to_map_now[i];
            if(word_to_map_now.size() > 1 && !check_exist(possi_word.c_str(), voc)){
                word_to_map_now.erase(word_to_map_now.begin()+i);
                i--;
                continue;
            }
            double Max_prob = -10000000000;
            string Max_possi_word_pre;
            int Max_pre_index;
            Node tmp_Node;

            for (int j = 0; j < word_to_map_pre.size(); j++){
                string possi_word_pre = word_to_map_pre[j];
                if(word_to_map_pre.size() > 1 && !check_exist(possi_word_pre.c_str(), voc)){
                    word_to_map_pre.erase(word_to_map_pre.begin()+j);
                    j--;
                    continue;
                }
                double bigram_prob = getBigramProb(possi_word_pre.c_str(), possi_word.c_str(), voc, lm);
                double tmp_prob = Node_pre[j].prob + bigram_prob;
                if(tmp_prob > Max_prob){
                    Max_prob = tmp_prob;
                    Max_possi_word_pre = possi_word_pre;
                    Max_pre_index = j;
                }
            }
            tmp_Node.prob = Max_prob;
            tmp_Node.word = possi_word;
            tmp_Node.pre_index = Max_pre_index;
            Nodes_now.push_back(tmp_Node);
        }
        Viterbit_map.push_back(Nodes_now);
    }
    stack<string> mystack;
    for(int t = Words.size()-1, Max_index = 0; t >= 0; t--){
        Node tmp_node = Viterbit_map[t][Max_index];
        mystack.push(tmp_node.word);
        Max_index = tmp_node.pre_index;
    }
    while(!mystack.empty()){
        if(mystack.size() == 1)
            printf("%s", (mystack.top()).c_str());
        else
            printf("%s ", (mystack.top()).c_str());
        mystack.pop();
    }
    printf("\n");
}



int main(int argc, char** argv){
    int ngram_order;
    string input_text, ZhuYinMap_file, Language_model_file;
    // parse argv
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-text") == 0){
            input_text = argv[i + 1];
        }else if(strcmp(argv[i], "-map") == 0){
            ZhuYinMap_file = argv[i + 1];
        }else if(strcmp(argv[i], "-lm") == 0){
            Language_model_file = argv[i + 1];
        }else if(strcmp(argv[i], "-order") == 0){
            ngram_order = atoi(argv[i + 1]);
        }else{
            continue;
        }
        i++;
    }

    // create lm
    Vocab voc;
    Ngram lm( voc, ngram_order );
    {
        File lmFile(Language_model_file.c_str(), "r");
        lm.read(lmFile);
        lmFile.close();
    }

    // Create Map
    map<string, vector<string> > My_map;
    Create_Map(ZhuYinMap_file, My_map, voc);

    // printf("%s\n", input_text.c_str());
    // read input_text
    fstream fin_text;
    fin_text.open(input_text, fstream::in);
    string input_line;
    while(getline(fin_text, input_line)){
        vector<string> Words;
        Words.push_back("<s>");
        for (int i = 0 ; i < input_line.length(); i++){
            if(input_line[i] < 0){
                string word = input_line.substr(i, 2);
                Words.push_back(word);
                i++;
            }
        }
        Words.push_back("</s>");
        Viterbit(Words, My_map, voc, lm);
    }


}