#include "Parser.hpp"

#include <charconv>
#include <sstream>
#include <string>
#include <vector>

#include "Expression.hpp"
#include "Statement.hpp"
#include "utils/Error.hpp"

ParsedLine::ParsedLine() { statement_ = nullptr; }

ParsedLine::~ParsedLine() = default;

void ParsedLine::setLine(int line) { line_number_.emplace(line); }

std::optional<int> ParsedLine::getLine() { return line_number_; }

void ParsedLine::setStatement(Statement* stmt) { statement_ = stmt; }

Statement* ParsedLine::getStatement() const { return statement_; }

Statement* ParsedLine::fetchStatement() {
  Statement* temp = statement_;
  statement_ = nullptr;
  return temp;
}

ParsedLine Parser::parseLine(TokenStream& tokens,
                             const std::string& originLine) const {
  ParsedLine result;

  // 检查是否有行号
  const Token* firstToken = tokens.peek();
  if (firstToken && firstToken->type == TokenType::NUMBER) {
    result.setLine(parseLiteral(firstToken));
    tokens.get();  // 消费行号token

    // 如果只有行号，表示删除该行
    if (tokens.empty()) {
      return result;            //这里真删除了吗
    }
  }

  // 解析语句
  result.setStatement(parseStatement(tokens, originLine));

  return result;
}

Statement* Parser::parseStatement(TokenStream& tokens,
                                  const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  const Token* token = tokens.get();
  if (!token) {
    throw BasicError("SYNTAX ERROR");
  }

  switch (token->type) {
    case TokenType::LET:
      return parseLet(tokens, originLine);
    case TokenType::PRINT:
      return parsePrint(tokens, originLine);
    case TokenType::INPUT:
      return parseInput(tokens, originLine);
    case TokenType::GOTO:
      return parseGoto(tokens, originLine);
    case TokenType::IF:
      return parseIf(tokens, originLine);
    case TokenType::REM:
      return parseRem(tokens, originLine);
    case TokenType::END:
      return parseEnd(tokens, originLine);
    case TokenType::INDENT:
      return parseIndent(tokens, originLine);
    case TokenType::DEDENT:
      return parseDedent(tokens, originLine);
    default:
      throw BasicError("SYNTAX ERROR");
  }
}

Statement* Parser::parseLet(TokenStream& tokens,
                            const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  const Token* varToken = tokens.get();
  if (!varToken || varToken->type != TokenType::IDENTIFIER) {
    throw BasicError("SYNTAX ERROR");
  }

  std::string varName = varToken->text;

  if (tokens.empty() || tokens.get()->type != TokenType::EQUAL) {
    throw BasicError("SYNTAX ERROR");
  }

  auto expr = parseExpression(tokens);
  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  auto* statement = new LetStatement(varName, expr.get(), originLine);
  expr.release();
  return statement;
}

Statement* Parser::parsePrint(TokenStream& tokens,
                              const std::string& originLine) const {
  auto expr = parseExpression(tokens);
  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  auto* statement = new PrintStatement(originLine, expr.get());
  expr.release();
  return statement;
}

Statement* Parser::parseInput(TokenStream& tokens,
                              const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  const Token* varToken = tokens.get();
  if (!varToken || varToken->type != TokenType::IDENTIFIER) {
    throw BasicError("SYNTAX ERROR");
  }

  std::string varName = varToken->text;
  // TODO: create a corresponding stmt and return it.
  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  return new InputStatement(varName, originLine);
}

Statement* Parser::parseGoto(TokenStream& tokens,
                             const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  const Token* lineToken = tokens.get();
  if (!lineToken || lineToken->type != TokenType::NUMBER) {
    throw BasicError("SYNTAX ERROR");
  }

  int targetLine = parseLiteral(lineToken);
  // TODO: create a corresponding stmt and return it.

  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  return new GotoStatement(targetLine, originLine);
}

Statement* Parser::parseIf(TokenStream& tokens,
                           const std::string& originLine) const {
  // 解析左表达式
  auto leftExpr = parseExpression(tokens);

  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  // 解析比较操作符
  const Token* opToken = tokens.get();
  char op;
  switch (opToken->type) {
    case TokenType::EQUAL:
      op = '=';
      break;
    case TokenType::GREATER:
      op = '>';
      break;
    case TokenType::LESS:
      op = '<';
      break;
    default:
      throw BasicError("SYNTAX ERROR");
  }

  // 解析右表达式
  auto rightExpr = parseExpression(tokens);

  // 检查THEN关键字
  if (tokens.empty() || tokens.get()->type != TokenType::THEN) {
    throw BasicError("SYNTAX ERROR");
  }

  // 解析目标行号
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  const Token* lineToken = tokens.get();
  if (!lineToken || lineToken->type != TokenType::NUMBER) {
    throw BasicError("SYNTAX ERROR");
  }

  int targetLine = parseLiteral(lineToken);

  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  auto* statement =
      new IFStatement(targetLine, originLine, leftExpr.get(), rightExpr.get(), op);
  leftExpr.release();
  rightExpr.release();
  return statement;
}

