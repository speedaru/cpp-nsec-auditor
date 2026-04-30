#include <iostream>
#include <cstring>
#include <cstdio>

/**
 * @brief This file is a 'cobaye' (guinea pig) for the nsec-audit tool.
 * It intentionally contains security violations and complex code.
 */

void ProcessData(const char* src) {
    char dest[10];
    char buffer[100];
    const char* str = "example";

    // Rule 101: Critical - Banned function 'strcpy'
    // This is a classic buffer overflow vulnerability.
    strcpy(dest, src);

    // Rule 102: Warning - Banned function 'sprintf'
    // Prefer snprintf to avoid overflowing 'buffer'.
    sprintf(buffer, "%s", str);

    std::cout << "Buffer content: " << buffer << std::endl;
}

/**
 * @brief Rule 200: High Cyclomatic Complexity
 * This function contains multiple nested decision points (if, for, while, switch, &&, ||).
 * Total decision points here: 13 (> 10 threshold).
 */
void DeeplyNestedLogic(int a, int b, int c) {
    // 1. if
    if (a > 0 && b > 0) { // 2. &&
        // 3. for
        for (int i = 0; i < a; ++i) {
            // 4. if
            if (i % 2 == 0 || i == b) { // 5. ||
                std::cout << "Even or Match";
            } else { // 6. else
                // 7. switch
                switch (c) {
                    case 1: // 8. case
                        std::cout << "One";
                        break;
                    case 2: // 9. case
                        // 10. while
                        while (b > 0) {
                            // 11. if
                            if (b-- == 1) break;
                        }
                        break;
                    default:
                        // 12. if
                        if (a == b) std::cout << "Equal";
                        break;
                }
            }
        }
    } else { // 13. else
        std::cout << "Invalid input";
    }
}

int main() {
    ProcessData("This string is way too long for the destination buffer");
    DeeplyNestedLogic(10, 5, 2);
    return 0;
}
