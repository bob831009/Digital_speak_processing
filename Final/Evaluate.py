import os

Total_sentence_error = 0;
Total_sentence_num = 0;
Total_term_error = 0;
Total_term_num = 0;


for topic_index in range(4):
	Topic_sentence_error = 0;
	Topic_sentence_num = 0;
	Topic_term_error = 0;
	Topic_term_num = 0;

	topic_class = "topic" + str(topic_index);
	filename_dir = os.path.join("Document_data", topic_class, topic_class+'_filename.txt');
	fp = open(filename_dir);

	for line in fp:
		line = line.split(" ");
		for filename in line:
			if(filename == ''):
				break;
			ans_filename = filename[:5];
			# print(ans_filename);
			test_file1_dir = os.path.join('Result/ac8', topic_class, filename);
			test_file2_dir = os.path.join('/tmp2/b02902019/ans', ans_filename);

			test_file1 = open(test_file1_dir, errors='ignore');
			test_file2 = open(test_file2_dir, errors='ignore');

			for test_line1 in test_file1:
				Total_sentence_num += 1;
				Topic_sentence_num += 1;
				test_line2 = test_file2.readline();
				# test_line2.encode('big5');

				test_line1 = test_line1.strip().split(' ');
				test_line1.pop();
				test_line1.pop(0);
				test_line2 = test_line2.strip().split(' ');
				
				# print(len(test_line1));
				# print(len(test_line2));
				condiction = 0;
				for i in range(len(test_line1)):
					Total_term_num += 1;
					Topic_term_num += 1;
					if(test_line1[i] != test_line2[i]):
						Total_term_error += 1;
						Topic_term_error += 1;
						condiction = 1;
				if(condiction != 0):
					Total_sentence_error += 1;
					Topic_sentence_error += 1;

	print("Handling "+ topic_class);
	print("Topic sentence accuracy: %lf" % (1-float(Topic_sentence_error)/Topic_sentence_num));
	print("Topic term accuracy: %lf" % (1-float(Topic_term_error)/Topic_term_num));
	print("==========================");

print('Total sentence accuracy: %lf' % (1-float(Total_sentence_error)/Total_sentence_num));
print('Total term accuracy: %lf' % (1-float(Total_term_error)/Total_term_num));

