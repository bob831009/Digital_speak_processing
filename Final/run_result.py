import os

for i in range(4):
	topic_class = "topic" + str(i);
	filename_dir = os.path.join("Document_data", topic_class, topic_class+'_filename.txt');
	fp = open(filename_dir);
	print("Handling " +topic_class);
	print("=========================");
	for line in fp:
		line = line.split(" ");
		for file_name in line:
			test_file_dir = os.path.join("/tmp2/b02902019/ac2/0.2", file_name);
			lm_dir = os.path.join("Document_data", topic_class, 'bigram.lm');
			output_dir = os.path.join("Result/ac2", topic_class, file_name);
			print("Handling " + file_name);
			command = './CorrectWord -text ' + test_file_dir + ' -map_Big5 Big5-ZhuYin.map -map_ZhuYin ZhuYin-Big5.map -lm '+lm_dir+' -topic_class '+topic_class+' -order 2 > '+ output_dir;
			os.system(command);