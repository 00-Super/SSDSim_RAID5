#!/bin/bash

# 确保在项目根目录执行，且 ssd 文件已编译并具有可执行权限
if [ ! -x "./ssd" ]; then
    echo "错误: 未找到可执行文件 ./ssd，或者没有执行权限。请先编译代码。"
    exit 1
fi

echo "开始批量运行 trace 文件..."

# 遍历 trace 文件夹下所有以 .csv 结尾的文件
for trace_file in trace/*.csv; do
    # 确保文件真实存在（防止目录下没有 csv 文件时报错）
    if [ -f "$trace_file" ]; then
        echo "=================================================="
        echo "正在运行: ./ssd 1 $trace_file 4"
        
        # 执行你的 C 程序，传入对应的参数
        ./ssd 1 "$trace_file" 4
        
        # 如果你想让每个任务跑完停顿一下，可以取消下面这行的注释
        # sleep 1 
    fi
done

echo "=================================================="
echo "所有 trace 文件运行完毕！"