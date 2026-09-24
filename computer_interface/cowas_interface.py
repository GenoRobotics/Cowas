#!/usr/bin/env python

from PyQt5.QtCore import QSocketNotifier, QDateTime, Qt, QTimer, pyqtSignal, QObject
from PyQt5.QtWidgets import (QPlainTextEdit, QMessageBox, QInputDialog, QListWidget, QApplication, QCheckBox, QComboBox, QDateTimeEdit,
        QDial, QDialog, QFileDialog, QGridLayout, QGroupBox, QHBoxLayout, QLabel, QLineEdit,
        QProgressBar, QPushButton, QRadioButton, QScrollBar, QSizePolicy,
        QSlider, QSpinBox, QStyleFactory, QTableWidget, QTabWidget, QTextEdit,
        QVBoxLayout, QWidget)

import zmq
import sys
import os
import uuid
import csv
from datetime import datetime

MAXDEPTH=45
PATH_SAMPLE_FILE='Samples_planned'
ENDPOINT = "ipc://routing.ipc"

class Client():
    def __init__(self):
        context = zmq.Context.instance()
        client = context.socket(zmq.DEALER)
        client.setsockopt(zmq.IDENTITY, b'QtClient')
        client.connect(ENDPOINT)

        self.socket = client

    def dispatch(self, msg):
        msg = bytes(msg, 'utf-8')
        uid = uuid.uuid4().bytes
        self.socket.send_multipart([uid, msg])
        return uid

    def recv(self):
        return self.socket.recv_multipart()


class Interface(QDialog):
    def __init__(self, parent=None):
        super(Interface, self).__init__(parent)

        # init boxes of the interfaces
        self.functionSelection("None")
        self.createCowasFunctionSelector()
        self.createDeploymentController()
        self.createSampleSchedule()
        
        # init samples saved
        self.read_samples_from_file(PATH_SAMPLE_FILE)


        # init layout of the boxes
        mainLayout = QGridLayout()
        mainLayout.addWidget(self.cowasFunctionSelector, 1, 0)
        mainLayout.addWidget(self.deployment, 2, 0)
        mainLayout.addWidget(self.sampleScheduler, 3, 0)
		
        mainLayout.setRowStretch(1, 1)
        mainLayout.setRowStretch(2, 1)
        mainLayout.setColumnStretch(0, 1)
        mainLayout.setColumnStretch(1, 1)
        self.setLayout(mainLayout)

        self.setWindowTitle("COWAS Control Interface")


        # Communication with main_loop_rasberry
        self._client = Client()
        socket = self._client.socket
        self._notifier = QSocketNotifier(socket.getsockopt(zmq.FD),
                                         QSocketNotifier.Read, self)
        self._notifier.activated.connect(self._socket_activity)

        # Update the list of samples
        self.timer=QTimer()
        self.timer.timeout.connect(self.update_widgetList)
        self.timer.start(1000) # in ms

    def update_widgetList(self):
        current_date=datetime.now()
        for row in range(self.listWidget.count()):
            if self.listWidget.item(row) != None :
                date_sample=self.listWidget.item(row).text().split()
                sample_date = datetime.strptime(date_sample[0]+" "+date_sample[1], '%d/%m/%y %H:%M')
                if sample_date<current_date:
                    self.listWidget.takeItem(row)


    def _socket_activity(self):
        self._notifier.setEnabled(False)
        flags = self._client.socket.getsockopt(zmq.EVENTS)

        if flags & zmq.POLLIN:
            received = self._client.recv()
            msg_id, msg = received
            if msg != b'rollingUp(received)' and  msg != b'rollingDown(received)':
                self.upRollButton.setEnabled(True)
                self.downRollButton.setEnabled(True)
                self.activateFunctionButton.setEnabled(True)
            print("[Socket] event received: " + repr(received))
        elif flags & zmq.POLLOUT:
            print("[Socket] event sent")
        elif flags & zmq.POLLERR:
            print("[Socket] zmq.POLLERR")
        else:
            print("[Socket] FAILURE")

        self._notifier.setEnabled(True)

        flags = self._client.socket.getsockopt(zmq.EVENTS)

    def send_data(self, msg):
        # Diseable other buttons
        if msg != 'stopRolling':
            if msg != 'rollingUp':
                self.upRollButton.setEnabled(False)
            if msg != 'rollingDown':
                self.downRollButton.setEnabled(False)
            self.activateFunctionButton.setEnabled(False)
        self._client.dispatch(msg)
        print("[UI] sent: " + msg)

    def noneFunction(self):
        print("None")
    
    def reloadManifoldFunction(self):
        self.send_data("reloadManifoldFunction")

