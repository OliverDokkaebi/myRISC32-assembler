#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include <cctype>
#include <cstdint>

// Instruction formats as per myRV32I specification
enum class InstructionFormat {
    R_TYPE,  // register-register
    I_TYPE,  // immediate
    S_TYPE,  // store
    B_TYPE,  // branch
    U_TYPE,  // upper immediate
    J_TYPE   // jump
};

// Instruction structure to store details about each instruction
struct Instruction {
    std::string name;
    InstructionFormat format;
    uint32_t opcode;
    uint32_t funct3;
    uint32_t funct7;
};

// Register structure to map names to numbers
struct Register {
    std::string name;
    int number;
};

// Symbol structure for labels
struct Symbol {
    std::string name;
    uint32_t address;
};

// Function to trim whitespace from start and end of a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Function to check if a string is a number
bool isNumber(const std::string& str) {
    if (str.empty()) return false;
    
    // Check for hexadecimal number
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        for (size_t i = 2; i < str.size(); i++) {
            if (!isxdigit(str[i])) return false;
        }
        return true;
    }
    
    // Check for decimal number (allow negative numbers)
    size_t start = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    for (size_t i = start; i < str.size(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

// Function to parse a number from string
int parseNumber(const std::string& str) {
    // For hexadecimal numbers
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return std::stoi(str, nullptr, 16);
    }
    // For decimal numbers
    return std::stoi(str);
}

// Function to split string by delimiter
std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delim)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

// Function to populate instruction map
std::unordered_map<std::string, Instruction> createInstructionMap() {
    std::unordered_map<std::string, Instruction> instructions;
    
    // R-type instructions
    instructions["add"] = {"add", InstructionFormat::R_TYPE, 0b0110011, 0b000, 0b0000000};
    instructions["sub"] = {"sub", InstructionFormat::R_TYPE, 0b0110011, 0b000, 0b0100000};
    instructions["sll"] = {"sll", InstructionFormat::R_TYPE, 0b0110011, 0b001, 0b0000000};
    instructions["slt"] = {"slt", InstructionFormat::R_TYPE, 0b0110011, 0b010, 0b0000000};
    instructions["sltu"] = {"sltu", InstructionFormat::R_TYPE, 0b0110011, 0b011, 0b0000000};
    instructions["xor"] = {"xor", InstructionFormat::R_TYPE, 0b0110011, 0b100, 0b0000000};
    instructions["srl"] = {"srl", InstructionFormat::R_TYPE, 0b0110011, 0b101, 0b0000000};
    instructions["sra"] = {"sra", InstructionFormat::R_TYPE, 0b0110011, 0b101, 0b0100000};
    instructions["or"] = {"or", InstructionFormat::R_TYPE, 0b0110011, 0b110, 0b0000000};
    instructions["and"] = {"and", InstructionFormat::R_TYPE, 0b0110011, 0b111, 0b0000000};
    
    // I-type instructions
    instructions["addi"] = {"addi", InstructionFormat::I_TYPE, 0b0010011, 0b000, 0};
    instructions["slti"] = {"slti", InstructionFormat::I_TYPE, 0b0010011, 0b010, 0};
    instructions["sltiu"] = {"sltiu", InstructionFormat::I_TYPE, 0b0010011, 0b011, 0};
    instructions["xori"] = {"xori", InstructionFormat::I_TYPE, 0b0010011, 0b100, 0};
    instructions["ori"] = {"ori", InstructionFormat::I_TYPE, 0b0010011, 0b110, 0};
    instructions["andi"] = {"andi", InstructionFormat::I_TYPE, 0b0010011, 0b111, 0};
    instructions["slli"] = {"slli", InstructionFormat::I_TYPE, 0b0010011, 0b001, 0};
    instructions["srli"] = {"srli", InstructionFormat::I_TYPE, 0b0010011, 0b101, 0};
    instructions["srai"] = {"srai", InstructionFormat::I_TYPE, 0b0010011, 0b101, 0b0100000};
    
    // Load instructions (I-type)
    instructions["lb"] = {"lb", InstructionFormat::I_TYPE, 0b0000011, 0b000, 0};
    instructions["lh"] = {"lh", InstructionFormat::I_TYPE, 0b0000011, 0b001, 0};
    instructions["lw"] = {"lw", InstructionFormat::I_TYPE, 0b0000011, 0b010, 0};
    instructions["lbu"] = {"lbu", InstructionFormat::I_TYPE, 0b0000011, 0b100, 0};
    instructions["lhu"] = {"lhu", InstructionFormat::I_TYPE, 0b0000011, 0b101, 0};
    
    // S-type instructions
    instructions["sb"] = {"sb", InstructionFormat::S_TYPE, 0b0100011, 0b000, 0};
    instructions["sh"] = {"sh", InstructionFormat::S_TYPE, 0b0100011, 0b001, 0};
    instructions["sw"] = {"sw", InstructionFormat::S_TYPE, 0b0100011, 0b010, 0};
    
    // B-type instructions
    instructions["beq"] = {"beq", InstructionFormat::B_TYPE, 0b1100011, 0b000, 0};
    instructions["bne"] = {"bne", InstructionFormat::B_TYPE, 0b1100011, 0b001, 0};
    instructions["blt"] = {"blt", InstructionFormat::B_TYPE, 0b1100011, 0b100, 0};
    instructions["bge"] = {"bge", InstructionFormat::B_TYPE, 0b1100011, 0b101, 0};
    instructions["bltu"] = {"bltu", InstructionFormat::B_TYPE, 0b1100011, 0b110, 0};
    instructions["bgeu"] = {"bgeu", InstructionFormat::B_TYPE, 0b1100011, 0b111, 0};
    
    // U-type instructions
    instructions["lui"] = {"lui", InstructionFormat::U_TYPE, 0b0110111, 0, 0};
    instructions["auipc"] = {"auipc", InstructionFormat::U_TYPE, 0b0010111, 0, 0};
    
    // J-type instructions
    instructions["jal"] = {"jal", InstructionFormat::J_TYPE, 0b1101111, 0, 0};
    
    // JALR (I-type)
    instructions["jalr"] = {"jalr", InstructionFormat::I_TYPE, 0b1100111, 0b000, 0};
    
    return instructions;
}

