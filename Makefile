PROJECT = interpreter
SOURCES = main.cpp interpreter.cpp parser.cpp scanner.cpp engine.cpp \
          vartable.cpp labtable.cpp array.cpp error.cpp common.cpp
HEADERS = $(filter-out main.hpp, $(SOURCES:.cpp=.hpp)) hashtable.hpp
OBJECTS = $(SOURCES:.cpp=.o)
CXX = g++
CXXFLAGS = -Wall -g -std=c++98 -D DEBUG=0
LDLIBS = -lm
CTAGS = /usr/bin/ctags
INSTALL = install
PREFIX = /usr/local

$(PROJECT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

deps.mk: $(SOURCES) Makefile
	$(CXX) -MM $(SOURCES) > $@

tags: $(SOURCES) $(HEADERS)
	$(CTAGS) $(SOURCES) $(HEADERS)

run: $(PROJECT)
	./$(PROJECT) scripts/script3.txt

tar:
	tar -cf $(PROJECT).tar $(SOURCES) $(HEADERS) Makefile README.txt scripts

clean:
	rm -f $(PROJECT) *.o deps.mk tags

install: $(PROJECT)
	$(INSTALL) $(PROJECT) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(PROJECT)

ifneq (unistall, $(MAKECMDGOALS))
ifneq (clean, $(MAKECMDGOALS))
ifneq (tags, $(MAKECMDGOALS))
ifneq (tar, $(MAKECMDGOALS))
-include deps.mk
endif
endif
endif
endif

