#include "postprocess/classifier.hpp"
#include "postprocess/softmax.hpp"
#include <algorithm>
namespace pipeai {
void Classifier::classify(InferenceResult& r){
 r.probabilities=Softmax::apply(r.logits);
 auto it=std::max_element(r.probabilities.begin(),r.probabilities.end());
 r.class_id=static_cast<int>(it-r.probabilities.begin()); r.confidence=*it;
}
const char* Classifier::label(int id){ static const char* n[]={"normal","collapse","bulge","flip","torsion"}; return id>=0&&id<5?n[id]:"unknown"; }
}
