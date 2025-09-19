#include <iostream>
#include <memory>
#include "common/value.h"
#include "sql/expr/expression.h"
#include "sql/parser/parse_defs.h"

using namespace std;

// Test IS NULL functionality
void test_is_null() {
    cout << "Testing IS NULL functionality:" << endl;
    
    // Create a constant value expression for integer 2
    Value int_value;
    int_value.set_int(2);
    auto left_expr = make_unique<ValueExpr>(int_value);
    
    // Create a NULL value expression for the right side (as done in parser)
    Value null_value;
    null_value.set_type(AttrType::UNDEFINED);
    null_value.set_null(true);
    auto right_expr = make_unique<ValueExpr>(null_value);
    
    // Create IS NULL comparison expression
    auto is_null_expr = make_unique<ComparisonExpr>(IS_NULL_OP, std::move(left_expr), std::move(right_expr));
    
    // Test try_get_value (constant folding)
    Value result;
    RC rc = is_null_expr->try_get_value(result);
    
    cout << "try_get_value result: ";
    if (rc == RC::SUCCESS) {
        cout << "SUCCESS, value = " << result.to_string() << endl;
        cout << "Expected: false (since 2 is not null)" << endl;
        
        if (result.attr_type() == AttrType::BOOLEANS && !result.get_boolean()) {
            cout << "✓ IS NULL test passed!" << endl;
        } else {
            cout << "✗ IS NULL test failed!" << endl;
        }
    } else {
        cout << "FAILED with rc = " << strrc(rc) << endl;
    }
}

// Test IS NOT NULL functionality
void test_is_not_null() {
    cout << "\nTesting IS NOT NULL functionality:" << endl;
    
    // Create a constant value expression for integer 2
    Value int_value;
    int_value.set_int(2);
    auto left_expr = make_unique<ValueExpr>(int_value);
    
    // Create a NULL value expression for the right side
    Value null_value;
    null_value.set_type(AttrType::UNDEFINED);
    null_value.set_null(true);
    auto right_expr = make_unique<ValueExpr>(null_value);
    
    // Create IS NOT NULL comparison expression
    auto is_not_null_expr = make_unique<ComparisonExpr>(IS_NOT_NULL_OP, std::move(left_expr), std::move(right_expr));
    
    // Test try_get_value (constant folding)
    Value result;
    RC rc = is_not_null_expr->try_get_value(result);
    
    cout << "try_get_value result: ";
    if (rc == RC::SUCCESS) {
        cout << "SUCCESS, value = " << result.to_string() << endl;
        cout << "Expected: true (since 2 is not null)" << endl;
        
        if (result.attr_type() == AttrType::BOOLEANS && result.get_boolean()) {
            cout << "✓ IS NOT NULL test passed!" << endl;
        } else {
            cout << "✗ IS NOT NULL test failed!" << endl;
        }
    } else {
        cout << "FAILED with rc = " << strrc(rc) << endl;
    }
}

// Test with actual NULL value
void test_null_is_null() {
    cout << "\nTesting NULL IS NULL functionality:" << endl;
    
    // Create a NULL value expression
    Value null_value;
    null_value.set_type(AttrType::UNDEFINED);
    null_value.set_null(true);
    auto left_expr = make_unique<ValueExpr>(null_value);
    
    // Create another NULL value expression for the right side
    Value null_value2;
    null_value2.set_type(AttrType::UNDEFINED);
    null_value2.set_null(true);
    auto right_expr = make_unique<ValueExpr>(null_value2);
    
    // Create IS NULL comparison expression
    auto is_null_expr = make_unique<ComparisonExpr>(IS_NULL_OP, std::move(left_expr), std::move(right_expr));
    
    // Test try_get_value (constant folding)
    Value result;
    RC rc = is_null_expr->try_get_value(result);
    
    cout << "try_get_value result: ";
    if (rc == RC::SUCCESS) {
        cout << "SUCCESS, value = " << result.to_string() << endl;
        cout << "Expected: true (since NULL is null)" << endl;
        
        if (result.attr_type() == AttrType::BOOLEANS && result.get_boolean()) {
            cout << "✓ NULL IS NULL test passed!" << endl;
        } else {
            cout << "✗ NULL IS NULL test failed!" << endl;
        }
    } else {
        cout << "FAILED with rc = " << strrc(rc) << endl;
    }
}

int main() {
    test_is_null();
    test_is_not_null();
    test_null_is_null();
    return 0;
}
