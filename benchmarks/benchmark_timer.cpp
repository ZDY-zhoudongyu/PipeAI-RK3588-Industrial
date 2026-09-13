#include <chrono>
#include <iostream>

int main(){
    auto start=std::chrono::steady_clock::now();
    volatile double x=0;
    for(int i=0;i<1000000;i++) x+=i*0.001;
    auto end=std::chrono::steady_clock::now();
    std::cout << "benchmark_ms="
              << std::chrono::duration<double,std::milli>(end-start).count()
              << std::endl;
    return 0;
}
