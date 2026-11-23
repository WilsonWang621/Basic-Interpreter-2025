#include <iostream>
#include <memory>
#include <string>

#include "../include/Lexer.hpp"
#include "../include/Parser.hpp"
#include "../include/Program.hpp"
#include "../include/Token.hpp"
#include "../include/utils/Error.hpp"

int main() {
  Lexer lexer;
  Parser parser;
  Program program;
  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty()) {
      continue;
    }
    try {
      // TODO: The main function.
      TokenStream tokens = lexer.tokenize(line);
      // 检查第一个 token 的类型
      const Token* firstToken = tokens.peek();   //cursor_ >= tokens_.size() 返回nullptr
      if (!firstToken) {
        continue; // 空行，跳过
      }

      // 如果是标识符，可能是解释器命令   没参数

        std::string command = firstToken->text;   //第一个对应的内容

        if (command == "RUN") {
          program.run();
          continue;
        } else if (command == "LIST") {
          // std::cout <<"in list" << std::endl;
          program.list();
          continue;
        } else if (command == "CLEAR") {
          program.clear();
          continue;
        } else if (command == "QUIT") {
          // std::cout << "in quit" << std::endl;
          break;
        } else if (command == "HELP") {
          std::cout << "支持的命令:\n"
                       "  RUN - 开始执行程序\n"
                       "  LIST - 列出当前所有的程序行\n"
                       "  CLEAR - 清除当前所有的程序行\n"
                       "  QUIT - 退出解释器\n"
                       "  HELP - 打印帮助信息\n\n"
                       "程序指令:\n"
                       "  [行号] REM <注释> - 注释行\n"
                       "  [行号] LET <变量> = <表达式> - 赋值语句\n"
                       "  [行号] PRINT <表达式> - 打印表达式值\n"
                       "  [行号] INPUT <变量> - 输入变量值\n"
                       "  [行号] END - 结束程序\n"
                       "  [行号] GOTO <行号> - 跳转到指定行\n"
                       "  [行号] IF <表达式1> <操作符> <表达式2> THEN <行号> - 条件跳转\n"
                       "  [行号] INDENT - 进入新的作用域块\n"
                       "  [行号] DEDENT - 退出当前作用域块\n\n"
                       "立即执行指令 (不带行号):\n"
                       "  LET <变量> = <表达式>\n"
                       "  PRINT <表达式>\n"
                       "  INPUT <变量>\n"
                       "  INDENT\n"
                       "  DEDENT\n";
          continue;
        }else if (command == "INDENT") {
          program.indent();
          continue;
        }else if (command == "DEDENT") {
          program.dedent();
          continue;
        }

      auto parsedLine = parser.parseLine(tokens, line);
      std::unique_ptr<Statement> statement(parsedLine.fetchStatement());
      if (const auto lineNumber = parsedLine.getLine()) {
        if (statement) {
          program.addStmt(*lineNumber, statement.get());
          statement.release();
        } else {
          program.removeStmt(*lineNumber);
        }
      } else {
        program.execute(statement.get());
      }
    } catch (const BasicError& e) {
      // std::cout << "in catch" << std::endl;
      std::cout << e.message() << "\n";
    }
  }
  return 0;
}
