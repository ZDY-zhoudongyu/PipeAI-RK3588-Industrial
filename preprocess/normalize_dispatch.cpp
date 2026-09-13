#include "normalize_dispatch.h"
#ifdef __ARM_NEON
#include "arm_neon.h"
#endif
void normalize_dispatch(const float* in,float* out,int n,float mean,float inv_std){
#ifdef __ARM_NEON
 float32x4_t m=vdupq_n_f32(mean), s=vdupq_n_f32(inv_std); int i=0; for(;i+3<n;i+=4){auto x=vld1q_f32(in+i); x=vsubq_f32(x,m); x=vmulq_f32(x,s); vst1q_f32(out+i,x);} for(;i<n;i++) out[i]=(in[i]-mean)*inv_std;
#else
 for(int i=0;i<n;i++) out[i]=(in[i]-mean)*inv_std;
#endif
}
