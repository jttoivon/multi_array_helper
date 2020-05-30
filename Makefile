CXXFLAGS=-Wall -g

all: print_array #initializer_list

#initializer_list: initializer_list.cpp
#	g++ $(CXXFLAGS) $< -o $@

print_array: print_array.cpp
	g++ $(CXXFLAGS) $< -o $@

clean:
	rm initializer_list print_array
