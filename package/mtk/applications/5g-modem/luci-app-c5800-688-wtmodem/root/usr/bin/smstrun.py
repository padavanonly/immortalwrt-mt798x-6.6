import json
import os
import time
import requests
import subprocess
from datetime import datetime

def read_token_from_config():
    config_path = "/usr/bin/smstrun.conf"
    try:
        with open(config_path, 'r') as file:
            token = file.read().strip()  
            if not token:  
                print("未填写token，程序已退出！")
                exit()
            return token
    except FileNotFoundError:
        print("未找到配置文件，程序已退出！")
        exit()

def read_title_from_config():
    title_path = "/usr/bin/smstrun-title.conf"
    try:
        with open(title_path, 'r') as file:
            title = file.read().strip()  
            if not title:
                title = "CPE短信转发标题未定义"
            return title
    except FileNotFoundError:
        return "CPE短信转发标题未定义"

def write_summary_to_file(count, out):
    summary_path = "/tmp/smstrunsum.conf"
    current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    try:
        with open(summary_path, 'a') as file:  
            file.write(f"本次转发时间: {current_time}\n") 
            file.write(f"已完成: {count}次短信转发\n") 
            file.write(f"转发内容:\n\n{out}\n") 
            file.write("\n")
    except Exception as e:
        print(f"写入文件失败: {e}")

def check_lock(lock_file):
    if os.path.exists(lock_file):
        print("脚本已经在运行中。")
        return False
    else:
        try:
            with open(lock_file, 'w') as file:
                file.write(str(os.getpid()))
            return True
        except Exception as e:
            print("无法创建锁文件: ", e)
            return False

def remove_lock(lock_file):
    try:
        os.remove(lock_file)
    except Exception as e:
        print("无法删除锁文件: ", e)

def forward():
    lock_file = "/tmp/smstrun.lock"
    if not check_lock(lock_file):
        return
    
    try:
        token = read_token_from_config()
        print("Enjoy! 已完成测试并开启转发功能，重启后完成开机自启。")
        count = 0
        while True:
            try:
                result = subprocess.run(['sh', '/usr/bin/smstrun.sh'], capture_output=True, text=True, check=True)
                out = result.stdout
                if "发件人" in out:
                    title = read_title_from_config()
                    message = out
                    push_pushplus(message, token, title)
                    count += 1
                    write_summary_to_file(count, out)
                else:
                    print("未检测到新消息，继续检测...")
            except subprocess.CalledProcessError as e:
                print(f"执行命令失败: {e}. 返回值: {e.returncode}. 错误信息: {e.stderr}")
            except UnicodeDecodeError as e:
                print(f"解码错误: {e}. 尝试重新运行。")
            except Exception as e:
                print("发生未处理异常: ", str(e))
            time.sleep(5)
    finally:
        remove_lock(lock_file)

def push_pushplus(message, token, title):
    url = "http://www.pushplus.plus/send"
    data = {
        "token": token,
        "title": title,
        "content": message
    }
    try:
        response = requests.post(url, json=data)
        result = response.json()
        print("Response:\n", result)
    except Exception as e:
        print("Error occurred: ", str(e))

if __name__ == '__main__':
    forward()
