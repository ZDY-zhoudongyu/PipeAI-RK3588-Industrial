#include "rknn_context.hpp"
namespace pipeai {
bool RknnContext::init(const std::string&){return true;}
void RknnContext::release(){ctx_=nullptr;}
bool RknnContext::valid() const{return ctx_!=nullptr;}
}
