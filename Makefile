CXXFLAGS=-Wall -g

all: examples

#initializer_list: initializer_list.cpp
#	g++ $(CXXFLAGS) $< -o $@

examples: examples.cpp
	g++ $(CXXFLAGS) $< -o $@

clean:
	rm initializer_list print_array
