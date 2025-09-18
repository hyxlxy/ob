#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

// Simulate the improved arithmetic operations
template <class T>
T improved_multiply(T left, T right) {
    // Use double precision for float calculations to match MySQL 8.0 behavior
    if constexpr (std::is_same_v<T, float>) {
        double left_double = static_cast<double>(left);
        double right_double = static_cast<double>(right);
        double result_double = left_double * right_double;
        return static_cast<T>(result_double);
    } else {
        return left * right;
    }
}

template <class T>
T improved_add(T left, T right) {
    // Use double precision for float calculations to match MySQL 8.0 behavior
    if constexpr (std::is_same_v<T, float>) {
        double left_double = static_cast<double>(left);
        double right_double = static_cast<double>(right);
        double result_double = left_double + right_double;
        return static_cast<T>(result_double);
    } else {
        return left + right;
    }
}

template <class T>
T improved_subtract(T left, T right) {
    // Use double precision for float calculations to match MySQL 8.0 behavior
    if constexpr (std::is_same_v<T, float>) {
        double left_double = static_cast<double>(left);
        double right_double = static_cast<double>(right);
        double result_double = left_double - right_double;
        return static_cast<T>(result_double);
    } else {
        return left - right;
    }
}

template <class T>
T improved_negate(T input) {
    // Use double precision for float calculations to match MySQL 8.0 behavior
    if constexpr (std::is_same_v<T, float>) {
        double input_double = static_cast<double>(input);
        double result_double = -input_double;
        return static_cast<T>(result_double);
    } else {
        return -input;
    }
}

// Simulate the double_to_str function
std::string double_to_str(double v) {
    char buf[256];
    double scaled = v * 100.0;
    double bias = (scaled >= 0.0 ? 1e-8 : -1e-8);
    scaled += bias;
    double rounded_v = round(scaled) / 100.0;

    snprintf(buf, sizeof(buf), "%.2f", rounded_v);
    size_t len = strlen(buf);
    while (len > 0 && buf[len - 1] == '0') {
        len--;
    }
    if (len > 0 && buf[len - 1] == '.') {
        len--;
    }

    return std::string(buf, len);
}

void test_complex_expression_with_values(int col1, int col2, float col3, float col4) {
    std::cout << "Testing complex expression with improved precision:" << std::endl;
    std::cout << "Expression: -(col2*(-8)+1)+(col4+4)*(col1+col3*5)" << std::endl;
    std::cout << "Values: col1=" << col1 << ", col2=" << col2 << ", col3=" << col3 << ", col4=" << col4 << std::endl;
    
    std::cout << std::fixed << std::setprecision(10);
    
    // Calculate step by step using improved operations
    std::cout << "\nStep-by-step calculation:" << std::endl;
    
    // Step 1: col2 * (-8)
    float step1 = improved_multiply(static_cast<float>(col2), -8.0f);
    std::cout << "col2 * (-8) = " << step1 << std::endl;
    
    // Step 2: col2*(-8) + 1
    float step2 = improved_add(step1, 1.0f);
    std::cout << "col2*(-8) + 1 = " << step2 << std::endl;
    
    // Step 3: -(col2*(-8) + 1)
    float step3 = improved_negate(step2);
    std::cout << "-(col2*(-8) + 1) = " << step3 << std::endl;
    
    // Step 4: col3 * 5
    float step4 = improved_multiply(col3, 5.0f);
    std::cout << "col3 * 5 = " << step4 << std::endl;
    
    // Step 5: col1 + col3*5
    float step5 = improved_add(static_cast<float>(col1), step4);
    std::cout << "col1 + col3*5 = " << step5 << std::endl;
    
    // Step 6: col4 + 4
    float step6 = improved_add(col4, 4.0f);
    std::cout << "col4 + 4 = " << step6 << std::endl;
    
    // Step 7: (col4+4) * (col1+col3*5)
    float step7 = improved_multiply(step6, step5);
    std::cout << "(col4+4) * (col1+col3*5) = " << step7 << std::endl;
    
    // Step 8: Final result
    float final_result = improved_add(step3, step7);
    std::cout << "Final result = " << final_result << std::endl;
    
    // Convert to string using the same logic as the database
    std::string result_str = double_to_str(static_cast<double>(final_result));
    std::cout << "Formatted result: " << result_str << std::endl;
    
    // Compare with old method (direct float operations)
    std::cout << "\nComparison with direct float operations:" << std::endl;
    float old_step1 = col2 * (-8.0f);
    float old_step2 = old_step1 + 1.0f;
    float old_step3 = -old_step2;
    float old_step4 = col3 * 5.0f;
    float old_step5 = col1 + old_step4;
    float old_step6 = col4 + 4.0f;
    float old_step7 = old_step6 * old_step5;
    float old_final = old_step3 + old_step7;
    
    std::cout << "Old method result: " << old_final << std::endl;
    std::cout << "Old method formatted: " << double_to_str(static_cast<double>(old_final)) << std::endl;
    
    std::cout << "\nDifference: " << (final_result - old_final) << std::endl;
}

void test_complex_expression() {
    // Test the original problematic case
    std::cout << "=== Test Case 1: Original Data ===" << std::endl;
    test_complex_expression_with_values(4, 2, 4.55f, 8.24f);

    // Test some cases that might produce results around 197.88
    std::cout << "\n=== Test Case 2: Data that might produce ~197.88 ===" << std::endl;
    test_complex_expression_with_values(1, 1, 3.0f, 7.93f);

    std::cout << "\n=== Test Case 3: Another potential case ===" << std::endl;
    test_complex_expression_with_values(2, 2, 2.8f, 7.43f);

    std::cout << "\n=== Test Case 4: Edge case ===" << std::endl;
    test_complex_expression_with_values(3, 3, 2.6f, 6.93f);
}

int main() {
    test_complex_expression();
    return 0;
}
