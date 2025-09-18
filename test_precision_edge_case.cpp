#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

// Simulate the old (problematic) arithmetic operations
template <class T>
T old_multiply(T left, T right) {
    return left * right;  // Direct float multiplication
}

template <class T>
T old_add(T left, T right) {
    return left + right;  // Direct float addition
}

template <class T>
T old_subtract(T left, T right) {
    return left - right;  // Direct float subtraction
}

template <class T>
T old_negate(T input) {
    return -input;  // Direct float negation
}

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

void compare_methods(int col1, int col2, float col3, float col4) {
    std::cout << "Comparing old vs improved methods:" << std::endl;
    std::cout << "Values: col1=" << col1 << ", col2=" << col2 << ", col3=" << col3 << ", col4=" << col4 << std::endl;
    
    std::cout << std::fixed << std::setprecision(10);
    
    // Old method calculation
    std::cout << "\n--- Old Method (Direct Float Operations) ---" << std::endl;
    float old_step1 = old_multiply(static_cast<float>(col2), -8.0f);
    float old_step2 = old_add(old_step1, 1.0f);
    float old_step3 = old_negate(old_step2);
    float old_step4 = old_multiply(col3, 5.0f);
    float old_step5 = old_add(static_cast<float>(col1), old_step4);
    float old_step6 = old_add(col4, 4.0f);
    float old_step7 = old_multiply(old_step6, old_step5);
    float old_final = old_add(old_step3, old_step7);
    
    std::cout << "Old final result: " << old_final << std::endl;
    std::cout << "Old formatted: " << double_to_str(static_cast<double>(old_final)) << std::endl;
    
    // Improved method calculation
    std::cout << "\n--- Improved Method (Double Precision) ---" << std::endl;
    float new_step1 = improved_multiply(static_cast<float>(col2), -8.0f);
    float new_step2 = improved_add(new_step1, 1.0f);
    float new_step3 = improved_negate(new_step2);
    float new_step4 = improved_multiply(col3, 5.0f);
    float new_step5 = improved_add(static_cast<float>(col1), new_step4);
    float new_step6 = improved_add(col4, 4.0f);
    float new_step7 = improved_multiply(new_step6, new_step5);
    float new_final = improved_add(new_step3, new_step7);
    
    std::cout << "Improved final result: " << new_final << std::endl;
    std::cout << "Improved formatted: " << double_to_str(static_cast<double>(new_final)) << std::endl;
    
    // Show difference
    std::cout << "\nDifference: " << (new_final - old_final) << std::endl;
    
    // Check if this fixes the 197.87 -> 197.88 issue
    std::string old_str = double_to_str(static_cast<double>(old_final));
    std::string new_str = double_to_str(static_cast<double>(new_final));
    
    if (old_str != new_str) {
        std::cout << "*** PRECISION FIX DETECTED! ***" << std::endl;
        std::cout << "Old result: " << old_str << " -> New result: " << new_str << std::endl;
    } else {
        std::cout << "No difference in formatted output." << std::endl;
    }
}

// Try to create a case that might show the precision difference
void test_edge_cases() {
    std::cout << "=== Testing Edge Cases for Precision Issues ===" << std::endl;
    
    // Test cases that might be close to the 197.875 boundary
    // where rounding could go either way
    
    // Case 1: Try to create a result very close to 197.875
    std::cout << "\n--- Case 1: Targeting 197.875 boundary ---" << std::endl;
    compare_methods(1, 1, 3.0f, 7.925f);  // Slightly different from 7.93
    
    // Case 2: Try with slightly different precision
    std::cout << "\n--- Case 2: Precision variation ---" << std::endl;
    compare_methods(1, 1, 2.999f, 7.931f);
    
    // Case 3: Try with values that might accumulate precision errors
    std::cout << "\n--- Case 3: Accumulation test ---" << std::endl;
    compare_methods(2, 2, 2.7999f, 7.4299f);
    
    // Case 4: Test with the exact problematic values if we can find them
    std::cout << "\n--- Case 4: Potential problematic case ---" << std::endl;
    compare_methods(4, 2, 4.549999f, 8.240001f);  // Slightly modified original values
}

int main() {
    test_edge_cases();
    return 0;
}
