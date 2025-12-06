import serial
import serial.tools.list_ports as port_list
import time
import threading
from cobs import cobs

BROADCAST_DELAY = 50

# Class for managing input;
class BroadcastLock:
    def __init__(self):
        self.pwmR = 0
        self.pwmL = 0
        self.flagSM = 0
        self.lock = threading.Lock()

    def setPwm(self, pwmValL=None, pwmValR=None):
        with self.lock:
            if pwmValR != None:
                if (abs(pwmValR) < 1400):
                    self.pwmR = pwmValR
                else:
                    self.pwmR = 0
            if pwmValL != None:
                if (abs(pwmValL) < 1400):
                    self.pwmL = pwmValL
                else:
                    self.pwmL = 0
                
    def setFlag(self, flagBool):
        with self.lock:
            if flagBool:
                self.flagSM = 1
            else:
                self.flagSM = 0
        
    
    def getPwm(self):
        with self.lock:
            return [self.pwmR, self.pwmL]
        
    

UserInput = BroadcastLock()


# Determing the current ports
#ports = list(port_list.comports())
#for p in ports:
#    print (p)

# Serial port for determining the input
serialPort = serial.Serial(
    port="COM4", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

# Global variable for reading input string
serialString = ""  # Used to hold data coming over UART

# Broadcast packet formatter
def FormBroadcastPacket(pwmR, pwmL, flagActive):
    
    packet = bytes()
    if (abs(pwmR) < 1600) and (abs(pwmL) < 1600):
        packet = pwmL.to_bytes(2, 'little', signed=True) + pwmR.to_bytes(2, 'little', signed=True)
    else:
        packet = b'\0' + b'\0'

    packet = packet + flagActive.to_bytes(1, 'little')

    formTime = time.time()
    packet = packet + int(formTime).to_bytes(4, 'little') + int((formTime-int(formTime))*1000).to_bytes(2,'little')
    sendOut = cobs.encode(packet) + b'\0'
    #print("BROADCASTING", packet, "SIZE:", len(packet))
    return sendOut
    
    
# Main serial connection code
def main(exit, sendMsg):
    print('Main program')

    lastPacket = bytes()
    timeCount = int(time.perf_counter_ns() / 1000000)
    while not exit.is_set(): # MAIN EVENT LOOP
        # Read from Serial
        serialString = serialPort.readline()
        
        # Time ping and output packet send
        if (int(time.perf_counter_ns() / 1000000) - timeCount > BROADCAST_DELAY):
            #print("TIME PING")
            if sendMsg.is_set():
                pwmList = UserInput.getPwm()
                #print(pwmList)

                serialPort.write(FormBroadcastPacket(pwmList[0],pwmList[1],1))
            timeCount = int(time.perf_counter_ns() / 1000000)
            
            
        #print("RUNNING")
        if len(serialString)>1:
            #Print the contents of the serial data
            #print("RAW IN: ", serialString)
            serialCatch = serialString
            #print(serialCatch)
            packetList = serialCatch.split(b'\x00')
            for packet in packetList:
                
                if len(packet) > 1:
                    #print("CURRENT PACKET: ", packet, "LENGTH", len(packet))
                    try:
                        decodedPacket = cobs.decode(packet)
                        #print("DECODE PACKET: ", decodedPacket, "LENGTH", len(decodedPacket))
                        if len(decodedPacket) == 37:
                            if decodedPacket != lastPacket:
                                data = decodedPacket.split(b':')
                                lastPacket = decodedPacket
                                #print(data)
                                print("PWM_L:", int.from_bytes(data[0], 'little', signed=True),"PWM_R:", int.from_bytes(data[1], 'little', signed=True), "ENC_L:", int.from_bytes(data[2], 'little', signed=True), "ENC_R:", int.from_bytes(data[3], 'little',signed=True), "SHOOTER STATE:", data[10])
                    except:
                        pass
                decodedPacket = ''
                #print("END FOR")

    #EXIT
    serialPort.close()
    print('Exiting')
    # Add some possible cleanup code here

exit = threading.Event()
sendMsg = threading.Event()

main_thread = threading.Thread(name='main program',
                               target=main, args=(exit, sendMsg))
main_thread.start()


while True:
    testIn = input().lower()
    if testIn == 'kill':
        print('Terminating program')
        
        exit.set()
        break
    if testIn == 'send':
        print("STARTING BROADCAST")
        sendMsg.set()
        pass
    if testIn == 'nosend':
        print("STOPPING BROADCAST")
        sendMsg.clear()
    if testIn == 'setr':
        print("SETTING RIGHT PWM")
        x = int()
        try:
            x = int(input())
        except:
            print("Not a Number, retry")
        UserInput.setPwm(pwmValR=x)
        print("SET SUCCESS")
        pass
    if testIn == 'setl':
        print("SETTING LEFT PWM")
        x = int()
        try:
            x = int(input())
        except:
            print("Not a Number, retry")
        UserInput.setPwm(pwmValL=x)
        print("SET SUCCESS")
        pass
    testIn = ''
    

main_thread.join()

