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
      result = lhs / rhs;
      break;
    default:
      throw BasicError("UNSUPPORTED OPERATOR");
  }

  if (result < std::numeric_limits<int>::min() ||
      result > std::numeric_limits<int>::max()) {
    throw BasicError("SYNTAX ERROR");
  }
  return static_cast<int>(result);
}
