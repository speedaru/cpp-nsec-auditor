#include <iostream>
#include <vector>

/**
 * @file nesting_test.cpp
 * @brief This file is designed to trigger the NestingDepthRule (Rule 252).
 */

void DeeplyNestedViolation() {
    // Level 1
    if (true) {
        // Level 2
        for (int i = 0; i < 10; ++i) {
            // Level 3
            if (i % 2 == 0) {
                // Level 4
                while (false) {
                    // Level 5: This should trigger the depth violation (Max default: 4)
                    {
                        std::cout << "Level 5 reached." << std::endl;
                    }
                }
            }
        }
    }
}

void DeepAndLongViolation() {
    // Level 1
    if (true) {
        // Level 2
        for (int i = 0; i < 1; ++i) {
            // Level 3
            if (i == 0) {
                // Deep ( >= 3) 
                // We will add more than 20 lines here to trigger the "Long Block" metric.
                
                // part 1
                std::cout << "Line 1" << std::endl;
                std::cout << "Line 2" << std::endl;
                std::cout << "Line 3" << std::endl;
                std::cout << "Line 4" << std::endl;
                std::cout << "Line 5" << std::endl;
                std::cout << "Line 6" << std::endl;
                std::cout << "Line 7" << std::endl;
                std::cout << "Line 8" << std::endl;
                std::cout << "Line 9" << std::endl;
                std::cout << "Line 10" << std::endl;

                // part 2
                std::cout << "Line 11" << std::endl;
                std::cout << "Line 12" << std::endl;
                std::cout << "Line 13" << std::endl;
                std::cout << "Line 14" << std::endl;
                std::cout << "Line 15" << std::endl;
                std::cout << "Line 16" << std::endl;
                std::cout << "Line 17" << std::endl;
                std::cout << "Line 18" << std::endl;

                {
                    std::cout << "Line 19" << std::endl;
                    std::cout << "Line 20" << std::endl;
                    std::cout << "Line 21" << std::endl;
                    std::cout << "Line 22 - This block is now > 20 lines and nested at level 4." << std::endl;
                }
            }
        }
    }
}

int main() {
    DeeplyNestedViolation();
    DeepAndLongViolation();
    return 0;
}
