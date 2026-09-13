#include "cpu_affinity.hpp"
#ifdef __linux__
#include <sched.h>
#endif
namespace pipeai { bool bindThreadCpu(pthread_t thread,int cpu){
#ifdef __linux__
 cpu_set_t set; CPU_ZERO(&set); CPU_SET(cpu,&set); return pthread_setaffinity_np(thread,sizeof(set),&set)==0;
#else
 (void)thread;(void)cpu; return false;
#endif
}}
