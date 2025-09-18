#include <iostream>
#include <iomanip>
#include <cmath>

// Simulate the calculation that should produce 197.88 but produces 197.87
int main() {
    // Test data: col1=4, col2=2, col3=4.55, col4=8.24
    // But this doesn't match the expected result, so let me try to find the correct data
    
    // Let's try to reverse engineer what data would produce 197.88
    // Expression: -(col2*(-8)+1)+(col4+4)*(col1+col3*5)
    
    std::cout << std::fixed << std::setprecision(10);
    
    // Test case 1: Original data
    {
        int col1 = 4, col2 = 2;
        float col3 = 4.55f, col4 = 8.24f;
        
        std::cout << "=== Test Case 1: Original Data ===" << std::endl;
        std::cout << "col1=" << col1 << ", col2=" << col2 << ", col3=" << col3 << ", col4=" << col4 << std::endl;
        
        // Calculate step by step using float precision
        float step1 = col2 * (-8);  // 2 * (-8) = -16
        float step2 = step1 + 1;    // -16 + 1 = -15
        float step3 = -step2;       // -(-15) = 15
        float step4 = col3 * 5;     // 4.55 * 5 = 22.75
        float step5 = col1 + step4; // 4 + 22.75 = 26.75
        float step6 = col4 + 4;     // 8.24 + 4 = 12.24
        float step7 = step6 * step5; // 12.24 * 26.75
        float result = step3 + step7;
        
        std::cout << "Step 1 (col2*(-8)): " << step1 << std::endl;
        std::cout << "Step 2 (step1+1): " << step2 << std::endl;
        std::cout << "Step 3 (-step2): " << step3 << std::endl;
        std::cout << "Step 4 (col3*5): " << step4 << std::endl;
        std::cout << "Step 5 (col1+step4): " << step5 << std::endl;
        std::cout << "Step 6 (col4+4): " << step6 << std::endl;
        std::cout << "Step 7 (step6*step5): " << step7 << std::endl;
        std::cout << "Final result: " << result << std::endl;
        
        // Now calculate using double precision
        std::cout << "\n--- Using Double Precision ---" << std::endl;
        double d_step1 = static_cast<double>(col2) * (-8.0);
        double d_step2 = d_step1 + 1.0;
        double d_step3 = -d_step2;
        double d_step4 = static_cast<double>(col3) * 5.0;
        double d_step5 = static_cast<double>(col1) + d_step4;
        double d_step6 = static_cast<double>(col4) + 4.0;
        double d_step7 = d_step6 * d_step5;
        double d_result = d_step3 + d_step7;
        
        std::cout << "Double Step 7: " << d_step7 << std::endl;
        std::cout << "Double Final result: " << d_result << std::endl;
        std::cout << "Converted to float: " << static_cast<float>(d_result) << std::endl;
    }
    
    // Let me try to find data that would produce around 197.88
    std::cout << "\n=== Searching for data that produces ~197.88 ===" << std::endl;
    
    // Try different combinations
    for (int col1 = 1; col1 <= 10; col1++) {
        for (int col2 = 1; col2 <= 5; col2++) {
            for (int col3_int = 200; col3_int <= 600; col3_int++) {
                for (int col4_int = 400; col4_int <= 900; col4_int++) {
                    float col3 = col3_int / 100.0f;
                    float col4 = col4_int / 100.0f;
                    
                    double d_step1 = static_cast<double>(col2) * (-8.0);
                    double d_step2 = d_step1 + 1.0;
                    double d_step3 = -d_step2;
                    double d_step4 = static_cast<double>(col3) * 5.0;
                    double d_step5 = static_cast<double>(col1) + d_step4;
                    double d_step6 = static_cast<double>(col4) + 4.0;
                    double d_step7 = d_step6 * d_step5;
                    double d_result = d_step3 + d_step7;
                    
                    if (std::abs(d_result - 197.88) < 0.01) {
                        std::cout << "Found match: col1=" << col1 << ", col2=" << col2 
                                  << ", col3=" << col3 << ", col4=" << col4 
                                  << " -> result=" << d_result << std::endl;
                    }
                }
            }
        }
    }
    
    return 0;
}
