import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

# 加载数据
def load_data(file_path):
    df = pd.read_csv(file_path)
    # 移除百分比符号并转换为浮点数
    df['Percentage'] = df['Percentage'].str.rstrip('%').astype(float) / 100
    return df

# 合并数据
def merge_data(file1, file2):
    df1 = load_data(file1)
    df2 = load_data(file2)
    # 合并两个数据集
    merged = pd.merge(df1, df2, on='Filename', suffixes=('_x', '_y'))
    return merged

# 绘制散点图并保存为PDF
def plot_data(merged_data, pdf_path):
    with PdfPages(pdf_path) as pdf:
        x = merged_data['Percentage_y']
        y = merged_data['Percentage_x']
        plt.figure(figsize=(8, 8))
        # 修改点的颜色为红色并缩小点的大小
        plt.scatter(x, y, color='red', alpha=1, s=20, marker='o', edgecolors='none')
        # plt.title('Comparison of Coverage', fontsize=18, fontweight='bold')
        plt.xlabel('Cov of SMTint', fontsize=16, fontweight='medium')
        plt.ylabel('Cov of HighCov', fontsize=16, fontweight='medium')
        plt.xlim(0, 1)  # 设置x轴的范围
        plt.ylim(0, 1)  # 设置y轴的范围
        plt.grid(True)  # 显示网格
        # 添加对角线
        plt.plot([0, 1], [0, 1], 'gray', linestyle='--')  # 对角线颜色为灰色，线型为虚线
        pdf.savefig()  # 保存图形到PDF
        plt.close()

# 使用示例
file1 = 'csv_dir/time_limit_900_lia-s.csv'
file2 = 'csv_dir/time_limit_900_smt.csv'
pdf_path = 'pdf_dir/time_limit_900_lia-s_vs_smt.pdf'
merged_data = merge_data(file1, file2)
plot_data(merged_data, pdf_path)
