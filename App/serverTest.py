import socket

def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', 12345))  # Binds to the same address and port as the C client
    server_socket.listen(1)

    print("Server started, waiting for connections...")

    while True:
        conn, addr = server_socket.accept()
        print(f"Connection established from {addr}")

        data = conn.recv(1024)  # Receive data (assuming it's smaller than 1024 bytes)
        if data:
            print(f"Received data: {data.decode('utf-8')}")

            # Echo the received data back to the client
            conn.sendall(b"Server received your data: " + data)

        conn.close()


start_server()