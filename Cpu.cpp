#include "Cpu.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>
#include <limits>

Cpu::Cpu() 
    : instSize(0)
    , smthWentWrong(false)
{
    registers["AYB"] = 0; // accumulator
    registers["BEN"] = 0;
    registers["GIM"] = 0;
    registers["DA"] = 0; // for CMP
    registers["ECH"] = 0;
    registers["ZA"] = 0;
    registers["GH"] = 0; // analogue of EIP

    // Initialize memory to zero
    memory.resize(memorySize, "0");
}

void Cpu::load(const std::string& file)
{
    memory.resize(memorySize, "0");
    std::string instruction;
    std::ifstream fin;
    fin.open(file);
    if (!fin.is_open()) {
        std::cerr << "ERROR while opening file\n";
        smthWentWrong = true;
        return;
    }
    while (std::getline(fin, instruction)) {
        memory.insert(memory.begin() + instSize, instruction);
        ++instSize;
    }
    if (instSize > memorySize) {
        std::cerr << "Instructions exceed program memory\n";
        smthWentWrong = true;
        return;
    }
    fin.close();
}

int Cpu::findLabelAddress(const std::string& label)
{
    std::string label1 = label + ":";
    for (int i = 0; i < instSize; ++i) {
        std::string operation;
        std::istringstream iss(memory[i]);
        iss >> operation;
        if (operation == label1) {
            return i;
        }
    }
    return -1; // if not found
}

// Please in input file, in instructions with 2 operands, write commas (,) after whitespase
// E.g. MOV BEN , 24

