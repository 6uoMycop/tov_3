import sys
import random

# sys.argv[1] - path to source file to process
# sys.argv[2] - start line of original file

pair_commands = [
    ["push {}", "pop {}"],
    ["inc {}", "dec {}"]
]

reg = [
    "eax",
    "ebx",
    "ecx",
    "edx"
]
asm = "__asm "
col = ";\n"


def insert_dead_code(file_data, index):
    num_inserted_lines = 0

    if not random.randint(0, 2):  # (prob 1/3)
        if not random.randint(0, 3):  # nop (prob 1/4)
            file_data.insert(index, asm + "nop" + col)
            num_inserted_lines += 1
        if not random.randint(0, 3):  # mov self (prob 1/4)
            reg_num = random.randint(0, 3)
            file_data.insert(index, asm + "mov {} {}".format(reg[reg_num], reg[reg_num]) + col)
            num_inserted_lines += 1

        # pair_commands
        com_num = random.randint(0, len(pair_commands) - 1)
        reg_num = random.randint(0, 3)

        file_data.insert(index + num_inserted_lines, asm + pair_commands[com_num][0].format(reg[reg_num]) + col)
        num_inserted_lines += 1

        num_inserted_lines += insert_dead_code(file_data, index + 1)

        file_data.insert(index + num_inserted_lines, asm + pair_commands[com_num][1].format(reg[reg_num]) + col)
        num_inserted_lines += 1

    return num_inserted_lines


file = open(str(sys.argv[1]), 'r')
file_lines = file.readlines()
file.close()

can_insert = 0
i = int(sys.argv[2])
while i < len(file_lines):
    if '{' in file_lines[i]:
        can_insert += 1
    if '}' in file_lines[i]:
        can_insert -= 1

    if can_insert:
        i += insert_dead_code(file_lines, i + 1)

    i += 1

file = open(str(sys.argv[1]), 'w')
file.writelines(file_lines)
file.close()