// Function to populate register map
std::unordered_map<std::string, int> createRegisterMap() {
    std::unordered_map<std::string, int> registers;
    
    // Register names and their corresponding numbers
    registers["zero"] = 0;
    registers["ra"] = 1;
    registers["sp"] = 2;
    registers["gp"] = 3;
    registers["tp"] = 4;
    registers["t0"] = 5;
    registers["t1"] = 6;
    registers["t2"] = 7;
    registers["s0"] = 8;
    registers["fp"] = 8;  // s0 and fp are the same register
    registers["s1"] = 9;
    registers["a0"] = 10;
    registers["a1"] = 11;
    registers["a2"] = 12;
    registers["a3"] = 13;
    registers["a4"] = 14;
    registers["a5"] = 15;
    registers["a6"] = 16;
    registers["a7"] = 17;
    registers["s2"] = 18;
    registers["s3"] = 19;
    registers["s4"] = 20;
    registers["s5"] = 21;
    registers["s6"] = 22;
    registers["s7"] = 23;
    registers["s8"] = 24;
    registers["s9"] = 25;
    registers["s10"] = 26;
    registers["s11"] = 27;
    registers["t3"] = 28;
    registers["t4"] = 29;
    registers["t5"] = 30;
    registers["t6"] = 31;
    
    // Add x0-x31 notation
    for (int i = 0; i <= 31; i++) {
        registers["x" + std::to_string(i)] = i;
    }
    
    return registers;
}

// Function to encode R-type instruction
uint32_t encodeRType(const Instruction& instr, int rd, int rs1, int rs2) {
    uint32_t machineCode = 0;
    machineCode |= instr.opcode;                 // opcode at bits 0-6
    machineCode |= (static_cast<uint32_t>(rd) << 7);        // rd at bits 7-11
    machineCode |= (static_cast<uint32_t>(instr.funct3) << 12); // funct3 at bits 12-14
    machineCode |= (static_cast<uint32_t>(rs1) << 15);      // rs1 at bits 15-19
    machineCode |= (static_cast<uint32_t>(rs2) << 20);      // rs2 at bits 20-24
    machineCode |= (static_cast<uint32_t>(instr.funct7) << 25); // funct7 at bits 25-31
    return machineCode;
}

