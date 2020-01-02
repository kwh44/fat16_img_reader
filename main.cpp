#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>

int main(int argv, char *argc[]) {
    
    if (argv < 2) return -1;

    std::ifstream input(std::string(argc[1]), std::ios::binary);

    std::vector<char> bytes(
         (std::istreambuf_iterator<char>(input)),
         (std::istreambuf_iterator<char>()));
    
    input.close();
    
    return 0;
}
