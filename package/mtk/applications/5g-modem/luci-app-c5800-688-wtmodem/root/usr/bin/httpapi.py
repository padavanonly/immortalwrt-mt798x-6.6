import asyncio
import websockets
import subprocess  # 导入 subprocess 模块

WS_PORT = 5000

# 异步处理外部命令并通过 WebSocket 发送结果
async def execute_command_and_send(command, websocket):
    # 拼接最终执行的命令字符串
    commands = f"sendat 1 '{command}'"
    # 执行命令并获取输出
    process = subprocess.Popen(commands, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout, stderr = process.communicate()
    
    # 如果命令执行过程中有错误输出，将错误作为响应发送
    decoded_string = stderr if stderr else stdout

    # 发送执行结果到 WebSocket 客户端
    await websocket.send(decoded_string)

# WebSocket 处理函数
async def handle_websocket(websocket, path):
    async for message in websocket:
        print(f"Received command from frontend: {message}")
        # 调用异步执行命令函数，并传递 WebSocket
        await execute_command_and_send(message, websocket)

# 启动 WebSocket 服务器
async def start_websocket_server():
    async with websockets.serve(handle_websocket, "0.0.0.0", WS_PORT):
        print(f"WebSocket server running on port {WS_PORT}...")
        await asyncio.Future()  # 保持服务器运行

if __name__ == "__main__":
    # 使用 asyncio 启动 WebSocket 服务器
    asyncio.run(start_websocket_server())
