#!/usr/bin/env bash
# H747 双核编译+烧录：增量编译、显示进度并自动复位。
# 用法：./flash.sh [cm7|cm4|all]

set -o pipefail

PROG="${STM32_PROGRAMMER_CLI:-/opt/st/stm32cubeclt_1.21.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI}"
CMAKE_BIN="${CMAKE_BIN:-cmake}"
FREQ=4000
CM7_ELF="CM7/build/STM32H747_CM7.elf"
CM4_ELF="CM4/build/STM32H747_CM4.elf"

SKIP_LINES="STMicroelectronics|ST-LINK|Board|Voltage|SWD freq|Connect mode|Reset mode|Device ID|Revision ID|Device name|NVM size|Device type|Device CPU|BL Version|Warning|Memory Read|Disabling|Time elapsed|^$"

do_build() {
    if [[ ! -f build/Debug/CMakeCache.txt ]]; then
        "$CMAKE_BIN" --preset Debug || { echo "配置失败"; exit 1; }
    fi
    "$CMAKE_BIN" --build --preset Debug || { echo "编译失败"; exit 1; }

    python3 -c '
import json

commands = []
for path in ("CM7/build/compile_commands.json", "CM4/build/compile_commands.json"):
    with open(path, encoding="utf-8") as source:
        commands.extend(json.load(source))
with open("build/Debug/compile_commands.json", "w", encoding="utf-8") as output:
    json.dump(commands, output, indent=2)
' 2>/dev/null
}

flash_core() {
    echo "[$1]"
    "$PROG" -c port=SWD freq="$FREQ" -d "$2" 2>&1 | grep -vE "$SKIP_LINES"
}

if [[ ! -x "$PROG" ]]; then
    echo "未找到 STM32_Programmer_CLI：$PROG"
    echo "可通过 STM32_PROGRAMMER_CLI 环境变量指定路径。"
    exit 1
fi

do_build
case "${1:-all}" in
    cm7)
        flash_core "CM7" "$CM7_ELF"
        "$PROG" -c port=SWD freq="$FREQ" -Rst >/dev/null 2>&1
        ;;
    cm4)
        flash_core "CM4" "$CM4_ELF"
        "$PROG" -c port=SWD freq="$FREQ" -Rst >/dev/null 2>&1
        ;;
    all)
        flash_core "CM7" "$CM7_ELF"
        flash_core "CM4" "$CM4_ELF"
        "$PROG" -c port=SWD freq="$FREQ" -Rst >/dev/null 2>&1
        ;;
    *)
        echo "用法：$0 cm7 | cm4 | all"
        exit 1
        ;;
esac
echo "完成"
