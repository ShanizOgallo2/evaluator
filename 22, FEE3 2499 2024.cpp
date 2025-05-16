#include <iostream>
#include <stack>
#include <sstream>
#include <map>
#include <vector>
#include <cctype>
#include <cmath>
#include <functional>
#include <iomanip>
#include <algorithm>

using namespace std;

enum TokenType { NUMBER, OPERATOR, VARIABLE, FUNCTION, PARENTHESIS };

struct Token {
    TokenType type;
    string value;
};

map<string, int> precedence = {
    {"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"^", 3}
};

map<string, function<double(vector<double>)>> functions = {
    {"sin", [](vector<double> args) { return sin(args[0] * M_PI / 180); }},
    {"cos", [](vector<double> args) { return cos(args[0] * M_PI / 180); }},
    {"sqrt", [](vector<double> args) { return sqrt(args[0]); }},
    {"log", [](vector<double> args) { return log10(args[0]); }}
};

map<string, double> variables = {
    {"pi", 3.1415926535},
    {"e", 2.7182818284}
};

bool isFunction(const string& s) {
    return functions.find(s) != functions.end();
}

bool isOperator(const string& op) {
    return precedence.find(op) != precedence.end();
}

vector<Token> tokenize(const string& expr) {
    vector<Token> tokens;
    string temp;
    for (size_t i = 0; i < expr.length(); ++i) {
        char ch = expr[i];
        if (isspace(ch)) continue;

        if (isdigit(ch) || ch == '.') {
            temp.clear();
            while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) {
                temp += expr[i++];
            }
            --i;
            tokens.push_back({NUMBER, temp});
        } else if (isalpha(ch)) {
            temp.clear();
            while (i < expr.length() && isalnum(expr[i])) {
                temp += expr[i++];
            }
            --i;
            if (isFunction(temp)) {
                tokens.push_back({FUNCTION, temp});
            } else {
                // Insert implicit '*' if previous token is NUMBER, VARIABLE, or ')'
                if (!tokens.empty() &&
                    (tokens.back().type == NUMBER || tokens.back().type == VARIABLE ||
                     (tokens.back().type == PARENTHESIS && tokens.back().value == ")"))) {
                    tokens.push_back({OPERATOR, "*"});
                }
                tokens.push_back({VARIABLE, temp});
            }
        } else if (ch == '(' || ch == ')') {
            // Insert implicit '*' if previous token is NUMBER, VARIABLE, or ')'
            if (ch == '(' && !tokens.empty() &&
                (tokens.back().type == NUMBER || tokens.back().type == VARIABLE ||
                 (tokens.back().type == PARENTHESIS && tokens.back().value == ")"))) {
                tokens.push_back({OPERATOR, "*"});
            }
            tokens.push_back({PARENTHESIS, string(1, ch)});
        } else {
            tokens.push_back({OPERATOR, string(1, ch)});
        }
    }
    return tokens;
}

vector<Token> infixToPostfix(const vector<Token>& tokens) {
    vector<Token> output;
    stack<Token> opStack;
    for (const auto& token : tokens) {
        if (token.type == NUMBER || token.type == VARIABLE) {
            output.push_back(token);
        } else if (token.type == FUNCTION) {
            opStack.push(token);
        } else if (token.type == OPERATOR) {
            while (!opStack.empty() &&
                   ((opStack.top().type == FUNCTION) ||
                    (opStack.top().type == OPERATOR &&
                     precedence[opStack.top().value] >= precedence[token.value]))) {
                output.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        } else if (token.value == "(") {
            opStack.push(token);
        } else if (token.value == ")") {
            while (!opStack.empty() && opStack.top().value != "(") {
                output.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty()) opStack.pop(); // Remove '('
            if (!opStack.empty() && opStack.top().type == FUNCTION) {
                output.push_back(opStack.top());
                opStack.pop();
            }
        }
    }
    while (!opStack.empty()) {
        output.push_back(opStack.top());
        opStack.pop();
    }
    return output;
}

double evaluatePostfix(const vector<Token>& postfix) {
    stack<double> evalStack;
    for (const auto& token : postfix) {
        if (token.type == NUMBER) {
            evalStack.push(stod(token.value));
        } else if (token.type == VARIABLE) {
            if (variables.find(token.value) != variables.end()) {
                evalStack.push(variables[token.value]);
            } else {
                throw runtime_error("Undefined variable: " + token.value);
            }
        } else if (token.type == OPERATOR) {
            if (evalStack.size() < 2) throw runtime_error("Insufficient operands for operator");
            double b = evalStack.top(); evalStack.pop();
            double a = evalStack.top(); evalStack.pop();
            if (token.value == "+") evalStack.push(a + b);
            else if (token.value == "-") evalStack.push(a - b);
            else if (token.value == "*") evalStack.push(a * b);
            else if (token.value == "/") evalStack.push(a / b);
            else if (token.value == "^") evalStack.push(pow(a, b));
        } else if (token.type == FUNCTION) {
            if (evalStack.empty()) throw runtime_error("Insufficient arguments for function");
            double arg = evalStack.top(); evalStack.pop();
            evalStack.push(functions[token.value]({arg}));
        }
    }
    if (evalStack.size() != 1) throw runtime_error("Invalid expression evaluation");
    return evalStack.top();
}

void defineCustomFunction(const string& name, function<double(vector<double>)> func) {
    functions[name] = func;
}

string optimizeExpression(const string& expr) {
    string optimized = expr;
    size_t pos = 0;
    while ((pos = optimized.find("--", pos)) != string::npos) {
        optimized.replace(pos, 2, "+");
    }
    return optimized;
}

void visualizeEvaluation(const vector<Token>& postfix) {
    cout << "Postfix Expression: ";
    for (const auto& token : postfix) {
        cout << token.value << " ";
    }
    cout << endl;
}

int main() {
    // Define custom function: cube(x) = x^3
    defineCustomFunction("cube", [](vector<double> args) {
        return pow(args[0], 3);
    });

    double xValue;
    cout << "Enter value for x (if any): ";
    cin >> xValue;
    variables["x"] = xValue;

    double yValue;
    cout << "Enter value for y (if any): ";
    cin >> yValue;
    variables["y"] = yValue;

    cin.ignore(); // Clear newline after reading y

    string expression;
    cout << "\nTip: Use parentheses for fractional powers like: 16^(1/2),\n"
         << "and include '*' for multiplication (e.g., 2*x not 2x)\n";
    cout << "Enter an expression: ";
    getline(cin, expression);

    try {
        string simplified = optimizeExpression(expression);
        auto tokens = tokenize(simplified);
        auto postfix = infixToPostfix(tokens);
        visualizeEvaluation(postfix);
        double result = evaluatePostfix(postfix);
        cout << fixed << setprecision(6) << "Result: " << result << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