Statement* Parser::parseRem(TokenStream& tokens,
                            const std::string& originLine) const {
  const Token* remInfo = tokens.get();
  if (!remInfo || remInfo->type != TokenType::REMINFO) {
    throw BasicError("SYNTAX ERROR");
  }
  // TODO: create a corresponding stmt and return it.
  std::string comment = remInfo->text;
  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  return new RemStatement(originLine, comment);
}

Statement* Parser::parseEnd(TokenStream& tokens,
                            const std::string& originLine) const {
  // TODO: create a corresponding stmt and return it.
  if (!tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }

  return new EndStatement(originLine);
}

// to add scope function
Statement* Parser::parseIndent(TokenStream& tokens,
                               const std::string& originLine) const {
  if (!tokens.empty()) {  //shouldn't have anything behind
    throw BasicError("SYNTAX ERROR");
  }

  return new IndentStatement(originLine);
}

Statement* Parser::parseDedent(TokenStream& tokens,
                               const std::string& originLine) const {
  if (!tokens.empty()) {  //shouldn't have anything behind
    throw BasicError("SYNTAX ERROR");
  }

  return new DedentStatement(originLine);
}


std::unique_ptr<Expression> Parser::parseExpression(TokenStream& tokens) const {
  auto expression = parseExpression(tokens, 0);
  const auto* next = tokens.peek();
  if (next && next->type == TokenType::RIGHT_PAREN) {
    throw BasicError("MISMATCHED PARENTHESIS");
  }
  return expression;
}

std::unique_ptr<Expression> Parser::parseExpression(TokenStream& tokens,
                                                    int precedence) const {
  // 解析左操作数
  std::unique_ptr<Expression> left;
  const Token* token = tokens.get();
  if (!token) {
    throw BasicError("SYNTAX ERROR");
  }

  if (token->type == TokenType::NUMBER) {
    left = std::make_unique<ConstExpression>(parseLiteral(token));
  } else if (token->type == TokenType::IDENTIFIER) {
    left = std::make_unique<VariableExpression>(token->text);
  } else if (token->type == TokenType::LEFT_PAREN) {
    left = parseExpression(tokens, 0);

    if (tokens.empty() || tokens.get()->type != TokenType::RIGHT_PAREN) {
      throw BasicError("MISMATCHED PARENTHESIS");
    }
  } else {
    throw BasicError("SYNTAX ERROR");
  }

  // 检查是否有运算符
  while (const auto* opToken = tokens.peek()) {
    // 检查是否是运算符
    int opPrecedence = getPrecedence(opToken->type);
    if (opPrecedence == -1 || opPrecedence < precedence) {
      break;
    }

    tokens.get();  // 消费运算符token

    char op;
    switch (opToken->type) {
      case TokenType::PLUS:
        op = '+';
        break;
      case TokenType::MINUS:
        op = '-';
        break;
      case TokenType::MUL:
        op = '*';
        break;
      case TokenType::DIV:
        op = '/';
        break;
      default:
        throw BasicError("SYNTAX ERROR");
    }

    // 解析右操作数，使用更高的优先级
    auto right = parseExpression(tokens, opPrecedence + 1);
    auto expression =
        std::make_unique<CompoundExpression>(left.get(), op, right.get());
    left.release();
    right.release();
    left = std::move(expression);
  }

  return left;
}

int Parser::getPrecedence(TokenType op) const {
  switch (op) {
    case TokenType::PLUS:
    case TokenType::MINUS:
      return 1;
    case TokenType::MUL:
    case TokenType::DIV:
      return 2;
    default:
      return -1;
  }
}

int Parser::parseLiteral(const Token* token) const {
  if (!token || token->type != TokenType::NUMBER) {
    throw BasicError("SYNTAX ERROR");
  }

  int value;
  const char* begin = token->text.data();
  const char* end = begin + token->text.size();
  const auto parsed = std::from_chars(begin, end, value);
  if (parsed.ptr == end && parsed.ec == std::errc::result_out_of_range) {
    throw BasicError("INT LITERAL OVERFLOW");
  }
  if (parsed.ec != std::errc{} || parsed.ptr != end) {
    throw BasicError("SYNTAX ERROR");
  }
  return value;
}
