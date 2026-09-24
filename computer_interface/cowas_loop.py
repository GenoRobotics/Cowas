#!/usr/bin/env python
# -*- coding: utf-8 -*-

from pickle import TRUE
import threading
import random
import time
from time import strftime
from typing import Type
import zmq
from datetime import datetime, timezone
from sample_class import *
import serial


ENDPOINT = "ipc://routing.ipc"
PATH_SAMPLE_FILE='Samples_planned'
PATH_EXECUTED_SAMPLE='Samples_executed'

def configSerialPort():
    global arduino
    arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    # WINDOWS
    # arduino = serial.Serial("COM4", 9600, timeout=1)

def thread_function(worker):
    #! change here if new functions are added
    switcher ={
            "reloadManifoldFunction" : lambda : reloadManifoldFunction(),
            "purgeSterivexFunction": lambda : purgeSterivexFunction(),
            "purgeContainerFunction": lambda : purgeContainerFunction(),
            "purgePipesFunction": lambda : purgePipesFunction(),
            "fillContainerFunction": lambda : fillContainerFunction(),
            "sampleFunction": lambda : sampleFunction(depth),
            "rollingUp": lambda : rollingFunction('Up'),
            "rollingDown": lambda : rollingFunction('Down'),
            "stopRolling": lambda : stopRolling()
    }
    while True:
        print("listening -------------------------------")
        request = worker.recv_multipart()
        client_id, msg_id, msg = request
        msg_str=msg.decode("utf-8")
        if (msg_str[0:6]=="sample"):
            if len(msg_str)==15:
                depth=int(msg_str[14])
            elif len(msg_str)==16:
                depth=int(msg_str[14:16])
            else:
                print("\nWrong format depth received\n")
            msg_str="sampleFunction"

        # Get the function from switcher dictionary
        func = switcher.get(msg_str, lambda: "Invalid Function")
        
        # Execute the function
        func()

        response = msg + b'(received)'
        worker.send_multipart([client_id, msg_id, response])
        print("[Worker] response sent: ", [response])

        if msg == b"END":
            break

# function definitions from the switcher

def reloadManifoldFunction():
    msg="reloadManifoldFunction\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)

def purgeSterivexFunction():
    msg="purgeSterivexFunction\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)

def purgeContainerFunction():
    msg="purgeContainerFunction\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)

def purgePipesFunction():
    msg="purgePipesFunction\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)

def fillContainerFunction():
    msg="fillContainerFunction\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)

def sampleFunction(depth):
    msg="sampleFunction"+str(depth)
    communicate_arduino(msg)
    print("[Worker] Message sent to arduino: "+msg)
    current_date=datetime.now()
    date_txt=current_date.strftime('%d/%m/%Y %H:%M ')
    sample_txt=date_txt+str(depth)
    add_sample_to_file(PATH_EXECUTED_SAMPLE, sample_txt)
    print("[Worker] Sample saved")

def rollingFunction(direction):
    msg="rollingFunction"+direction+"\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)

def stopRolling():
    msg="stopRolling\n"
    print("[Worker] Message sent to arduino: "+msg)
    communicate_arduino(msg)
    
# ----------------------------

def communicate_arduino(message):
    '''
    string: message

    Emptying the buffer if any message from arduino was there, sending the message and
    expecting only 1 line back from the arduino
    '''
    if arduino.isOpen():
        print("{} connected!".format(arduino.port))
        try:
            time.sleep(0.1)
            while(arduino.inWaiting() > 0): # number of bytes in the receive buffer
                t = arduino.read()
                    
            arduino.write(message.encode()) # sending message, encode default UTF-8
            
            while arduino.inWaiting()==0:
                pass
            if  arduino.inWaiting()>0: 
                answer=arduino.readline()   # up until \n
                print("[Arduino]: received "+str(answer.decode()))
                arduino.flushInput() #remove data after reading
                # ! deprieciated 3.0: reset_input_buffer() new function

        except KeyboardInterrupt:
            print("KeyboardInterrupt has been caught.")


def main_loop_automatic_sampling():
    while True: 
        time.sleep(10) # TODO 60s!
        samples=read_samples_from_file(PATH_SAMPLE_FILE)
        
        # Current date time in local system
        current_date=datetime.now()
        for sample in samples:
            if not is_invalide_sample_input(sample):
                date_sample=sample.split()
                sample_date = datetime.strptime(date_sample[0]+" "+date_sample[1], '%d/%m/%y %H:%M')
                if sample_date<current_date:
                    print("[Worker] Planned sample sent")
                    sampleFunction(date_sample[2])
                    samples.remove(sample)
                    update_file(PATH_SAMPLE_FILE, samples)
                    break

            else:
                samples.remove(sample)
            time.sleep(1)
            
        
if __name__ == "__main__":
    context = zmq.Context.instance()
    worker = context.socket(zmq.ROUTER)
    worker.bind(ENDPOINT)
    configSerialPort()
    
    # init date and time
    datetime.now(timezone.utc)
    datetime(2021, 1, 26, 15, 43, tzinfo=timezone.utc)
    
    # init multi thread
    thread = threading.Thread(target=thread_function, args=(worker,))
    thread.start()
    
    main_loop_automatic_sampling()