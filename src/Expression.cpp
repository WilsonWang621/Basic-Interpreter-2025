#include "Expression.hpp"

#include <limits>

#include "VarState.hpp"
#include "utils/Error.hpp"

ConstExpression::ConstExpression(int value) : value_(value) {}

int ConstExpression::evaluate(const VarState&) const { return value_; }

VariableExpression::VariableExpression(std::string name)
    : name_(std::move(name)) {}

int VariableExpression::evaluate(const VarState& state) const {
  return state.getValue(name_);
}

CompoundExpression::CompoundExpression(Expression* left, char op,
                                       Expression* right)
    : left_(left), right_(right), op_(op) {}

CompoundExpression::~CompoundExpression() {
  delete left_;
  delete right_;
}

int CompoundExpression::evaluate(const VarState& state) const {
  constexpr long long maxValue = std::numeric_limits<int>::max();
  const long long lhs = left_->evaluate(state);
  const long long rhs = right_->evaluate(state);
  long long result;

  switch (op_) {
    case '+':
      result = lhs + rhs;
      break;
    case '-':
      result = lhs - rhs;
      break;
    case '*':
      result = lhs * rhs;
      break;
    case '/':
      if (rhs == 0) {
        throw BasicError("DIVIDE BY ZERO");
      }
      if (lhs == std::numeric_limits<int>::min() && rhs == -1) {
        throw BasicError("SYNTAX ERROR");
      }
      return static_cast<int>(lhs / rhs);
    default:
      throw BasicError("UNSUPPORTED OPERATOR");
  }

  // 加减乘按标程的 32 位结果回绕，避免有符号整数溢出。
  constexpr long long range = maxValue - std::numeric_limits<int>::min() + 1;
  const long long wrapped = static_cast<unsigned int>(result);
  return static_cast<int>(wrapped > maxValue ? wrapped - range : wrapped);
}
