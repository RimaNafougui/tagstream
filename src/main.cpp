#include <iostream> 
#include <string> 
#include "tagstream/matcher.hpp"

int main(){
    std::string line;
    while(std::getline(std::cin, line)){
        std::cout << tagstream::to_lower(line) << '\n';
    }
}