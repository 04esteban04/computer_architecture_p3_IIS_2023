import sys
import socket
import os 
import numpy as np
from pathlib import Path
import tkinter as tk
from tkinter import *
from tkinter import messagebox
from threading import Thread


class App:
    

    def __init__(self, window):
        self.window = window
        self.window.title('Robot de Humedad')
        self.window.geometry("1050x550")
        self.window.resizable(False,False)        

        #Button (Conexion)
        self.connect_button = Button(window, text="Conectar", font=('Calibri',12),command=self.start_server).place(x=680,y=180)

        # Text Space (Datos sensor humedad)
        self.data_text = Text(window, width=28, height=13, wrap=WORD)
        self.data_text.place(x=210, y=115)

        # Text Space (Alerta conexion)
        self.noti_text = Text(window, width=30, height=4, wrap=WORD)
        self.noti_text.place(x=600, y=290)

        # Promedio de datos
        self.promedio = Label(window,width=20, height=2).place(x=635,y=107)

    def start_server(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind(('0.0.0.0', 12345))  # Binds to the same address and port as the C client
        server_socket.listen(1)

        print("Server started, waiting for connections...")
        self.noti_text.insert("Server started, waiting for connections...")
        received_data_list = []

        while True:
            conn, addr = server_socket.accept()
            print(f"Connection established from {addr}")

            data = conn.recv(1024)  # Receive data (assuming it's smaller than 1024 bytes)
            if data:
                data_deco = data.decode('utf-8')
                self.data_text.insert(data_deco)
                received_data_list.append(data_deco)

                if len(received_data_list) > 25:
                    received_data_list = received_data_list[-25:]

                print(f"Received data: {data_deco}")

                # Calcular el promedio
                promedio = sum(received_data_list) / len(received_data_list)
                self.promedio.textvariable(promedio)

                # Echo the received data back to the client
                conn.sendall(b"Server received your data: " + data)

            conn.close()

#Function to load images
def load_image(name):
    #rute = os.path.join ("Imagenes/", name)
    rute = os.path.join ("/home/esteban/Desktop/repoP3-Arqui/computer_architecture_p3_IIS_2023/App Arqui/Imagenes/", name)
    image = PhotoImage (file = rute)
    return image

if __name__ == "__main__":
    window = Tk()
    bgMain = load_image("Principal.png")
    P_canvas= Canvas (window, width = 1050, height = 550, bg = "black")
    P_canvas.place (x = 0, y = 0)
    ImgCanvas = P_canvas.create_image (0, 0, anchor = NW, image = bgMain)
    app = App(window)
    window.mainloop()