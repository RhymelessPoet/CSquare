import os
import re

# ====================== 配置项 ======================
# 要统计的代码根目录
TARGET_DIR = "./"
# 需要统计的文件后缀
SUPPORT_SUFFIX = [".py", ".java", ".js", ".c", ".cpp", ".h", ".html", ".css"]
# 需要排除的文件夹（如venv、node_modules、dist）
EXCLUDE_DIR = ["thirdparty", "node_modules", ".vscode", "build", ".git"]
# ====================================================

# 函数/方法匹配正则（适配主流语言）
FUNC_PATTERN = re.compile(
    r"^\s*(def\s+|function\s+|public\s+|private\s+|protected\s+|static\s+)*\w+\s*\([^)]*\)\s*(:|\{)"
)

# 统计数据初始化
total_files = 0
total_lines = 0
total_empty_lines = 0
total_comment_lines = 0
total_functions = 0

def count_lines(file_path):
    """统计单个文件的行数、注释、空行、函数"""
    global total_files, total_lines, total_empty_lines, total_comment_lines, total_functions
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
    except:
        # 编码异常文件跳过
        return

    total_files += 1
    file_lines = len(lines)
    empty_lines = 0
    comment_lines = 0
    func_count = 0

    for line in lines:
        stripped = line.strip()
        # 空行
        if not stripped:
            empty_lines += 1
            continue
        # 单行注释
        if stripped.startswith(("#", "//", "/*", "--")):
            comment_lines += 1
        # 块注释开头（/* <!--）
        elif stripped.startswith(("/*", "<!--")):
            comment_lines += 1
        # 匹配函数
        if FUNC_PATTERN.match(stripped):
            func_count += 1

    # 累加总数
    total_lines += file_lines
    total_empty_lines += empty_lines
    total_comment_lines += comment_lines
    total_functions += func_count

    # 打印单个文件结果
    print(f"📄 {file_path} | 总行数：{file_lines} | 空行：{empty_lines} | 注释：{comment_lines} | 函数：{func_count}")

def scan_dir(root_dir):
    """递归遍历目录"""
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # 排除不需要的目录
        dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIR]
        for file in filenames:
            # 只处理指定后缀文件
            if os.path.splitext(file)[1] in SUPPORT_SUFFIX:
                file_path = os.path.join(dirpath, file)
                count_lines(file_path)

if __name__ == "__main__":
    print("=" * 80)
    print("开始统计代码...")
    print("=" * 80)
    scan_dir(TARGET_DIR)

    # 输出最终统计结果
    print("\n" + "=" * 80)
    print("📊 代码统计总览")
    print("=" * 80)
    print(f"✅ 统计文件总数：{total_files} 个")
    print(f"✅ 代码总行数：{total_lines} 行")
    print(f"✅ 空行总数：{total_empty_lines} 行")
    print(f"✅ 注释总行数：{total_comment_lines} 行")
    print(f"✅ 有效代码行：{total_lines - total_empty_lines - total_comment_lines} 行")
    print(f"✅ 函数/方法总数：{total_functions} 个")
    print("=" * 80)