# TODO : modify to also send the manifold slot of the sterivex to purge
#    def purgeSterivexFunction(self):
#        self.send_data("purgeSterivexFunction")

    def purgeContainerFunction(self):
        self.send_data("purgeContainerFunction")

    def purgePipesFunction(self):
        self.send_data("purgePipesFunction")

    def fillContainerFunction(self):
        self.send_data("fillContainerFunction")

    def sampleFunction(self):
        msgbox = QMessageBox()
        text, ok = QInputDialog.getText(self, 'Depth selection', 'Enter depth [m] and confirm ok :')
        
        if ok:
            if text.isdigit():
                if int(text)>0 and int(text)<45:
                    self.send_data("sampleFunction"+text)
                else:
                    msg = QMessageBox()
                    msg.setIcon(QMessageBox.Critical)
                    msg.setText("Error format depth")
                    msg.setInformativeText('Invalide depth input\n Depth should be between 0 and 45 [m]')
                    msg.setWindowTitle("Error")
                    msg.exec_()
            else:
                msg = QMessageBox()
                msg.setIcon(QMessageBox.Critical)
                msg.setText("Error format depth")
                msg.setInformativeText('Invalide depth input\n Depth should be between 0 and 45 [m]')
                msg.setWindowTitle("Error")
                msg.exec_()

    def pushButtonFunctionExecution(self):
        switcher ={
            "None": self.noneFunction,
            "Reload Manifold": self.reloadManifoldFunction,
#            "Purge Sterivex": self.purgeSterivexFunction, 
            "Purge Container": self.purgeContainerFunction,
            "Purge Pipes": self.purgePipesFunction,
            "Fill container": self.fillContainerFunction,
            "Sample": self.sampleFunction,
        }
        # Get the function from switcher dictionary
        func = switcher.get(self.functionSelected, lambda: "Invalid Function")
        # Execute the function
        func()
        

    def functionSelection(self, functionType):
        self.functionSelected=functionType

    def createCowasFunctionSelector(self):
        self.cowasFunctionSelector = QGroupBox("Control")

        # ComboBox
        functionComboBox = QComboBox()
        functionComboBox.addItem("None")
        functionComboBox.addItem("Reload Manifold")
