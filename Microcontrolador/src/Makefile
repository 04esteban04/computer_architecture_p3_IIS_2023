all:
	./install.sh
	cd Proyectos/p3
	. $$HOME/esp/esp-idf/export.sh 
	idf.py set-target esp32c6
	idf.py build
	sudo chmod a+rw /dev/ttyACM0
	idf.py -p /dev/ttyACM0 flash

b:
	idf.py build

bf:
	idf.py build
	sudo chmod a+rw /dev/ttyACM1
	idf.py -p /dev/ttyACM1 flash

m:
	sudo chmod a+rw /dev/ttyACM1
	idf.py monitor

bfm:
	idf.py build
	sudo chmod a+rw /dev/ttyACM1
	idf.py -p /dev/ttyACM1 flash monitor
