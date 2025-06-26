import shutil
import subprocess
from pathlib import Path

# --- 获取当前目录 ---
current_dir = Path(__file__).resolve().parent

# --- 文件与路径配置 ---
XML_FILE = current_dir / "esp_comm.xml"
OUTPUT_DIR = current_dir / "./mavlink_gen"
PYMAVGEN_PATH = current_dir / "../../from_github/mavlink/pymavlink/tools/mavgen.py"

# --- 检查 XML 是否存在 ---
if not XML_FILE.is_file():
    print(f"[❌] 找不到 XML 文件: {XML_FILE}")
    exit(1)

# --- 删除旧目录（如果存在） ---
if OUTPUT_DIR.exists():
    print(f"[🧹] 清空目录 {OUTPUT_DIR}...")
    shutil.rmtree(OUTPUT_DIR)

# --- 重新创建目录（确保存在） ---
print(f"[📁] 创建目录 {OUTPUT_DIR}...")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# --- 调用 mavgen 生成头文件 ---
print(f"[⚙️] 正在生成 MAVLink 头文件...")
cmd = [
    "python", str(PYMAVGEN_PATH.resolve()),
    "--lang=C",
    "--wire-protocol", "2.0",
    "--output", str(OUTPUT_DIR.resolve()),
    str(XML_FILE.resolve())
]
subprocess.run(cmd, check=True)

# --- 结果 ---
print(f"[✅] 生成成功，路径：{OUTPUT_DIR.resolve()}")
