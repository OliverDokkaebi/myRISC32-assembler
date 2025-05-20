# myRV32I Assembler

A C++ implementation of an assembler for the RISC-V RV32I instruction set. This assembler translates RISC-V assembly code into binary machine code that can be executed on RISC-V processors.

## Features

- Supports all core RV32I instructions:
  - R-type: `add`, `sub`, `sll`, `slt`, `sltu`, `xor`, `srl`, `sra`, `or`, `and`
  - I-type: `addi`, `slti`, `sltiu`, `xori`, `ori`, `andi`, `slli`, `srli`, `srai`
  - Load instructions: `lb`, `lh`, `lw`, `lbu`, `lhu`
  - S-type: `sb`, `sh`, `sw`
  - B-type: `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu`
  - U-type: `lui`, `auipc`
  - J-type: `jal`
  - Others: `jalr`
- Supports labels and symbols for code and data references
- Two-pass assembly for handling forward references
- Supports both register names (`a0`, `t0`, etc.) and register numbers (`x0`, `x10`, etc.)
- Handles hexadecimal and decimal immediate values
- Comprehensive error handling with descriptive messages
- Comments using `#` character

## Usage

```bash
./montador input_file [output_file]
```

If no output file is specified, the default output file will be `memoria.mif`.

## Input File Format

The input assembly file should follow RISC-V assembly syntax:

```assembly
# This is a comment
label: instruction operands  # This is an inline comment
```

Example:

```assembly
# Simple program to calculate sum of 1 to 10
main:
    addi x2, x0, 0      # Initialize sum to 0
    addi x3, x0, 1      # Initialize i to 1
    addi x4, x0, 11     # Upper bound (n+1)
loop:
    beq x3, x4, end     # If i == 11, exit loop
    add x2, x2, x3      # sum += i
    addi x3, x3, 1      # i++
    jal x0, loop        # Jump back to loop
end:
    sw x2, 0(x5)        # Store result to memory
```

## Output Format

The assembler outputs binary machine code in little-endian format, with each byte on a separate line.

## Building the Project

```bash
g++ -std=c++11 -o montador montador.cpp
```

## Supported Instruction Formats

### R-type Instructions
Format: `instruction rd, rs1, rs2`
- `add rd, rs1, rs2`: rd = rs1 + rs2
- `sub rd, rs1, rs2`: rd = rs1 - rs2
- `sll rd, rs1, rs2`: rd = rs1 << rs2
- `slt rd, rs1, rs2`: rd = (rs1 < rs2) ? 1 : 0 (signed)
- `sltu rd, rs1, rs2`: rd = (rs1 < rs2) ? 1 : 0 (unsigned)
- `xor rd, rs1, rs2`: rd = rs1 ^ rs2
- `srl rd, rs1, rs2`: rd = rs1 >> rs2 (logical)
- `sra rd, rs1, rs2`: rd = rs1 >> rs2 (arithmetic)
- `or rd, rs1, rs2`: rd = rs1 | rs2
- `and rd, rs1, rs2`: rd = rs1 & rs2

### I-type Instructions
Format: `instruction rd, rs1, imm`
- `addi rd, rs1, imm`: rd = rs1 + imm
- `slti rd, rs1, imm`: rd = (rs1 < imm) ? 1 : 0 (signed)
- `sltiu rd, rs1, imm`: rd = (rs1 < imm) ? 1 : 0 (unsigned)
- `xori rd, rs1, imm`: rd = rs1 ^ imm
- `ori rd, rs1, imm`: rd = rs1 | imm
- `andi rd, rs1, imm`: rd = rs1 & imm
- `slli rd, rs1, imm`: rd = rs1 << imm
- `srli rd, rs1, imm`: rd = rs1 >> imm (logical)
- `srai rd, rs1, imm`: rd = rs1 >> imm (arithmetic)

### Load Instructions (I-type)
Format: `instruction rd, offset(rs1)`
- `lb rd, offset(rs1)`: rd = SignExt(Mem[rs1 + offset][7:0])
- `lh rd, offset(rs1)`: rd = SignExt(Mem[rs1 + offset][15:0])
- `lw rd, offset(rs1)`: rd = Mem[rs1 + offset][31:0]
- `lbu rd, offset(rs1)`: rd = ZeroExt(Mem[rs1 + offset][7:0])
- `lhu rd, offset(rs1)`: rd = ZeroExt(Mem[rs1 + offset][15:0])

### S-type Instructions
Format: `instruction rs2, offset(rs1)`
- `sb rs2, offset(rs1)`: Mem[rs1 + offset][7:0] = rs2[7:0]
- `sh rs2, offset(rs1)`: Mem[rs1 + offset][15:0] = rs2[15:0]
- `sw rs2, offset(rs1)`: Mem[rs1 + offset][31:0] = rs2[31:0]

### B-type Instructions
Format: `instruction rs1, rs2, label/offset`
- `beq rs1, rs2, label`: if (rs1 == rs2) PC += offset
- `bne rs1, rs2, label`: if (rs1 != rs2) PC += offset
- `blt rs1, rs2, label`: if (rs1 < rs2) PC += offset (signed)
- `bge rs1, rs2, label`: if (rs1 >= rs2) PC += offset (signed)
- `bltu rs1, rs2, label`: if (rs1 < rs2) PC += offset (unsigned)
- `bgeu rs1, rs2, label`: if (rs1 >= rs2) PC += offset (unsigned)

### U-type Instructions
Format: `instruction rd, imm`
- `lui rd, imm`: rd = imm << 12
- `auipc rd, imm`: rd = PC + (imm << 12)

### J-type Instructions
Format: `instruction rd, label/offset` or `instruction label/offset`
- `jal rd, label`: rd = PC + 4; PC += offset
- `jal label`: ra = PC + 4; PC += offset (shorthand for `jal ra, label`)

### JALR Instruction (I-type)
Format: `jalr rd, rs1, offset` or `jalr rs1, offset`
- `jalr rd, rs1, offset`: rd = PC + 4; PC = rs1 + offset
- `jalr rs1, offset`: ra = PC + 4; PC = rs1 + offset (shorthand for `jalr ra, rs1, offset`)

## License

This project is released under the MIT License.

The MIT License is a permissive free software license originating at the Massachusetts Institute of Technology (MIT). It puts only very limited restriction on reuse and has good compatibility with other licenses. It permits users to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software, while providing a disclaimer of warranty. The main conditions are that the copyright notice and permission notice shall be included in all copies or substantial portions of the software.

## Authors

- **Gabriela Mota Lages Gomes**
- **João Victor de Oliveira**

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- This assembler was created for the RV32I instruction set, a subset of the RISC-V instruction set architecture.
- Thanks to the RISC-V Foundation for the open instruction set specification.