// Function to encode I-type instruction
uint32_t encodeIType(const Instruction& instr, int rd, int rs1, int imm) {
    uint32_t machineCode = 0;
    machineCode |= instr.opcode;                 // opcode at bits 0-6
    machineCode |= (static_cast<uint32_t>(rd) << 7);        // rd at bits 7-11
    machineCode |= (static_cast<uint32_t>(instr.funct3) << 12); // funct3 at bits 12-14
    machineCode |= (static_cast<uint32_t>(rs1) << 15);      // rs1 at bits 15-19
    machineCode |= ((static_cast<uint32_t>(imm) & 0xFFF) << 20); // imm at bits 20-31
    return machineCode;
}

// Function to encode S-type instruction
uint32_t encodeSType(const Instruction& instr, int rs1, int rs2, int imm) {
    uint32_t machineCode = 0;
    machineCode |= instr.opcode;                 // opcode at bits 0-6
    machineCode |= ((static_cast<uint32_t>(imm) & 0x1F) << 7);   // imm[4:0] at bits 7-11
    machineCode |= (static_cast<uint32_t>(instr.funct3) << 12); // funct3 at bits 12-14
    machineCode |= (static_cast<uint32_t>(rs1) << 15);      // rs1 at bits 15-19
    machineCode |= (static_cast<uint32_t>(rs2) << 20);      // rs2 at bits 20-24
    machineCode |= ((static_cast<uint32_t>(imm) & 0xFE0) << (25 - 5)); // imm[11:5] at bits 25-31
    return machineCode;
}

// Function to encode B-type instruction
uint32_t encodeBType(const Instruction& instr, int rs1, int rs2, int imm) {
    uint32_t machineCode = 0;
    machineCode |= instr.opcode;                       // opcode at bits 0-6
    machineCode |= ((static_cast<uint32_t>(imm) & 0x800) >> (11 - 7));   // imm[11] at bit 7
    machineCode |= ((static_cast<uint32_t>(imm) & 0x1E) << (8 - 1));     // imm[4:1] at bits 8-11
    machineCode |= (static_cast<uint32_t>(instr.funct3) << 12);       // funct3 at bits 12-14
    machineCode |= (static_cast<uint32_t>(rs1) << 15);            // rs1 at bits 15-19
    machineCode |= (static_cast<uint32_t>(rs2) << 20);            // rs2 at bits 20-24
    machineCode |= ((static_cast<uint32_t>(imm) & 0x7E0) << (25 - 5));   // imm[10:5] at bits 25-30
    machineCode |= ((static_cast<uint32_t>(imm) & 0x1000) << (31 - 12)); // imm[12] at bit 31
    return machineCode;
}

// Function to encode U-type instruction
uint32_t encodeUType(const Instruction& instr, int rd, int imm) {
    uint32_t machineCode = 0;
    machineCode |= instr.opcode;                 // opcode at bits 0-6
    machineCode |= (static_cast<uint32_t>(rd) << 7);        // rd at bits 7-11
    machineCode |= (static_cast<uint32_t>(imm) & 0xFFFFF000);    // imm[31:12] at bits 12-31
    return machineCode;
}

// Function to encode J-type instruction
uint32_t encodeJType(const Instruction& instr, int rd, int imm) {
    uint32_t machineCode = 0;
    machineCode |= instr.opcode;                       // opcode at bits 0-6
    machineCode |= (static_cast<uint32_t>(rd) << 7);              // rd at bits 7-11
    machineCode |= ((static_cast<uint32_t>(imm) & 0xFF000) << (12 - 12));  // imm[19:12] at bits 12-19
    machineCode |= ((static_cast<uint32_t>(imm) & 0x800) << (20 - 11));    // imm[11] at bit 20
    machineCode |= ((static_cast<uint32_t>(imm) & 0x7FE) << (21 - 1));     // imm[10:1] at bits 21-30
    machineCode |= ((static_cast<uint32_t>(imm) & 0x100000) << (31 - 20)); // imm[20] at bit 31
    return machineCode;
}

// Function to parse operands from a comma-separated list
std::vector<std::string> parseOperands(const std::string& operandsStr) {
    std::vector<std::string> operands;
    std::string operand;
    std::istringstream operandStream(operandsStr);
    
    while (std::getline(operandStream, operand, ',')) {
        operands.push_back(trim(operand));
    }
    
    return operands;
}

