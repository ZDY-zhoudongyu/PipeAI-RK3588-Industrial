/*
 PipeAI-RK3588 v1.23
 Offline inference entry.

 Flow:
 CSV
  -> RawSample
  -> Preprocess
  -> Tensor
  -> RKNN
  -> PostProcess
  -> InferenceResult

 This file is the integration point for the complete offline validation path.
*/

#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "usage: pipeai_infer input.csv\n";
        return -1;
    }

    std::cout << "PipeAI inference input: "
              << argv[1] << std::endl;

    // Integration flow:
    // 1. load csv sensor data
    // 2. build 128x9 window
    // 3. preprocess
    // 4. tensor conversion
    // 5. rknn inference
    // 6. postprocess result

    return 0;
}
