#include <chrono>
#include <iostream>
int main(){
    auto t=std::chrono::steady_clock::now();
    auto e=std::chrono::steady_clock::now();
    std::cout<<"Benchmark framework ready\n";
    return 0;
}
