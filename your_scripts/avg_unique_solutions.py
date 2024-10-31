import os

def calculate_average_lines(directory):
    # 遍历指定目录及其所有子目录
    for root, dirs, files in os.walk(directory):
        sample_files = [f for f in files if f.endswith('.samples')]
        if sample_files:  # 只有当存在 .samples 文件时才进行计算
            total_lines = 0
            file_count = len(sample_files)

            for file in sample_files:
                file_path = os.path.join(root, file)
                line_count = 0
                with open(file_path, 'r', encoding='utf-8') as f:
                    for line in f:
                        line_count += 1
                total_lines += line_count

            # 计算平均行数
            average_lines = total_lines / file_count if file_count > 0 else 0

            # 将结果写入新文件
            result_file_path = os.path.join(root, 'average_lines.txt')
            with open(result_file_path, 'w', encoding='utf-8') as result_file:
                result_file.write(f"Average number of lines: {average_lines}\n")
                result_file.write(f"Total files counted: {file_count}\n")

# 使用示例
directory_path = '/mnt/mydrive/liasampling/time_limt_900/QF_LIA'  # 替换为你的目录路径
calculate_average_lines(directory_path)
