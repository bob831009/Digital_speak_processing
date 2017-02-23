#include <iostream>
#include <string>
#include <stdio.h>
#include <map>
#include <algorithm>
#include <vector>
#include <string.h>
#include <fstream>

using namespace std;
bool test_exist(vector<string> v, string target){
	if(find(v.begin(), v.end(), target) != v.end())
		return true;
	else
		return false;
}
int main(int argc, char** argv){
	string input_line;
	map<string, vector<string> > My_map;
	map<string, string> word_map;
	fstream fin;
	fin.open(argv[1], fstream::in);
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
				if(not test_exist(My_map[ZhuYin_token], word))
					My_map[ZhuYin_token].push_back(word);
				i = ZhuYin_end;

			}else if(ZhuYin_part[i] >= 0){
				i++;
			}
		}
	}

	FILE *fp = fopen(argv[2], "w");
	for (map<string, vector<string> >::iterator i = My_map.begin(); i != My_map.end(); ++i){
		string key = i->first;
		vector<string> v = i->second;
		fprintf(fp, "%s    ", key.c_str());
		for (int j = 0; j < v.size(); j++){
			fprintf(fp, "%s ", v[j].c_str());
		}
		fprintf(fp, "\n");
		// for (int j = 0; j < v.size(); j++){
		// 	if(word_map.find(v[j]) == word_map.end()){
		// 		word_map[v[j]] = v[j];
		// 		fprintf(fp, "%c%c     %c%c\n", v[j][0], v[j][1], v[j][0], v[j][1]);
		// 	}
		// }
	}
}