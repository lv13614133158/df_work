#!/bin/bash

# ====================
# 格式化JSON函数
# ====================

format_json() {
    local input_file="$1"
    local output_file="$2"

    > "$output_file" # 清空输出文件

    tab_level=0

    while IFS= read -r line; do
        while IFS= read -rn1 char; do
            case "$char" in
                "{")
                    printf "{" >> "$output_file"
                    ((tab_level++))
                    printf "\n%*s" $((tab_level * 4)) "" >> "$output_file"
                    ;;
                "}")
                    ((tab_level--))
                    if ((tab_level < 0)); then
                        tab_level=0
                    fi
                    printf "\n%*s}" $((tab_level * 4)) "" >> "$output_file"
                    ;;
                ",")
                    printf ",\n%*s" $((tab_level * 4)) "" >> "$output_file"
                    ;;
                *)
                    printf "%s" "$char" >> "$output_file"
                    ;;
            esac
        done <<< "$line"
        printf "\n" >> "$output_file"
    done < "$input_file"
}


if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_log_file>"
    exit 1
fi

file_path="$1"
output_file="idps_test_json.log"
format_json "$file_path" "$output_file"
echo "优化完成  保存文件： $output_file"