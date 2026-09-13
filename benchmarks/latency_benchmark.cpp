#include <chrono>
#include <iostream>
int main(){
 auto t0=std::chrono::steady_clock::now();
 auto t1=std::chrono::steady_clock::now();
 std::cout<<"PipeAI latency benchmark skeleton: "
 <<std::chrono::duration<double,std::milli>(t1-t0).count()<<" ms\n";
 return 0;
}
