#include <iostream>
#include <iomanip>
#include <memory>
#include "common/value.h"
#include "sql/expr/expression.h"

using namespace std;

// Helper function to create a ValueExpr
unique_ptr<Expression> make_value_expr(float value) {
    Value v;
    v.set_float(value);
    return make_unique<ValueExpr>(v);
}

unique_ptr<Expression> make_value_expr(int value) {
    Value v;
    v.set_int(value);
    return make_unique<ValueExpr>(v);
}

// Test the complex expression: -(col2*(-8)+1)+(col4+4)*(col1+col3*5)
// With values: col1=4, col2=2, col3=4.55, col4=8.24
void test_complex_expression() {
    cout << "Testing complex expression: -(col2*(-8)+1)+(col4+4)*(col1+col3*5)" << endl;
    cout << "With values: col1=4, col2=2, col3=4.55, col4=8.24" << endl;
    
    // Create value expressions for each column
    auto col1 = make_value_expr(4);
    auto col2 = make_value_expr(2);
    auto col3 = make_value_expr(4.55f);
    auto col4 = make_value_expr(8.24f);
    
    // Build the expression step by step
    // col2*(-8)
    auto neg8 = make_value_expr(-8);
    auto col2_mul_neg8 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::MUL, col2->copy(), neg8->copy());
    
    // col2*(-8)+1
    auto one = make_value_expr(1);
    auto col2_mul_neg8_plus1 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::ADD, col2_mul_neg8->copy(), one->copy());
    
    // -(col2*(-8)+1)
    auto neg_col2_mul_neg8_plus1 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::NEGATIVE, col2_mul_neg8_plus1->copy(), nullptr);
    
    // col3*5
    auto five = make_value_expr(5);
    auto col3_mul_5 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::MUL, col3->copy(), five->copy());
    
    // col1+col3*5
    auto col1_plus_col3_mul_5 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::ADD, col1->copy(), col3_mul_5->copy());
    
    // col4+4
    auto four = make_value_expr(4);
    auto col4_plus_4 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::ADD, col4->copy(), four->copy());
    
    // (col4+4)*(col1+col3*5)
    auto col4_plus_4_mul_col1_plus_col3_mul_5 = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::MUL, col4_plus_4->copy(), col1_plus_col3_mul_5->copy());
    
    // Final expression: -(col2*(-8)+1)+(col4+4)*(col1+col3*5)
    auto final_expr = make_unique<ArithmeticExpr>(ArithmeticExpr::Type::ADD, neg_col2_mul_neg8_plus1->copy(), col4_plus_4_mul_col1_plus_col3_mul_5->copy());
    
    // Create a dummy tuple (not used for constant expressions)
    class DummyTuple : public Tuple {
    public:
        RC cell_at(int index, Value &cell) const override { return RC::INTERNAL; }
        int cell_num() const override { return 0; }
        RC find_cell(const TupleCellSpec &spec, Value &cell) const override { return RC::INTERNAL; }
    };
    
    DummyTuple dummy_tuple;
    Value result;
    
    // Test individual components
    cout << "\nTesting individual components:" << endl;
    
    Value temp;
    col2_mul_neg8->try_get_value(temp);
    cout << "col2*(-8) = " << temp.to_string() << endl;
    
    col2_mul_neg8_plus1->try_get_value(temp);
    cout << "col2*(-8)+1 = " << temp.to_string() << endl;
    
    neg_col2_mul_neg8_plus1->try_get_value(temp);
    cout << "-(col2*(-8)+1) = " << temp.to_string() << endl;
    
    col3_mul_5->try_get_value(temp);
    cout << "col3*5 = " << temp.to_string() << endl;
    
    col1_plus_col3_mul_5->try_get_value(temp);
    cout << "col1+col3*5 = " << temp.to_string() << endl;
    
    col4_plus_4->try_get_value(temp);
    cout << "col4+4 = " << temp.to_string() << endl;
    
    col4_plus_4_mul_col1_plus_col3_mul_5->try_get_value(temp);
    cout << "(col4+4)*(col1+col3*5) = " << temp.to_string() << endl;
    
    // Test final result
    RC rc = final_expr->try_get_value(result);
    if (rc == RC::SUCCESS) {
        cout << "\nFinal result: " << result.to_string() << endl;
        cout << "Expected: around 197.88 (if this matches the problematic case)" << endl;
        
        // Check if the result is close to 197.88
        float result_float = result.get_float();
        if (abs(result_float - 197.88f) < 0.01f) {
            cout << "✓ Result is close to expected 197.88" << endl;
        } else if (abs(result_float - 197.87f) < 0.01f) {
            cout << "✗ Result is 197.87 (the problematic value)" << endl;
        } else {
            cout << "? Result is neither 197.87 nor 197.88" << endl;
        }
    } else {
        cout << "Failed to evaluate expression: " << strrc(rc) << endl;
    }
}

int main() {
    cout << fixed << setprecision(10);
    test_complex_expression();
    return 0;
}
