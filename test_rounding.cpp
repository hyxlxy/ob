#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <vector>

// Simulate the double_to_str function
std::string double_to_str(double v) {
    char buf[256];
    // Use a tiny bias before rounding to 2 decimals to better match MySQL's
    // double-precision evaluation when the stored value is a 32-bit float.
    // This compensates for cases like 197.875 being represented as 197.874999... in float.
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

int main() {
    std::cout << std::fixed << std::setprecision(15);
    
    // Test values around 197.87 and 197.88
    std::vector<double> test_values = {
        197.870,
        197.871,
        197.872,
        197.873,
        197.874,
        197.8745,
        197.875,
        197.8755,
        197.876,
        197.877,
        197.878,
        197.879,
        197.880,
        197.8749999,
        197.8750001,
        197.8799999,
        197.8800001
    };
    
    std::cout << "Testing double_to_str function:" << std::endl;
    for (double val : test_values) {
        std::string result = double_to_str(val);
        std::cout << "Input: " << val << " -> Output: " << result << std::endl;
    }
    
    // Test the specific case that might be causing the issue
    std::cout << "\nTesting potential problematic values:" << std::endl;
    
    // Simulate what might happen in the actual calculation
    float f1 = 197.875f;  // This might be stored as 197.874999... in float
    double d1 = static_cast<double>(f1);
    std::cout << "float 197.875f as double: " << d1 << " -> " << double_to_str(d1) << std::endl;
    
    float f2 = 197.87f;
    double d2 = static_cast<double>(f2);
    std::cout << "float 197.87f as double: " << d2 << " -> " << double_to_str(d2) << std::endl;
    
    float f3 = 197.88f;
    double d3 = static_cast<double>(f3);
    std::cout << "float 197.88f as double: " << d3 << " -> " << double_to_str(d3) << std::endl;
    
    return 0;
}
