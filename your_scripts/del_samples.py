import os

def delete_samples_files(directory):
    # 遍历目录
    for root, dirs, files in os.walk(directory):
        for file in files:
            # 检查文件扩展名是否为 .samples
            if file.endswith('.samples'):
                # 构造完整的文件路径
                file_path = os.path.join(root, file)
                # 删除文件
                os.remove(file_path)
                # 打印删除文件的信息
                print(f"Deleted file: {file_path}")

# 指定需要清理的目录路径
directory_to_clean = '/mnt/mydrive/megasampler/time_limt_900'
delete_samples_files(directory_to_clean)
