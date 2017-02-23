# The following two variable will be commandline determined by TA
# For testing, you could uncomment them.
SRIPATH ?= /nfs/undergrad/02/b02902015/DSP/srilm-1.5.10
MACHINE_TYPE ?= i686-m64
LM ?= bigram.lm

CXX = g++
CXXFLAGS = -O3 -I$(SRIPATH)/include -w
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = CorrectWord
SRC = CorrectWord.cpp
OBJ = $(SRC:.cpp=.o)
TO = ZhuYin-Big5.map
FROM = Big5-ZhuYin.map
MAP_PROGRAM = mapping.cpp


.PHONY: all clean map run

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
		$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
		$(CXX) $(CXXFLAGS) -c $<
run:
		@#TODO How to run your code toward different txt? 
		./CorrectWord -text example_ans.txt -map_Big5 $(FROM) -map_ZhuYin $(TO) -lm $(LM) -order 2;

compile_map:
		$(CXX) $(MAP_PROGRAM) -o $(MAP_PROGRAM:.cpp=)
map:
		@#TODO How to map?
		@echo "Mapping!"
		@./mapping $(FROM) $(TO)
		@#matlab mapping.m $(FROM) $(TO)
		@#python mapping.py $(FROM) $(TO)
		@#sh mapping.sh $(FROM) $(TO)
		@#perl mapping.pl Big5-ZhuYin.map ZhuYin-Big5.map
clean:
		$(RM) $(OBJ) $(TARGET)

check:
		@for i in $(shell seq 1 10) ; do \
			echo "Checking $$i.txt"; \
			diff result1/$$i.txt result2/$$i.txt; \
		done;

ans:
		@for i in $(shell seq 1 10) ; do \
			echo "Making Ans $$i.txt"; \
			$(SRIPATH)/bin/$(MACHINE_TYPE)/disambig  -text testdata/$$i.txt -map $(TO) -lm $(LM) -order 2 > result1/$$i.txt; \
		done;
