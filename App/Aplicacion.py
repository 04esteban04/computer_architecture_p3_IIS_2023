import sys
import socket
import os 
import numpy as np
from pathlib import Path
import tkinter as tk
from tkinter import *
from tkinter import messagebox
from threading import Thread

"""
    Clase que modela la aplicacion de escritorio
"""
class App:
    
    """
        Funcion para inicializar la aplicacion
    """
    def __init__(self, window):
        self.window = window
        self.window.title('Robot de Humedad')
        self.window.geometry("1050x550")
        self.window.resizable(False,False)        

        #Button (Conexion)
        self.connect_button = Button(window, text="Conectar", font=('Calibri',12),command=self.start_server).place(x=720,y=400)

        # Text Space (Datos sensor humedad)
        self.data_text = Text(window, width=32, height=13, wrap=WORD)
        self.data_text.place(x=200, y=105)

        # Text Space (Alerta conexion)
        self.noti_text = Text(window, width=33, height=6, wrap=WORD)
        self.noti_text.place(x=620, y=250)

        # Promedio de datos
        self.promedio_label = Label(window,width=36, height=2)
        self.promedio_label.place(x=620, y=107)

    """
        Funcion para realizar la conexion y mostar los datos
    """
    def start_server(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind(('0.0.0.0', 12345))  # Binds to the same address and port as the C client
        server_socket.listen(1)

        #print("Server started, waiting for connections...")
        self.noti_text.insert(END, "Servidor iniciado, esperando por conexiones ... \n")
        received_data_temp = []
        received_data_hum = []

        self.noti_text.update_idletasks()

        while True:
            conn, addr = server_socket.accept()
            data = conn.recv(1024)  # Receive data (assuming it's smaller than 1024 bytes)
            
            self.show_data(addr, data, received_data_temp, received_data_hum)

            conn.sendall(b"El servidor recibio sus datos: " + data)
            conn.close()

    """
        Funcion para mostrar los datos
    """
    def show_data(self, addr, data, received_data_temp, received_data_hum):
        #print(f"Connection established from {addr} \n")
        self.noti_text.insert(END, f"Conexion establecida en {addr} \n")
        self.noti_text.see(END)


        if data:
            data_deco = data.decode('utf-8')
            #print(f"Received data: {data_deco}")
            
            if (data_deco.strip() != "Error al leer el sensor: -1" and data_deco.strip() != "Error al leer el sensor: -2"):
                self.data_text.insert(END, data_deco)
                self.data_text.see(END)
                
                dataRead = self.extract_data(data_deco)

                if dataRead[0] is not None and dataRead[1] is not None:
                    received_data_temp.append(dataRead[0])
                    received_data_hum.append(dataRead[1])

                    # Calcular el promedio
                    promedio_temp = sum(received_data_temp) / len(received_data_temp)
                    promedio_hum = sum(received_data_hum) / len(received_data_hum)
                    
                    self.promedio_label.config(text=f"Temperatura: {promedio_temp:.2f}°C, Humedad: {promedio_hum:.2f}%")
                    
                    if (len(received_data_temp) == 10):
                        received_data_temp = []
                        received_data_hum = []

            self.data_text.update_idletasks()
    
    """
        Funcion para extraer los datos recibidos
    """
    def extract_data(self, data_string):
        # Dividir el string en partes utilizando la coma como separador
        parts = data_string.split(',')

        # Inicializar variables para temperatura y humedad
        temperatura = None
        humedad = None

        # Extraer temperatura y humedad de cada parte
        for part in parts:
            if 'Temperatura' in part:
                temperatura = int(part.split(':')[1].strip('°C'))
            elif 'Humedad' in part:
                humedad = int(part.split(':')[1].strip().rstrip('%'))

        return temperatura, humedad

"""
    Funcion para cargar la imagen 
"""
def load_image(name):
    #rute = os.path.join ("Imagenes/", name)
    rute = os.path.join ("/home/esteban/Desktop/repoP3-Arqui/computer_architecture_p3_IIS_2023/App/Imagenes/", name)
    image = PhotoImage (file = rute)
    return image

"""
    Main
"""
if __name__ == "__main__":
    window = Tk()
    bgMain = load_image("Principal.png")
    P_canvas= Canvas (window, width = 1050, height = 550, bg = "black")
    P_canvas.place (x = 0, y = 0)
    ImgCanvas = P_canvas.create_image (0, 0, anchor = NW, image = bgMain)
    app = App(window)
    window.mainloop()