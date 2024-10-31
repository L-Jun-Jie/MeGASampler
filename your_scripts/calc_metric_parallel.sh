#!/bin/bash

# 确保至少有一个参数被提供
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <SMT_DIR>"
    exit 1
fi

SMT_DIR=$1
SAMPLES_DIR="/mnt/mydrive/liasampling/no_pls-lia/time_limt_900/$SMT_DIR"  # 在 samples 目录路径中附加 SMT_DIR
PYTHON_SCRIPT="your_scripts/calc_metric.py"
MODE="wire_coverage"
OUTPUT_DIR="calc_res/liasampling/no_pls-lia/time_limt_900/$SMT_DIR"  # 在输出目录路径中附加 SMT_DIR
CPU_CORES=$(nproc)

# 创建输出目录，如果不存在的话
mkdir -p "$OUTPUT_DIR"

# 使用 find 和 parallel 来并行处理文件
find "$SAMPLES_DIR" -type f -name "*.samples" -print0 | parallel -0 -j "$CPU_CORES" \
your_scripts/calc_samples.sh {} "$SMT_DIR" "$PYTHON_SCRIPT" "$MODE" "$OUTPUT_DIR"
