import os
import re

def find_numbers_in_file(filename):
    depth = 0
    total_vars = 0
    with open(filename, 'r') as file:
        content = file.read()

        # 查找 Formula tree depth 后的数字
        depth_match = re.search(r'Formula tree depth (\d+)', content)
        if depth_match:
            depth = int(depth_match.group(1))

        # 查找各种变量类型后的数字
        vars_to_search = ['Arrays', 'Bit-vectors', 'Bools', 'Bits',
                          'Uninterpreted functions', 'Ints', 'Reals']
        for var in vars_to_search:
            var_match = re.search(rf'{var} (\d+)', content)
            if var_match:
                total_vars += int(var_match.group(1))

    return depth, total_vars

def main(directory):
    total_depth = 0
    total_vars = 0
    count = 0

    # 遍历指定文件夹下的所有.out文件
    for filename in os.listdir(directory):
        if filename.endswith('.log'):
            filepath = os.path.join(directory, filename)
            depth, vars_sum = find_numbers_in_file(filepath)
            total_depth += depth
            total_vars += vars_sum
            count += 1
            print(f'File: {filename} - (Depth, Vars): ({depth}, {vars_sum})')

    if count > 0:
        # 计算平均值
        average_depth = total_depth / count
        average_vars = total_vars / count
        print(f'Average Depth: {average_depth:.2f}, Average Vars: {average_vars:.2f}')
    else:
        print("No .out files found in the directory.")

if __name__ == "__main__":
    directory = input("Enter the directory path containing .out files: ")
    main(directory)
