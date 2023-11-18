import sys
import bluetooth
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
        self.window.geometry("750x450")
        self.window.resizable(False,False)
        

        #Button
        self.devices_button = Button(window, text="Buscar", font=('Calibri',12), command=self.get_devices).place(x=305,y=150)
        self.connect_button = Button(window, text="Conectar", font=('Calibri',12),command=self.connect).place(x=300,y=200)
        self.average_button = Button(window, text="Obtener Promedio", font=('Calibri',12)).place(x=450,y=380)

         # Listbox
        self.devices_listbox = Listbox(window, width=33, height=12)
        self.devices_listbox.place(x=40, y=80)

        # Text Space
        self.data_text = Text(window, width=25, height=12, wrap=WORD)
        self.data_text.place(x=415, y=80)
        self.noti_text = Text(window, width=18, height=6, wrap=WORD)
        self.noti_text.place(x=260, y=320)
        self.promedio = Label(window,width=20, height=2).place(x=443,y=322)
    
    

    def get_devices(self):
        nearby_devices = bluetooth.discover_devices()
        self.devices_listbox.delete(0, END)  # Limpiar la lista antes de agregar nuevos dispositivos
        for address in nearby_devices:
            try:
                name = bluetooth.lookup_name(address)
                self.devices_listbox.insert(END, f"{name} - {address}")
            except bluetooth.btcommon.BluetoothError as err:
                print("No se pudo obtener el nombre del dispositivo", err)

    def connect(self):
        selected_device_index = self.devices_listbox.curselection()
        if not selected_device_index:
            self.noti_text.delete("1.0", END)
            self.noti_text.insert(END, "Seleccione un dispositivo antes de intentar la conexión.")
            return

        selected_device_info = self.devices_listbox.get(selected_device_index)
        selected_device_addr = selected_device_info.split(" - ")[1]

        try:
            self.device = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
            print(selected_device_addr)
            self.device.connect((selected_device_addr,1))
            self.noti_text.insert(END, "Conectado a:", selected_device_info)

            # Inicia un hilo para recibir datos en segundo plano
            receive_thread = Thread(target=self.receive_data)
            receive_thread.start()

        except bluetooth.btcommon.BluetoothError as err:
            self.noti_text.delete("1.0", END)
            self.noti_text.insert(END, "No se pudo conectar:", err)

    def receive_data(self):
        try:
            while True:
                data = self.device.recv(1024)
                if not data:
                    break
                self.data_text.insert(END, data)
                self.data_text.see(END)
        except bluetooth.btcommon.BluetoothError as err:
            print("Error al recibir datos:", err)
        finally:
            self.device.close()
#Function to load images
def load_image(name):
    rute = os.path.join ("Imagenes", name)
    image = PhotoImage (file = rute)
    return image

if __name__ == "__main__":
    window = Tk()
    bgMain = load_image("Principal.png")
    P_canvas= Canvas (window, width = 750, height = 450, bg = "black")
    P_canvas.place (x = 0, y = 0)
    ImgCanvas = P_canvas.create_image (0, 0, anchor = NW, image = bgMain)
    app = App(window)
    window.mainloop()