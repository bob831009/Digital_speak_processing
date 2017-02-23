#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stack>
#include "Ngram.h"

using namespace std;
double term_LAMDA = 0.2;
double LAMDA = 0.8;
double User_Trust = 3;
bool check_exist(const char *w1, Vocab &voc){
    VocabIndex wid = voc.getIndex(w1);
    if(wid == Vocab_None){
        return false;
    }else{
        return true;
    }
}

void Create_Big5_Map(string Big5_to_ZhuYin_file, map<string, vector<string> > &Big5_to_ZhuYin_map, Vocab &voc){
    string input_line;
    fstream fin;
    fin.open(Big5_to_ZhuYin_file, fstream::in);
    while(getline(fin, input_line)){
        string word = input_line.substr(0, 2);
        string ZhuYin_part = input_line.substr(2, input_line.length()-2);

        for (int cond = 0, i = 0; i < ZhuYin_part.length();){
            if(ZhuYin_part[i] < 0){
                int ZhuYin_end;
                for (ZhuYin_end = i; ZhuYin_end < ZhuYin_part.length(); ZhuYin_end += 2){
                    if(ZhuYin_part[ZhuYin_end] > 0)
                        break;
                }
                string ZhuYin_token = ZhuYin_part.substr(i, ZhuYin_end-i);
                Big5_to_ZhuYin_map[word].push_back(ZhuYin_token);
                i = ZhuYin_end;

            }else if(ZhuYin_part[i] >= 0){
                i++;
            }
        }
    }
    Big5_to_ZhuYin_map["<s>"].push_back("<s>");
    Big5_to_ZhuYin_map["</s>"].push_back("</s>");
    return;
}
void Create_ZhuYin_Map(string ZhuYin_to_Big5_file, map<string, vector<string> > &ZhuYin_to_Big5_map, Vocab &voc){
    string input_line;
    fstream fin_map;
    fin_map.open(ZhuYin_to_Big5_file, fstream::in);
    while(getline(fin_map, input_line)){
        int key_end;
        for (key_end = 0; key_end < input_line.length(); key_end+=2){
            if(input_line[key_end] > 0)
                break;
        }

        string key = input_line.substr(0, key_end);
        string map_part = input_line.substr(key_end, input_line.length()-key_end);

        for (int i = 0; i < map_part.length(); i++){
            if(map_part[i] < 0){
                string map_to_word = map_part.substr(i, 2);
                ZhuYin_to_Big5_map[key].push_back(map_to_word);
                i++;
            }
        }
    }
    ZhuYin_to_Big5_map["<s>"].push_back("<s>");
    ZhuYin_to_Big5_map["</s>"].push_back("</s>");
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

vector<string> All_Poss_Mapping(string word, map<string, vector<string> > &Big5_to_ZhuYin_map, map<string, vector<string> > &ZhuYin_to_Big5_map){
    vector<string> All_Poss_Word;
    vector<string> Possi_ZhuYin = Big5_to_ZhuYin_map[word];
    for(int i = 0; i < Possi_ZhuYin.size(); i++){
        string possi_ZhuYin = Possi_ZhuYin[i];
        vector<string> Possi_Word = ZhuYin_to_Big5_map[possi_ZhuYin];
        All_Poss_Word.insert(All_Poss_Word.end(), Possi_Word.begin(), Possi_Word.end());
    }

    return All_Poss_Word;
}
struct Node{
    double prob;
    string term;
    int pre_index;
};

struct InTermNode{
    double inTerm_prob;
    string term;
};

struct ViterbitNode{
    double prob_sum;
    double inTerm_prob;
    string term;
    int pre_index;
};
string ReverseViterbitMap(vector<vector<Node> > &TermViterbit_map, int start_index){
    string output;
    int index = start_index;
    for(int t = TermViterbit_map.size()-1; t >= 0; t--){
        output.insert(0, TermViterbit_map[t][index].term);
        index = TermViterbit_map[t][index].pre_index;
    }
    return output;
}
vector<InTermNode> TermViterbit(string term, map<string, vector<string> > &Big5_to_ZhuYin_map, map<string, vector<string> > &ZhuYin_to_Big5_map, Vocab &voc, Ngram &lm, Vocab &topic_voc, Ngram &topic_lm){
    vector<InTermNode> Possi_Word_Nodes;

    // printf("%s\n", term.c_str());
    // printf("====================\n");
    // one word term
    if(term.length()/2 == 1 || strcmp(term.c_str(), "</s>") == 0){
        vector<string> possi_Words = All_Poss_Mapping(term, Big5_to_ZhuYin_map, ZhuYin_to_Big5_map);
        for(int i = 0; i < possi_Words.size(); i++){
            InTermNode tmp_InTernNode;
            tmp_InTernNode.inTerm_prob = 0;
            tmp_InTernNode.term = possi_Words[i];
            Possi_Word_Nodes.push_back(tmp_InTernNode);
        }
    // multiple words term
    }else{
        vector<vector<Node> > TermViterbit_map;
        for(int t = 0 ; t < term.length(); t += 2){
            string word = term.substr(t, 2);
            vector<string> possi_Words = All_Poss_Mapping(word, Big5_to_ZhuYin_map, ZhuYin_to_Big5_map);
            vector<Node> tmp_Node_Vector;
            if(t == 0){
                for(int i = 0; i < possi_Words.size(); i++){
                    Node tmp_Node;
                    tmp_Node.term = possi_Words[i];
                    tmp_Node.prob = 0;
                    tmp_Node.pre_index = 0;
                    tmp_Node_Vector.push_back(tmp_Node);
                }
            }else{
                string pre_word = term.substr(t-2, 2);
                // printf("%s %s\n", pre_word.c_str(), word.c_str());
                vector<string> pre_possi_Words = All_Poss_Mapping(pre_word, Big5_to_ZhuYin_map, ZhuYin_to_Big5_map);
                for(int i = 0; i < possi_Words.size(); i++){
                    double Max_prob = -100000000;
                    int Max_pre_index = 0;
                    string Max_pre_string;
                    for (int j = 0; j < pre_possi_Words.size(); j++){
                        double bigram_prob = getBigramProb(pre_possi_Words[j].c_str(), possi_Words[i].c_str(), voc, lm);
                        double topic_bigram_prob = getBigramProb(pre_possi_Words[j].c_str(), possi_Words[i].c_str(), topic_voc, topic_lm);
                        double tmp_prob = TermViterbit_map[t/2-1][j].prob + term_LAMDA*bigram_prob + (1-term_LAMDA)*topic_bigram_prob;

                        // ===========for Debug=============
                        // printf("%s %s\n", pre_possi_Words[j].c_str(), possi_Words[i].c_str());
                        // printf("bigram: %lf\n", bigram_prob);
                        // printf("length: %d\n", TermViterbit_map[t/2-1].size());
                        // printf("tmp_prob: %lf\n", tmp_prob);
                        // =================================
                        if(tmp_prob > Max_prob){
                            Max_prob = tmp_prob;
                            Max_pre_index = j;
                            Max_pre_string = pre_possi_Words[j];
                        }
                    }
                    Node tmp_Node;
                    tmp_Node.term = possi_Words[i];
                    tmp_Node.prob = Max_prob;
                    tmp_Node.pre_index = Max_pre_index;
                    tmp_Node_Vector.push_back(tmp_Node);
                }
            }
            TermViterbit_map.push_back(tmp_Node_Vector);
        }

        vector<Node> Last_Column = TermViterbit_map[TermViterbit_map.size()-1];
        for(int i = 0; i < Last_Column.size(); i++){
            InTermNode tmp_InTernNode;
            tmp_InTernNode.inTerm_prob = Last_Column[i].prob;
            tmp_InTernNode.term = ReverseViterbitMap(TermViterbit_map, i);
            if(tmp_InTernNode.term == term)
                tmp_InTernNode.inTerm_prob += User_Trust;
            Possi_Word_Nodes.push_back(tmp_InTernNode);
        }
    }
    return Possi_Word_Nodes;
}
void Viterbit(vector<string> Terms, map<string, vector<string> > &Big5_to_ZhuYin_map, map<string, vector<string> > &ZhuYin_to_Big5_map, Vocab &voc, Ngram &lm, Vocab &topic_voc, Ngram &topic_lm){
    vector< vector<ViterbitNode> > Viterbit_map;

    for(int t = 0; t < Terms.size(); t++){

        if(t == 0){
            string start_word = "<s>";
            ViterbitNode start_node;
            vector<ViterbitNode> start_node_vector;

            start_node.prob_sum = 0;
            start_node.inTerm_prob = 0;
            start_node.term = start_word;
            start_node.pre_index = 0;
            start_node_vector.push_back(start_node);
            Viterbit_map.push_back(start_node_vector);
            continue;
        }
        string term = Terms[t];
        string term_pre = Terms[t-1];
        
        vector<InTermNode> possi_terms = TermViterbit(term, Big5_to_ZhuYin_map, ZhuYin_to_Big5_map, voc, lm, topic_voc, topic_lm);
        // printf("After TermViterbit\n");
        // for(int i = 0 ; i < possi_terms.size(); i++)
        //     printf("%s ", possi_terms[i].term.c_str());
        // printf("\n");
        vector<ViterbitNode> pre_Col = Viterbit_map[t-1];
        vector<ViterbitNode> now_Col;
        for(int i = 0; i < possi_terms.size(); i++){
            string possi_term_first_word = possi_terms[i].term.substr(0,2);
            double Max_prob = -10000000;
            int Max_pre_index = 0;

            for(int j = 0; j < pre_Col.size(); j++){
                string pre_possi_term = pre_Col[j].term;
                string pre_possi_term_last_word = pre_possi_term.substr(pre_possi_term.length()-2, 2);

                double bigram_prob = getBigramProb(pre_possi_term_last_word.c_str(), possi_term_first_word.c_str(), voc, lm);
                double topic_bigram_prob = getBigramProb(pre_possi_term_last_word.c_str(), possi_term_first_word.c_str(), topic_voc, topic_lm);
                double inTerm_prob = possi_terms[i].inTerm_prob;
                double tmp_prob = pre_Col[j].prob_sum + LAMDA*bigram_prob + inTerm_prob + (1-LAMDA)*topic_bigram_prob;
                if(tmp_prob > Max_prob){
                    Max_prob = tmp_prob;
                    Max_pre_index = j;
                }
            }

            ViterbitNode tmp_Viter_Node;
            tmp_Viter_Node.prob_sum = Max_prob;
            tmp_Viter_Node.inTerm_prob = possi_terms[i].inTerm_prob;
            tmp_Viter_Node.term = possi_terms[i].term;
            tmp_Viter_Node.pre_index = Max_pre_index;

            now_Col.push_back(tmp_Viter_Node);
        }
        Viterbit_map.push_back(now_Col);
    }

    stack<string> mystack;
    for(int t = Terms.size()-1, Max_index = 0; t >= 0; t--){
        ViterbitNode tmp_node = Viterbit_map[t][Max_index];
        mystack.push(tmp_node.term);
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

vector<string> Big5_split(string str) {
    vector<string> internal;
    for(int i = 0; i < str.length();){
        if(str[i] < 0){
            int word_end;
            for(word_end = i; word_end < str.length(); word_end += 2){
                if(str[word_end] > 0)
                    break;
            }
            string term = str.substr(i, word_end-i);
            internal.push_back(term);
            i = word_end;
        }else{
            i++;
        }
    }

    return internal;
}

int main(int argc, char** argv){
    int ngram_order;
    string input_text, ZhuYin_to_Big5_file, Language_model_file, Big5_to_ZhuYin_file;
    string topic_file_dir;
    // parse argv
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-text") == 0){
            input_text = argv[i + 1];
        }else if(strcmp(argv[i], "-map_Big5") == 0){
            Big5_to_ZhuYin_file = argv[i + 1];
        }else if(strcmp(argv[i], "-lm") == 0){
            Language_model_file = argv[i + 1];
        }else if(strcmp(argv[i], "-order") == 0){
            ngram_order = atoi(argv[i + 1]);
        }else if(strcmp(argv[i], "-map_ZhuYin") == 0){
            ZhuYin_to_Big5_file = argv[i + 1];
        }else if(strcmp(argv[i], "-topic_class") == 0){
            topic_file_dir = argv[i + 1];
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

    Vocab topic_voc;
    string topic_language_file = string("./Document_data/") + topic_file_dir + string("/bigram.lm");
    Ngram topic_lm(topic_voc, ngram_order);
    {
        File topic_lmFile(topic_language_file.c_str(), "r");
        topic_lm.read(topic_lmFile);
        topic_lmFile.close();
    }

    // Create BIG5 to ZhuYin Map
    map<string, vector<string> > Big5_to_ZhuYin_map;
    Create_Big5_Map(Big5_to_ZhuYin_file, Big5_to_ZhuYin_map, voc);

    // Create ZhuYIn to BIG5 Map
    map<string, vector<string> > ZhuYin_to_Big5_map;
    Create_ZhuYin_Map(ZhuYin_to_Big5_file, ZhuYin_to_Big5_map, voc);

    // read input_text
    fstream fin_text;
    fin_text.open(input_text, fstream::in);
    string input_line;
    while(getline(fin_text, input_line)){
        vector<string> Terms;
        Terms = Big5_split(input_line);
        Terms.insert(Terms.begin(), "<s>");
        Terms.push_back("</s>");

        Viterbit(Terms, Big5_to_ZhuYin_map, ZhuYin_to_Big5_map, voc, lm, topic_voc, topic_lm);
    }
}