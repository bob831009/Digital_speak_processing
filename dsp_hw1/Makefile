.PHONY: all clean run

CFLAGS+=
LDFLAGS+=-lm     # link to math library

TARGET=train.cpp test.cpp

all: $(TARGET)
	g++ train.cpp -o train
	g++ test.cpp -o test

clean:
	rm -f train test
testing:
	./test  modellist.txt  testing_data2.txt  result2.txt
training:
	./train  $(iter)  model_init.txt  seq_model_0$(model).txt model_0$(model).txt
