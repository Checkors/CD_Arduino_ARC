import serial
import serial.tools.list_ports as port_list
from cobs import cobs

ports = list(port_list.comports())
for p in ports:
    print (p)
    
serialPort = serial.Serial(
    port="COM4", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
serialString = ""  # Used to hold data coming over UART


while 1:
    # Read data out of the buffer until a carraige return / new line is found
    serialString = serialPort.readline()

    # Print the contents of the serial data
    try:
        print(serialString.decode("Ascii"))
    except:
        pass