#        functionComboBox.addItem("Purge Sterivex")
        functionComboBox.addItem("Purge Container")
        functionComboBox.addItem("Purge Pipes")
        functionComboBox.addItem("Fill container")
        functionComboBox.addItem("Sample")
        functionLabel = QLabel("&Function:")
        functionLabel.setBuddy(functionComboBox)
        functionComboBox.activated[str].connect(self.functionSelection)

        # Button
        self.activateFunctionButton = QPushButton("Execute")
        self.activateFunctionButton.setDefault(False)
        self.activateFunctionButton.clicked.connect(self.pushButtonFunctionExecution)

        functionSelectorLayout = QHBoxLayout()
        functionSelectorLayout.addWidget(functionLabel)
        functionSelectorLayout.addWidget(functionComboBox)
        functionSelectorLayout.addStretch(3)
        functionSelectorLayout.addWidget(self.activateFunctionButton)

        self.cowasFunctionSelector.setLayout(functionSelectorLayout)

    def upRollFunction(self):
        # if button is checked
        if self.upRollButton.isChecked():
            print("Rolling up")
            self.downRollButton.setEnabled(False)
            self.activateFunctionButton.setEnabled(False)
            self.send_data("rollingUp")

        # if it is unchecked
        else:
            self.downRollButton.setEnabled(True)
            self.activateFunctionButton.setEnabled(True)
            self.send_data("stopRolling")


    def downRollFunction(self):
        # if button is checked
        if self.downRollButton.isChecked():
            print("Rolling down")
            self.upRollButton.setEnabled(False)
            self.activateFunctionButton.setEnabled(False)
            self.send_data("rollingDown")


        # if it is unchecked
        else:
            self.upRollButton.setEnabled(True)
            self.activateFunctionButton.setEnabled(True)
            self.send_data("stopRolling")

    def createDeploymentController(self):
        self.deployment = QGroupBox("Deployment")

        # Button
        self.upRollButton = QPushButton("Up")
        self.upRollButton.setCheckable(True)
        self.upRollButton.setChecked(False)
        self.upRollButton.clicked.connect(self.upRollFunction)

        self.downRollButton = QPushButton("Down")
        self.downRollButton.setCheckable(True)
        self.downRollButton.setChecked(False)
        self.downRollButton.clicked.connect(self.downRollFunction)

        deploymentLabel = QLabel("&Pipes:")
        deploymentLabel.setBuddy(self.upRollButton)

        deploymentLayout = QHBoxLayout()
        deploymentLayout.addWidget(deploymentLabel)
        deploymentLayout.addWidget(self.upRollButton)
        deploymentLayout.addStretch(1)
        deploymentLayout.addWidget(self.downRollButton)

        self.deployment.setLayout(deploymentLayout)

    def is_invalide_sample_input(self, text, error_msg_display=True):
        wrong_format=False
        x = text.split()
        if len(x) !=3 :
            wrong_format=True
            error_msg='input'
        elif len(x[0].split('/'))!=3:
            error_msg='input'
            wrong_format=True
        elif len(x[1].split(':'))!=2:
            error_msg='input'
            wrong_format=True
        elif x[2].isdigit():
            date=x[0].split('/')
            time=x[1].split(':')
            if not date[0].isdigit() or not date[1].isdigit() or not date[2].isdigit():
                error_msg='date'
                wrong_format=True
            elif not time[0].isdigit() or not time[1].isdigit():
                error_msg='time'
                wrong_format=True
            elif (int(time[1])<0 or int(time[1])>59):
                error_msg='minutes'
                wrong_format=True  
            elif (int(time[0])<0 or int(time[0])>24):
                error_msg='hour'
                wrong_format=True
            elif (int(date[0])<0 or int(date[0])>31):
                error_msg='day'
                wrong_format=True
            elif (int(date[1])<0 or int(date[1])>12):
                error_msg='month'
                wrong_format=True
            elif (int(date[2])<0 or int(date[2])>99):
                error_msg='year'
                wrong_format=True
            
            # Verify if the date is valide
            try:
                current_date=datetime.now()
                sample_date = datetime.strptime(x[0]+" "+x[1], '%d/%m/%y %H:%M')
                if sample_date<current_date:
                    error_msg='Past date'
                    wrong_format=True
                    
            except ValueError:
                error_msg='Invalide date'
                wrong_format=True
                print("Invalide input date and time")

            depth = int(x[2])
            if (depth<0 or depth>MAXDEPTH):
                error_msg='depth'
                wrong_format=True

        else:
            error_msg='input'
            wrong_format=True

        if wrong_format and error_msg_display:
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("Error format sample")
            msg.setInformativeText('A sample should be written as : \ndate time depth[m] frequency\n\n(example: 25/07/22 14:35 25 1)\n\nInvalid '+error_msg)
            msg.setWindowTitle("Error")
            msg.exec_()

        return wrong_format

    def add_space_display(self, sample_txt):
        element=sample_txt.split()
        sample_txt=element[0]+"    "+element[1]+"       "+element[2]
        return sample_txt

    def rmv_space_display(self, sample_txt):
        element=sample_txt.split()
        sample_txt=element[0]+" "+element[1]+" "+element[2]
        return sample_txt

    def add(self, sample_txt='None', error_msg_display=True, init=False):
        if sample_txt == 'None' or sample_txt==False:
            sample_txt=self.txtText.text()
        if not self.is_invalide_sample_input(text=sample_txt, error_msg_display=error_msg_display):
            displayed_sample=self.add_space_display(sample_txt)
            self.listWidget.addItem(displayed_sample)
            if not init:
                self.add_sample_to_file(sample_txt)
        
    def importCsv(self):
        fname = QFileDialog.getOpenFileName(self, 'Open file', '/home/cowas/Desktop')

        if fname[0]:
            try:
                f = open(fname[0], mode='r', encoding='utf-8-sig')
                with f:
                    for i, row in enumerate(csv.reader(f)):
                        sample=row[0].split(';')
                        sample_txt = str(sample[0])+' '+str(sample[1])+' '+str(sample[2])
                        self.add(sample_txt, error_msg_display=False)
            except:
                print("Wrong format file")
         
    def edit(self):
        self.row = self.listWidget.currentRow()
        if self.row == -1:
            self.msg, self.ok = QInputDialog.getText(self, "Input", "Please modify a value", QLineEdit.Normal)
        else:
            self.msg, self.ok = QInputDialog.getText(self, "Input", "Please modify a value", QLineEdit.Normal, self.rmv_space_display(self.listWidget.item(self.row).text()))

        if self.ok and not self.is_invalide_sample_input(text=self.msg, error_msg_display=True):
            # Edit item in listWidget
            self.listWidget.takeItem(self.row)
            displayed_sample=self.add_space_display(self.msg)
            self.listWidget.insertItem(self.row, displayed_sample)

            # Edit item in Sample.txt
            self.update_file()

    def delete(self):
        self.row = self.listWidget.currentRow()
        self.listWidget.takeItem(self.row)
        # Edit item in Sample.txt
        self.update_file()

    def deleteAll(self):
        self.listWidget.clear()
        # Edit item in Sample.txt
        self.update_file()

    def createSampleSchedule(self):
        self.sampleScheduler = QGroupBox("Sample scheduler")

        self.example = QLabel(" ")
        self.example.setObjectName("example")
        self.example_sample = QLabel("     Date         Time      Depth")
        self.example_sample.setObjectName("label")


        self.gridLayout = QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        
        self.label = QLabel("Sample ")
        self.label.setObjectName("label")
        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
        
        self.txtText = QLineEdit("12/08/22 14:35 35")
        self.txtText.setObjectName("txtText")
        self.gridLayout.addWidget(self.txtText, 0, 1, 1, 1)

        self.gridLayout.addWidget(self.example, 1, 0, 1, 1)
        self.gridLayout.addWidget(self.example_sample, 1, 1, 1, 1)

        self.btnAdd = QPushButton("Add")
        self.btnAdd.setObjectName("btnAdd")
        self.gridLayout.addWidget(self.btnAdd, 2, 0, 1, 1)

        self.listWidget = QListWidget()
        self.listWidget.setObjectName("listWidget")
        self.gridLayout.addWidget(self.listWidget, 2, 1, 5, 1)
        
        self.btnImportCsv = QPushButton("Import csv")
        self.btnImportCsv.setObjectName("btnImportCsv")
        self.gridLayout.addWidget(self.btnImportCsv, 3, 0, 1, 1)
        
        self.btnEdit = QPushButton("Edit")
        self.btnEdit.setObjectName("btnEdit")
        self.gridLayout.addWidget(self.btnEdit, 4, 0, 1, 1)
        
        self.btnDelete = QPushButton("Delete")
        self.btnDelete.setObjectName("btnDelete")
        self.gridLayout.addWidget(self.btnDelete, 5, 0, 1, 1)
        
        self.btnDeleteAll = QPushButton("Delete all")
        self.btnDeleteAll.setObjectName("btnDeleteAll")
        self.gridLayout.addWidget(self.btnDeleteAll, 6, 0, 1, 1)
        
        schedulerLayout = QHBoxLayout()
        self.sampleScheduler.setLayout(self.gridLayout)

        self.btnAdd.clicked.connect(self.add)
        self.btnImportCsv.clicked.connect(self.importCsv)
        self.btnEdit.clicked.connect(self.edit)
        self.btnDelete.clicked.connect(self.delete)
        self.btnDeleteAll.clicked.connect(self.deleteAll)
		
    def read_samples_from_file(self, file_path):
        with open(file_path) as f:
            lines = f.readlines()

        for line in lines:
            if line[-1]=='\n':
                line_strip = line[:-1]
            else:
                line_strip=line
            self.add(sample_txt=line_strip, error_msg_display=False, init=True)
        f.close()

    def  add_sample_to_file(self, sample_txt):
        # Open the file in append & read mode ('a+')
        with open(PATH_SAMPLE_FILE, "a+") as file_object:
            # Move read cursor to the start of file.
            file_object.seek(0)
            # If file is not empty then append '\n'
            data = file_object.read(100)
            if len(data) > 0 :
                file_object.write("\n")
            # Append text at the end of file
            file_object.write(sample_txt)
        file_object.close()

    def update_file(self):
        list_of_lines=self.extract_list_listWidget()
        a_file = open(PATH_SAMPLE_FILE, "w")
        a_file.writelines(list_of_lines)
        a_file.close()

    def extract_list_listWidget(self):
        lst =self.listWidget
        items = []
        for x in range(lst.count()):
            items.append(lst.item(x).text()+'\n')
        return items

if __name__ == '__main__':
    app = QApplication([])
    gallery = Interface()
    gallery.update()
    gallery.show()
    sys.exit(app.exec())