// Function to parse load/store instructions with offset(rs1) format
std::pair<int, int> parseMemoryOperand(const std::string& operand, const std::unordered_map<std::string, int>& registers) {
    size_t openParen = operand.find('(');
    size_t closeParen = operand.find(')');
    
    if (openParen == std::string::npos || closeParen == std::string::npos) {
        throw std::runtime_error("Invalid memory operand format: " + operand);
    }
    
    std::string offsetStr = trim(operand.substr(0, openParen));
    std::string regStr = trim(operand.substr(openParen + 1, closeParen - openParen - 1));
    
    int offset = isNumber(offsetStr) ? parseNumber(offsetStr) : 0;
    
    auto regIt = registers.find(regStr);
    if (regIt == registers.end()) {
        throw std::runtime_error("Unknown register: " + regStr);
    }
    
    return {offset, regIt->second};
}

// Parse and assemble a single instruction
uint32_t assembleInstruction(const std::string& instructionStr, 
                            const std::unordered_map<std::string, Instruction>& instructions,
                            const std::unordered_map<std::string, int>& registers,
                            const std::unordered_map<std::string, uint32_t>& symbolTable,
                            uint32_t currentAddress) {
    // Split instruction into opcode and operands
    size_t spacePos = instructionStr.find(' ');
    if (spacePos == std::string::npos) {
        throw std::runtime_error("Invalid instruction format: " + instructionStr);
    }
    
    std::string opcode = trim(instructionStr.substr(0, spacePos));
    std::transform(opcode.begin(), opcode.end(), opcode.begin(), ::tolower);
    
    std::string operandsStr = trim(instructionStr.substr(spacePos + 1));
    std::vector<std::string> operands = parseOperands(operandsStr);
    
    // Find instruction in the map
    auto it = instructions.find(opcode);
    if (it == instructions.end()) {
        throw std::runtime_error("Unknown instruction: " + opcode);
    }
    
    const Instruction& instr = it->second;
    
    // Handle different instruction formats
    switch (instr.format) {
        case InstructionFormat::R_TYPE: {
            if (operands.size() != 3) {
                throw std::runtime_error("R-type instruction requires 3 operands: " + instructionStr);
            }
            
            // Get register numbers
            auto rdIt = registers.find(operands[0]);
            auto rs1It = registers.find(operands[1]);
            auto rs2It = registers.find(operands[2]);
            
            if (rdIt == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
            if (rs1It == registers.end()) throw std::runtime_error("Unknown register: " + operands[1]);
            if (rs2It == registers.end()) throw std::runtime_error("Unknown register: " + operands[2]);
            
            return encodeRType(instr, rdIt->second, rs1It->second, rs2It->second);
        }
        
        case InstructionFormat::I_TYPE: {
            // Handle load instructions specially
            if (opcode == "lb" || opcode == "lh" || opcode == "lw" || opcode == "lbu" || opcode == "lhu") {
                if (operands.size() != 2) {
                    throw std::runtime_error("Load instruction requires 2 operands: " + instructionStr);
                }
                
                auto rdIt = registers.find(operands[0]);
                if (rdIt == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
                
                // Parse memory operand
                auto [offset, rs1] = parseMemoryOperand(operands[1], registers);
                
                return encodeIType(instr, rdIt->second, rs1, offset);
            }
            // Handle JALR specially
            else if (opcode == "jalr") {
                if (operands.size() != 3 && operands.size() != 2) {
                    throw std::runtime_error("JALR instruction requires 2 or 3 operands: " + instructionStr);
                }
                
                int rd, rs1, imm;
                
                if (operands.size() == 3) {
                    auto rdIt = registers.find(operands[0]);
                    auto rs1It = registers.find(operands[1]);
                    if (rdIt == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
                    if (rs1It == registers.end()) throw std::runtime_error("Unknown register: " + operands[1]);
                    
                    rd = rdIt->second;
                    rs1 = rs1It->second;
                    
                    // Check if operand[2] is a number or a symbol
                    if (isNumber(operands[2])) {
                        imm = parseNumber(operands[2]);
                    } else {
                        auto symbolIt = symbolTable.find(operands[2]);
                        if (symbolIt == symbolTable.end()) {
                            throw std::runtime_error("Unknown symbol: " + operands[2]);
                        }
                        imm = symbolIt->second;
                    }
                } else { // operands.size() == 2
                    rd = 1; // ra register
                    auto rs1It = registers.find(operands[0]);
                    if (rs1It == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
                    rs1 = rs1It->second;
                    
                    // Check if operand[1] is a number or a symbol
                    if (isNumber(operands[1])) {
                        imm = parseNumber(operands[1]);
                    } else {
                        auto symbolIt = symbolTable.find(operands[1]);
                        if (symbolIt == symbolTable.end()) {
                            throw std::runtime_error("Unknown symbol: " + operands[1]);
                        }
                        imm = symbolIt->second;
                    }
                }
                
                return encodeIType(instr, rd, rs1, imm);
            }
            // Regular I-type instructions
            else {
                if (operands.size() != 3) {
                    throw std::runtime_error("I-type instruction requires 3 operands: " + instructionStr);
                }
                
                auto rdIt = registers.find(operands[0]);
                auto rs1It = registers.find(operands[1]);
                if (rdIt == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
                if (rs1It == registers.end()) throw std::runtime_error("Unknown register: " + operands[1]);
                
                int imm;
                // Check if operand[2] is a number or a symbol
                if (isNumber(operands[2])) {
                    imm = parseNumber(operands[2]);
                } else {
                    auto symbolIt = symbolTable.find(operands[2]);
                    if (symbolIt == symbolTable.end()) {
                        throw std::runtime_error("Unknown symbol: " + operands[2]);
                    }
                    imm = symbolIt->second;
                }
                
                return encodeIType(instr, rdIt->second, rs1It->second, imm);
            }
        }
        
        case InstructionFormat::S_TYPE: {
            if (operands.size() != 2) {
                throw std::runtime_error("S-type instruction requires 2 operands: " + instructionStr);
            }
            
            auto rs2It = registers.find(operands[0]);
            if (rs2It == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
            
            // Parse memory operand
            auto [offset, rs1] = parseMemoryOperand(operands[1], registers);
            
            return encodeSType(instr, rs1, rs2It->second, offset);
        }
        
        case InstructionFormat::B_TYPE: {
            if (operands.size() != 3) {
                throw std::runtime_error("B-type instruction requires 3 operands: " + instructionStr);
            }
            
            auto rs1It = registers.find(operands[0]);
            auto rs2It = registers.find(operands[1]);
            if (rs1It == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
            if (rs2It == registers.end()) throw std::runtime_error("Unknown register: " + operands[1]);
            
            int imm;
            // Check if operand[2] is a number or a symbol
            if (isNumber(operands[2])) {
                imm = parseNumber(operands[2]);
            } else {
                auto symbolIt = symbolTable.find(operands[2]);
                if (symbolIt == symbolTable.end()) {
                    throw std::runtime_error("Unknown symbol: " + operands[2]);
                }
                imm = symbolIt->second - currentAddress;
            }
            
            return encodeBType(instr, rs1It->second, rs2It->second, imm);
        }
        
        case InstructionFormat::U_TYPE: {
            if (operands.size() != 2) {
                throw std::runtime_error("U-type instruction requires 2 operands: " + instructionStr);
            }
            
            auto rdIt = registers.find(operands[0]);
            if (rdIt == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
            
            int imm;
            // Check if operand[1] is a number or a symbol
            if (isNumber(operands[1])) {
                imm = parseNumber(operands[1]);
            } else {
                auto symbolIt = symbolTable.find(operands[1]);
                if (symbolIt == symbolTable.end()) {
                    throw std::runtime_error("Unknown symbol: " + operands[1]);
                }
                imm = symbolIt->second;
            }
            
            return encodeUType(instr, rdIt->second, imm);
        }
        
        case InstructionFormat::J_TYPE: {
            if (operands.size() != 2 && operands.size() != 1) {
                throw std::runtime_error("J-type instruction requires 1 or 2 operands: " + instructionStr);
            }
            
            int rd, imm;
            
            if (operands.size() == 2) {
                auto rdIt = registers.find(operands[0]);
                if (rdIt == registers.end()) throw std::runtime_error("Unknown register: " + operands[0]);
                rd = rdIt->second;
                
                // Check if operand[1] is a number or a symbol
                if (isNumber(operands[1])) {
                    imm = parseNumber(operands[1]);
                } else {
                    auto symbolIt = symbolTable.find(operands[1]);
                    if (symbolIt == symbolTable.end()) {
                        throw std::runtime_error("Unknown symbol: " + operands[1]);
                    }
                    imm = symbolIt->second - currentAddress;
                }
            } else { // operands.size() == 1
                rd = 1; // ra register
                
                // Check if operand[0] is a number or a symbol
                if (isNumber(operands[0])) {
                    imm = parseNumber(operands[0]);
                } else {
                    auto symbolIt = symbolTable.find(operands[0]);
                    if (symbolIt == symbolTable.end()) {
                        throw std::runtime_error("Unknown symbol: " + operands[0]);
                    }
                    imm = symbolIt->second - currentAddress;
                }
            }
            
            return encodeJType(instr, rd, imm);
        }
        
        default:
            throw std::runtime_error("Unsupported instruction format for " + opcode);
    }
}

// Function to write machine code to output file in little-endian format
void writeMachineCode(std::ofstream& outFile, uint32_t machineCode) {
    // Write byte 0 (least significant byte)
    outFile << std::bitset<8>((machineCode >> 0) & 0xFF).to_string() << std::endl;
    // Write byte 1
    outFile << std::bitset<8>((machineCode >> 8) & 0xFF).to_string() << std::endl;
    // Write byte 2
    outFile << std::bitset<8>((machineCode >> 16) & 0xFF).to_string() << std::endl;
    // Write byte 3 (most significant byte)
    outFile << std::bitset<8>((machineCode >> 24) & 0xFF).to_string() << std::endl;
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " input_file [output_file]" << std::endl;
        return 1;
    }
    
    // Set input and output file names
    std::string inputFile = argv[1];
    std::string outputFile = (argc > 2) ? argv[2] : "memoria.mif";
    
    // Create instruction and register maps
    std::unordered_map<std::string, Instruction> instructions = createInstructionMap();
    std::unordered_map<std::string, int> registers = createRegisterMap();
    
    // First pass: Build symbol table
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Error: Could not open input file " << inputFile << std::endl;
        return 1;
    }
    
    std::unordered_map<std::string, uint32_t> symbolTable;
    uint32_t address = 0;
    std::string line;
    
    while (std::getline(inFile, line)) {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        line = trim(line);
        if (line.empty()) continue;
        
        // Check for label
        size_t labelPos = line.find(':');
        if (labelPos != std::string::npos) {
            std::string label = trim(line.substr(0, labelPos));
            symbolTable[label] = address;
            
            // Check if there's an instruction after the label
            line = trim(line.substr(labelPos + 1));
            if (line.empty()) continue;
        }
        
        // Increment address by 4 bytes for each instruction
        address += 4;
    }
    
    inFile.close();
    
    // Second pass: Assemble instructions
    inFile.open(inputFile);
    if (!inFile) {
        std::cerr << "Error: Could not open input file " << inputFile << std::endl;
        return 1;
    }
    
    std::ofstream outFile(outputFile);
    if (!outFile) {
        std::cerr << "Error: Could not open output file " << outputFile << std::endl;
        return 1;
    }
    
    address = 0;
    while (std::getline(inFile, line)) {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        line = trim(line);
        if (line.empty()) continue;
        
        // Remove label if present
        size_t labelPos = line.find(':');
        if (labelPos != std::string::npos) {
            line = trim(line.substr(labelPos + 1));
            if (line.empty()) continue;
        }
        
        try {
            // Assemble the instruction
            uint32_t machineCode = assembleInstruction(line, instructions, registers, symbolTable, address);
            
            // Write the machine code to output file
            writeMachineCode(outFile, machineCode);
            
            // Increment address by 4 bytes for each instruction
            address += 4;
        } catch (const std::exception& e) {
            std::cerr << "Error assembling instruction: " << line << std::endl;
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }
    
    inFile.close();
    outFile.close();
    
    std::cout << "Assembly successful. Output written to " << outputFile << std::endl;
    return 0;
}