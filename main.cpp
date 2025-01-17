#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <map>
#include <random>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <sstream>
// thank you chatgpt
// future me here, chatgpt did not help so idk, use this for now
int genrandomnum(int min, int max) {
    std::random_device rd;
    return rd() % max + min;
}
std::vector<std::string> split(std::string str, char ch) {
    std::vector<std::string> vTemp;
    std::string sTemp;
    for (int i = 0; i < str.size(); ++i) {
        if (str[i] != ch) {
            sTemp += str[i];
        } else {
            vTemp.push_back(sTemp);
            sTemp = "";
        }
    }
    if (sTemp != "") {
        vTemp.push_back(sTemp);
    }
    return vTemp;
}
std::string getinside(std::string str, char ch1, char ch2) {
    size_t x = str.find(ch1);
    size_t y = str.find(ch2);
    return str.substr(x + 1,y - x - 1);
}
std::string getbehind(std::string str, char ch) {
    return str.substr(0, str.find(ch));
}
std::string getinside2(std::string str, char ch) {
    size_t first = str.find(ch);
    size_t second = str.find(ch, first + 1);
    return str.substr(first + 1, second - first - 1);
}
template<typename T>
std::string getforward(std::string str, T ch, int offset = 0) {
    size_t somewhere = str.find(ch);
    if (somewhere != std::string::npos) {
        return str.substr(somewhere + 1 + offset, str.size());
    }
}
struct vars {
    std::string type;
    std::variant<int, float, std::string> value;
};
void printvar(vars var) {
    if (var.type == "string") {
        std::cout << std::get<std::string>(var.value) << std::endl;
    } else if (var.type == "float") {
        std::cout << std::get<float>(var.value) << std::endl;
    } else if (var.type == "int") {
        std::cout << std::get<int>(var.value) << std::endl;
    }
}
vars randnum(int min, int max) {
    vars var;
    var.type = "int";
    var.value = genrandomnum(min, max);
    return var;
}
std::map<std::string, vars> mVars;
std::string readfile(std::string str) {
    std::fstream file;
    file.open(str);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContents = buffer.str();
    return fileContents;
}
struct Function {
    std::vector<std::string> code;
    std::vector<std::string> vars;
};
std::map<std::string, Function> funcs;
std::string tokenize(std::string str) {
    if (str.find('(') != std::string::npos && str.find('=') == std::string::npos && str.find("if") == std::string::npos && str.find("for") == std::string::npos && str.find("func") == std::string::npos && funcs.contains(getbehind(str, ')')) == false) {
       if (funcs.contains(getbehind(str, '(')) == false) {
           return "CALL\t" + getbehind(str, '(') + "\t" + getinside(str, '(', ')');
       } else {
           return "CALLFUNC\t" + getbehind(str, '(') + "\t" + getinside(str, '(', ')');
       }
    } else if (str.find("var") != std::string::npos && str.find("if") == std::string::npos && str.find("for") == std::string::npos) {
        // var somevar = 5;
        //     _______ trying to get this
        std::string actualname = str.substr(str.find("var") + 4, str.find('=') - str.find("var") - 5);
        std::string val = getforward(str, '=', 1);
        return "VAR\t" + actualname + "\t" + val;
    } else if (str.find("if") != std::string::npos) {
        if (str.find("if(") != std::string::npos) {
            return "IF\t" + getinside(str, '(', ')') + "\t" + getinside(str, '{', '}');
        }
    } else if (str.find("for") != std::string::npos) {
        return "FOR\t" + getinside(str, '(', ')') + "\t" + str.substr(str.find('{') + 1, str.size() - str.find('{') - 2);
    } else if (str.find("func") != std::string::npos) {
        return "FUNCTION\t" + str.substr(str.find("func") + 5, str.find('{') - 5) + "\t" + str.substr(str.find('{') + 1, str.size() - str.find('{') - 2);
    }
}
// here comes hell
std::string findvartype(std::string thing) {
    if (thing.find('.') != std::string::npos) {
        return "float";
    } else if (thing.find('"') != std::string::npos) {
        return "string";
    } else {
        return "int";
    }
    return "ok wtf?"; // trust me on this one bro
}
void run(std::string str) {
    std::cout << str << std::endl;
    std::vector<std::string> tokens = split(str, '\t');
    if (tokens[0] == "CALL") {
        if (tokens[1] == "print") {
            if (tokens[2].find('"') != std::string::npos) {
                std::cout << getinside2(str, '"') << std::endl;
            } else {
                if (mVars.contains(tokens[2])) {
                    if (mVars[tokens[2]].type == "string") {
                        std::cout << std::get<std::string>(mVars[tokens[2]].value) << std::endl;
                    } else if (mVars[tokens[2]].type == "float") {
                        std::cout << std::get<float>(mVars[tokens[2]].value) << std::endl;
                    } else {
                        std::cout << std::get<int>(mVars[tokens[2]].value) << std::endl;
                    }
                }
            }
        } else if (tokens[1] == "runfile") {
            std::vector<std::string> args = split(tokens[2], ',');
            if (args.size() >= 1) {
                std::string file = readfile(getinside2(args[0], '"'));
                // getinside2(std::string str, char ch)
                std::vector<std::string> lines = split(file, '\n');
                for (const auto line : lines) {
                    run(tokenize(line));
                }
            }
        } else if (tokens[1] == "--") {
            std::vector<std::string> args = split(tokens[2], ',');
            if (args.size() >= 2) {
                if (mVars.contains(args[0])) {
                    auto it = mVars.find(args[0]);
                    if (mVars.contains(args[1])) {
                        auto it_1 = mVars.find(args[1]);
                        if (it->second.type == "float") {
                            if (it_1->second.type == "int") {
                                float z = std::get<float>(it->second.value);
                                z -= static_cast<float>(std::get<int>(it_1->second.value));
                                it->second.value = z;
                            } else if (it_1->second.type == "float") {
                                float z = std::get<float>(it->second.value);
                                z -= std::get<float>(it_1->second.value);
                                it->second.value = z;
                            }
                        } else if (it->second.type == "int") {
                            if (it_1->second.type == "int") {
                                int z = std::get<int>(it->second.value);
                                z -= std::get<int>(it_1->second.value);
                                it->second.value = z;
                            } else if (it_1->second.type == "float") {
                                std::cout << "ERROR, CANT ADD FLOAT TO INT!, DECLARE THE VARIABLE WITH '.' eg: var x = 1.0" << std::endl;
                            }
                        }
                    } else {
                        if (it->second.type == "int") {
                            int z = std::get<int>(it->second.value);
                            z -= std::stoi(args[1]);
                            it->second.value = z;
                        } else if (it->second.type == "float") {
                            float z = std::get<float>(it->second.value);
                            z -= std::stof(args[1]);
                            it->second.value = z;
                        }
                    }
                }
            }
        } else if (tokens[1] == "**") {
            std::vector<std::string> args = split(tokens[2], ',');
            if (args.size() >= 2) {
                if (mVars.contains(args[0])) {
                    auto it = mVars.find(args[0]);
                    if (mVars.contains(args[1])) {
                        auto it_1 = mVars.find(args[1]);
                        if (it->second.type == "float") {
                            if (it_1->second.type == "int") {
                                float z = std::get<float>(it->second.value);
                                z = z * static_cast<float>(std::get<int>(it_1->second.value));
                                it->second.value = z;
                            } else if (it_1->second.type == "float") {
                                float z = std::get<float>(it->second.value);
                                z = z * std::get<float>(it_1->second.value);
                                it->second.value = z;
                            }
                        } else if (it->second.type == "int") {
                            if (it_1->second.type == "int") {
                                int z = std::get<int>(it->second.value);
                                z = z * std::get<int>(it_1->second.value);
                                it->second.value = z;
                            } else if (it_1->second.type == "float") {
                                std::cout << "ERROR, CANT ADD FLOAT TO INT!, DECLARE THE VARIABLE WITH '.' eg: var x = 1.0" << std::endl;
                            }
                        }
                    } else {
                        if (it->second.type == "int") {
                            int z = std::get<int>(it->second.value);
                            z = z * std::stoi(args[1]);
                            it->second.value = z;
                        } else if (it->second.type == "float") {
                            float z = std::get<float>(it->second.value);
                            z = z * std::stof(args[1]);
                            it->second.value = z;
                        }
                    }
                }
            }
        } else if (tokens[1] == "++") {
            std::vector<std::string> args = split(tokens[2], ',');
            if (args.size() >= 2) {
                if (mVars.contains(args[0])) {
                    auto it = mVars.find(args[0]);
                    if (mVars.contains(args[1])) {
                        auto it_1 = mVars.find(args[1]);
                        if (it->second.type == "float") {
                            if (it_1->second.type == "int") {
                                float z = std::get<float>(it->second.value);
                                z += static_cast<float>(std::get<int>(it_1->second.value));
                                it->second.value = z;
                            } else if (it_1->second.type == "float") {
                                float z = std::get<float>(it->second.value);
                                z += std::get<float>(it_1->second.value);
                                it->second.value = z;
                            }
                        } else if (it->second.type == "int") {
                            if (it_1->second.type == "int") {
                                int z = std::get<int>(it->second.value);
                                z += std::get<int>(it_1->second.value);
                                it->second.value = z;
                            } else if (it_1->second.type == "float") {
                                std::cout << "ERROR, CANT ADD FLOAT TO INT!, DECLARE THE VARIABLE WITH '.' eg: var x = 1.0" << std::endl;
                            }
                        }
                    } else {
                        if (it->second.type == "int") {
                            int z = std::get<int>(it->second.value);
                            z += std::stoi(args[1]);
                            it->second.value = z;
                        } else if (it->second.type == "float") {
                            float z = std::get<float>(it->second.value);
                            z += std::stof(args[1]);
                            it->second.value = z;
                        }
                    }
                }
            }
        }
    } else if (tokens[0] == "CALLFUNC") {
        if (funcs.contains(tokens[1])) {
            auto it = funcs.find(tokens[1]);
            std::vector<std::string> UserArgs = split(tokens[2], ',');
            if (UserArgs.size() == it->second.vars.size()) {
                for (int i = 0; i < UserArgs.size(); ++i) {
                    std::string name = "var " + it->second.vars[i] + " = " + UserArgs[i];
                    run(tokenize(name));
                }
            }
            for (const auto code : it->second.code) {
                run(tokenize(code));
            }
        }
    } else if (tokens[0] == "IF") {
        std::vector<std::string> expression = split(tokens[1], ' ');
        if (expression.size() >= 3) {
            std::string one;
            std::string two;
            std::string exp;
            if (mVars.contains(expression[0])) {
                if (mVars[expression[0]].type == "float") {
                    one = std::to_string(std::get<float>(mVars[expression[0]].value));
                } else if (mVars[expression[0]].type == "int") {
                    one = std::to_string(std::get<int>(mVars[expression[0]].value));
                } else if (mVars[expression[0]].type == "string") {
                    one = std::get<std::string>(mVars[expression[0]].value);;
                }
            } else {
                one = expression[0];
            }
            if (mVars.contains(expression[2])) {
                if (mVars[expression[2]].type == "float") {
                    two = std::to_string(std::get<float>(mVars[expression[2]].value));
                } else if (mVars[expression[2]].type == "int") {
                    two = std::to_string(std::get<int>(mVars[expression[2]].value));
                } else if (mVars[expression[2]].type == "string") {
                    two = std::get<std::string>(mVars[expression[2]].value);;
                }
            } else {
                two = expression[2];
            }
            exp = expression[1];
            if (exp == "==") {
                if (one == two) {
                    if (tokens.size() >= 3) {
                        std::vector<std::string> torun = split(tokens[2], ';');
                        for (size_t i = 0; i < torun.size(); ++i) {
                            run(tokenize(torun[i]));
                        }
                    }
                }
            } else if (exp == "!=") {
                if (one != two) {
                    if (tokens.size() >= 3) {
                        std::vector<std::string> torun = split(tokens[2], ';');
                        for (size_t i = 0; i < torun.size(); ++i) {
                            run(tokenize(torun[i]));
                        }
                    }
                }
            } else if (exp == ">") {
                try {
                    int one1 = std::stoi(one);
                    int two1 = std::stoi(two);
                    if (one1 > two1) {
                        if (tokens.size() >= 3) {
                            std::vector<std::string> torun = split(tokens[2], ';');
                            for (size_t i = 0; i < torun.size(); ++i) {
                                run(tokenize(torun[i]));
                            }
                        }
                    }
                } catch (...) {
                    std::cout << "Most likely you are trying to use either strings or the code just does not work lol" << std::endl;
                }
            }
        }
    } else if (tokens[0] == "FOR") {
        int num;
        if (mVars.contains(tokens[1])) {
            auto it = mVars.find(tokens[1]);
            if (it->second.type == "int") {
                num = std::get<int>(it->second.value);
            }
        } else {
            num = std::stoi(tokens[1]);
        }
        std::vector<std::string> torun = split(tokens[2], ';');
        for (int i = 0; i < num;++i) {
            for (const auto code : torun) {
                run(tokenize(code));
            }
        }
    } else if (tokens[0] == "FUNCTION") {
        std::string functionName = getbehind(tokens[1], '(');
        std::vector<std::string> args = split(getinside(tokens[1], '(', ')'), ',');
        std::cout << functionName << std::endl;
        Function func;
        func.vars = args;
        //func.args
        func.code = split(tokens[2], ';');
        if (funcs.contains(functionName)) {
            funcs.erase(functionName);
        }
        funcs.insert(std::make_pair(functionName, func));
    } else if (tokens[0] == "VAR") {
        if (str.find("(") == std::string::npos) {
            vars var;
            if (tokens[2].find('.') != std::string::npos) {
                var.type = "float";
            } else if (tokens[2].find('"') != std::string::npos) {
                var.type = "string";
            } else {
                var.type = "int";
            }
            if (var.type == "string") {
                var.value = getinside2(tokens[2], '"');
            } else if (var.type == "float") {
                var.value = std::stof(tokens[2]);
            } else {
                var.value = std::stoi(tokens[2]);
            }
            if (mVars.contains(tokens[1])) {
                mVars.erase(tokens[1]);
            }
            mVars.insert(std::make_pair(tokens[1], var));
        } else {
            if (tokens[2].find('(') != std::string::npos) {
                std::string functionname = getbehind(tokens[2], '(');
                if (functionname == "input") {
                    std::string args = getinside(tokens[2], '(', ')');
                    if (args.find(',') != std::string::npos) {
                        std::cout << "why the hell are you using ',' in a function where its one argument?" << std::endl;
                    } else {
                        if (args.find('"') != std::string::npos) {
                            std::cout << getinside2(args, '"');
                        } else {
                            if (mVars.contains(args)) {
                                printvar(mVars[args]);
                            }
                        }
                        std::string value;
                        std::getline(std::cin, value);
                        vars var;
                        var.type = "string";
                        var.value = value;
                        mVars.insert(std::make_pair(tokens[1], var));
                    }
                } else if (functionname == "stoi") {
                    std::string args = getinside(tokens[2], '(', ')');
                    if (args.find(',') != std::string::npos) {
                        std::cout << "why the hell are you using ',' in a function where its one argument?" << std::endl;
                    } else {
                        if (mVars.contains(args)) {
                            auto it = mVars.find(args);
                            if (it->second.type == "string") {
                                try {
                                    vars var;
                                    var.type = "int";
                                    std::cout << it->second.type << std::endl;
                                    std::cout << std::get<std::string>(it->second.value) << std::endl;
                                    var.value = std::stoi(std::get<std::string>(it->second.value));
                                    mVars.insert(std::make_pair(tokens[1], var));
                                } catch (...) {
                                    std::cout << "something went wrong!" << std::endl;
                                }
                            }
                        }
                    }
                } else if (functionname == "rand") {
                    std::string args = getinside(tokens[2], '(', ')');
                    std::vector<std::string> vArgs = split(args, ',');
                    if (vArgs.size() >= 2) {
                        int min = std::stoi(vArgs[0]);
                        int max = std::stoi(vArgs[1]);
                        vars var = randnum(min, max);
                        mVars.insert(std::make_pair(tokens[1], var));
                    }
                }
            }
        }
    }
}
int main() {
    srand(static_cast<unsigned int>(std::time(NULL)));
    while (true) {
        std::cout << "~>";
        std::string cmd;
        std::getline(std::cin, cmd);
        run(tokenize(cmd));
    }
}