void Cpu::execute(const std::string& file)
{
    clear();
    load(file); // fetch
    
    //decode, execute
    while (registers["GH"] < instSize) {
        std::string operation;
        std::istringstream iss(memory[registers["GH"]]);
        iss >> operation;

        if (operation == "MOV") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    registers[op1] = registers[op2];
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    registers[op1] = std::stoi(memory[memAddress]);
                }
                else {
                    // op2 is an immediate value
                    registers[op1] = std::stoi(op2);
                }
            }
            else {
                // op1 is a memory address
                int memAddress = std::stoi(op1.substr(1, op1.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    memory.insert(memory.begin() + memAddress, std::to_string(registers[op2]));
                }
                else {
                    // op2 is an immediate value
                    memory.insert(memory.begin() + memAddress, op2);
                }
            }
        }

        else if (operation == "ADD") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    registers[op1] += registers[op2];
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    } 
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    registers[op1] += std::stoi(memory[memAddress]);
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else {
                    // op2 is an immediate value
                    registers[op1] += std::stoi(op2);
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
            }
            else {
                // op1 is a memory address
                int memAddress = std::stoi(op1.substr(1, op1.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    int sum = std::stoi(memory.at(memAddress)) + registers[op2];
                    memory.insert(memory.begin() + memAddress, std::to_string(sum));
                    //overflow case
                    if (sum > std::numeric_limits<int>::max() || sum < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else {
                    // op2 is an immediate value
                    int sum = std::stoi(memory.at(memAddress)) + std::stoi(op2);
                    memory.insert(memory.begin() + memAddress, std::to_string(sum));
                    //overflow case
                    if (sum > std::numeric_limits<int>::max() || sum < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
            }
        }

        else if (operation == "SUB") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    registers[op1] -= registers[op2];
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    registers[op1] -= std::stoi(memory[memAddress]);
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else {
                    // op2 is an immediate value
                    registers[op1] -= std::stoi(op2);
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
            }
            else {
                // op1 is a memory address
                int memAddress = std::stoi(op1.substr(1, op1.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    int sub = std::stoi(memory.at(memAddress)) - registers[op2];
                    memory.insert(memory.begin() + memAddress, std::to_string(sub));
                    //overflow case
                    if (sub > std::numeric_limits<int>::max() || sub < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else {
                    // op2 is an immediate value
                    int sub = std::stoi(memory.at(memAddress)) - std::stoi(op2);
                    memory.insert(memory.begin() + memAddress, std::to_string(sub));
                    //overflow case
                    if (sub > std::numeric_limits<int>::max() || sub < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
            }
        }

        else if (operation == "MUL") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    registers[op1] *= registers[op2];
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    registers[op1] *= std::stoi(memory[memAddress]);
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
                else {
                    // op2 is an immediate value
                    registers[op1] *= std::stoi(op2);
                    //overflow case
                    if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                        registers["ZA"] = 1;
                    }
                }
            }
        }

        else if (operation == "DIV") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    if (registers[op2] != 0) {
                        registers[op1] /= registers[op2];
                        //overflow case
                        if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                            registers["ZA"] = 1;
                        }
                    }
                    else {
                        std::cerr << "Can't divide by zero\n";
                        smthWentWrong = true;
                        return;
                    }
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    if (std::stoi(memory[memAddress]) != 0) {
                        registers[op1] /= std::stoi(memory[memAddress]);
                        //overflow case
                        if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                            registers["ZA"] = 1;
                        }
                    }
                    else {
                        std::cerr << "Can't divide by zero\n";
                        smthWentWrong = true;
                        return;
                    }
                }
                else {
                    // op2 is an immediate value
                    if (std::stoi(op2) != 0) {
                        registers[op1] /= std::stoi(op2);
                        //overflow case
                        if (registers[op1] > std::numeric_limits<int>::max() || registers[op1] < std::numeric_limits<int>::min()) {
                            registers["ZA"] = 1;
                        }
                    }
                    else {
                        std::cerr << "Can't divide by zero\n";
                        smthWentWrong = true;
                        return;
                    }
                }
            }
        }

        else if (operation == "AND") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            std::bitset<sizeof(int) * 8> bits1;
            std::bitset<sizeof(int) * 8> bits2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    bits1 = registers[op1];
                    bits2 = registers[op2];
                    registers[op1] = (int)(bits1 & bits2).to_ulong();
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    bits1 = registers[op1];
                    bits2 = std::stoi(memory[memAddress]);;
                    registers[op1] = (int)(bits1 & bits2).to_ulong();
                }
                else {
                    // op2 is an immediate value
                    bits1 = registers[op1];
                    bits2 = std::stoi(op2);
                    registers[op1] = (int)(bits1 & bits2).to_ulong();
                }
            }
            else {
                // op1 is a memory address
                int memAddress = std::stoi(op1.substr(1, op1.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    bits1 = std::stoi(memory[memAddress]);
                    bits2 = registers[op2];
                    memory.insert(memory.begin() + memAddress, (bits1& bits2).to_string());
                }
                else {
                    // op2 is an immediate value
                    bits1 = std::stoi(memory[memAddress]);
                    bits2 = std::stoi(op2);
                    memory.insert(memory.begin() + memAddress, (bits1& bits2).to_string());
                }
            }
        }

        else if (operation == "OR") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            std::bitset<sizeof(int) * 8> bits1;
            std::bitset<sizeof(int) * 8> bits2;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    bits1 = registers[op1];
                    bits2 = registers[op2];
                    registers[op1] = (int)(bits1 | bits2).to_ulong();
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    bits1 = registers[op1];
                    bits2 = std::stoi(memory[memAddress]);
                    registers[op1] = (int)(bits1 | bits2).to_ulong();
                }
                else {
                    // op2 is an immediate value
                    bits1 = registers[op1];
                    bits2 = std::stoi(op2);
                    registers[op1] = (int)(bits1 | bits2).to_ulong();
                }
            }
            else {
                // op1 is a memory address
                int memAddress = std::stoi(op1.substr(1, op1.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    bits1 = std::stoi(memory[memAddress]);
                    bits2 = registers[op2];
                    memory.insert(memory.begin() + memAddress, (bits1 | bits2).to_string());
                }
                else {
                    // op2 is an immediate value
                    bits1 = std::stoi(memory[memAddress]);
                    bits2 = std::stoi(op2);
                    memory.insert(memory.begin() + memAddress, (bits1 | bits2).to_string());
                }
            }
        }

        else if (operation == "NOT") {
            std::string op;
            iss >> op;
            std::bitset<sizeof(int) * 8> bits;
            if (registers.find(op) != registers.end()) {
                // op is a register
                bits = registers[op];
                registers[op] = (int)(~bits).to_ulong();
            }
            else {
                // op is a memory address
                int memAddress = std::stoi(op.substr(1, op.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                bits = std::stoi(memory[memAddress]);
                memory.insert(memory.begin() + memAddress, (~bits).to_string());
            }
        }

        else if (operation == "CMP") {
            std::string op1;
            std::string op2;
            std::string comma;
            iss >> op1 >> comma >> op2;

            int value1 = 0;
            int value2 = 0;

            if (registers.find(op1) != registers.end()) {
                // op1 is a register
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    value1 = registers[op1];
                    value2 = registers[op2];
                }
                else if (op2.size() >= 3 && op2.size() <= 4 && op2[0] == '[' && op2[op2.size() - 1] == ']') {
                    // op2 is a memory address
                    int memAddress = std::stoi(op2.substr(1, op2.size() - 2));
                    if (memAddress < instSize) {
                        std::cerr << "The memory address you try to use is occupied by instructions\n";
                        smthWentWrong = true;
                        return;
                    }
                    else if (memAddress >= memorySize) {
                        std::cerr << "The memory exceeds program memory\n";
                        smthWentWrong = true;
                        return;
                    }
                    value1 = registers[op1];
                    value2 = std::stoi(memory[memAddress]);
                }
                else {
                    // op2 is an immediate value
                    value1 = registers[op1];
                    value2 = std::stoi(op2);
                }
            }
            else {
                // op1 is a memory address
                int memAddress = std::stoi(op1.substr(1, op1.size() - 2));
                if (memAddress < instSize) {
                    std::cerr << "The memory address you try to use is occupied by instructions\n";
                    smthWentWrong = true;
                    return;
                }
                else if (memAddress >= memorySize) {
                    std::cerr << "The memory exceeds program memory\n";
                    smthWentWrong = true;
                    return;
                }
                if (registers.find(op2) != registers.end()) {
                    // op2 is a register
                    value1 = std::stoi(memory[memAddress]);
                    value2 = registers[op2];
                }
                else {
                    // op2 is an immediate value
                    value1 = std::stoi(memory[memAddress]);
                    value2 = std::stoi(op2);
                }
            }
            int result = value1 - value2;
            if (result < 0) {
                registers["DA"] = -1; // if op1 is less than op2, register Da is set to -1
            }
            else if (result > 0) {
                registers["DA"] = 1; // if op1 is greater than op2, register Da is set to 1
            }
            else {
                registers["DA"] = 0; // if op1 is equal to op2, register Da is set to 1
            }
        }

        else if (operation == "JMP") {
            std::string label;
            iss >> label;
            int address = findLabelAddress(label);
            if (address == -1) {
                std::cerr << "Label not found\n";
                smthWentWrong = true;
                return;
            }
            else {
                labels[address] = label;
                registers["GH"] = address;
                std::size_t firstSpace = memory[registers["GH"]].find(' ');
                if (firstSpace != std::string::npos) {
                    memory[registers["GH"]] = memory[registers["GH"]].substr(firstSpace + 1);
                    continue;
                }
                else {
                    smthWentWrong = true;
                    return;
                }
            }
        }

        else if (operation == "JG") {
            std::string label;
            iss >> label;
            int address = findLabelAddress(label);
            if (address == -1) {
                std::cerr << "Label not found\n";
                smthWentWrong = true;
                return;
            }
            else {
                labels[address] = label;
                std::size_t firstSpace = memory[address].find(' ');
                if (firstSpace != std::string::npos) {
                    memory[address] = memory[address].substr(firstSpace + 1);
                    if (registers["DA"] == 1) {
                        registers["GH"] = address;
                        continue;
                    }
                }
                else {
                    smthWentWrong = true;
                    return;
                }
            }
        }

        else if (operation == "JL") {
            std::string label;
            iss >> label;
            int address = findLabelAddress(label);
            if (address == -1) {
                std::cerr << "Label not found\n";
                smthWentWrong = true;
                return;
            }
            else {
                labels[address] = label;
                std::size_t firstSpace = memory[address].find(' ');
                if (firstSpace != std::string::npos) {
                    memory[address] = memory[address].substr(firstSpace + 1);
                    if (registers["DA"] == -1) {
                        registers["GH"] = address;
                        continue;
                    }
                }
                else {
                    smthWentWrong = true;
                    return;
                }
            }
        }

        else if (operation == "JE") {
            std::string label;
            iss >> label;
            int address = findLabelAddress(label);
            if (address == -1) {
                std::cerr << "Label not found\n";
                smthWentWrong = true;
                return;
            }
            else {
                labels[address] = label;
                std::size_t firstSpace = memory[address].find(' ');
                if (firstSpace != std::string::npos) {
                    memory[address] = memory[address].substr(firstSpace + 1);
                    if (registers["DA"] == 0) {
                        registers["GH"] = address;
                        continue;
                    }
                }
                else {
                    smthWentWrong = true;
                    return;
                }
            }
        }

        else {
            std::cerr << "Incorrect instruction provided\n";
            smthWentWrong = true;
            return;
        }

        registers["GH"]++;
    }
}

void Cpu::clear() 
{
    registers["GH"] = 0;
    for (auto& reg : registers) {
        if (reg.first != "GH") {
            reg.second = 0;
        }
    }
    instSize = 0;
    smthWentWrong = false;
}

void Cpu::dump_memory() const
{
    if (smthWentWrong == true) {
        return;
    }
    std::cout << "Memory:\n";
    for (std::size_t i = 0; i < memorySize; ++i) {
        auto labelIt = labels.find(i);
        if (labelIt != labels.end()) {
            std::cout << "[" << i << "] : ";
            std::cout << labelIt->second << ": " << memory[i] << std::endl;
        }
        else {
            std::cout << "[" << i << "] : " << memory[i] << std::endl;
        }
    }
}