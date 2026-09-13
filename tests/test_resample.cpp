#include "preprocess/resample.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    {
        std::vector<float> src{0.0F, 10.0F, 20.0F};
        std::vector<float> out;
        const auto status = pipeai::resample_linear_normalized(src, 5, out);
        assert(status.ok());
        const float expected[] = {0.0F, 5.0F, 10.0F, 15.0F, 20.0F};
        assert(out.size() == 5);
        for (std::size_t i = 0; i < out.size(); ++i) {
            assert(std::fabs(out[i] - expected[i]) < 1e-6F);
        }
    }
    {
        std::vector<float> src{42.0F};
        std::vector<float> out;
        assert(pipeai::resample_linear_normalized(src, 128, out).ok());
        for (float v : out) assert(v == 42.0F);
    }

    std::cout << "resample tests passed\n";
    return 0;
}
