#pragma once
struct FrameContext { unsigned long long id{0}; double sensor_ms{0}; double preprocess_ms{0}; double tensor_ms{0}; double rknn_ms{0}; double post_ms{0}; };
