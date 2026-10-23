#!/usr/bin/env python
# -*- coding: utf-8 -*-
from datetime import datetime

MAXDEPTH=45

class Sample:
    def __init__(self, sample_txt):
        self.sample_txt=sample_txt
        self.decompose_date_time()

    def new_sample(self, sample_txt):
        self.sample_txt=sample_txt
        self.decompose_date_time()

    def decompose_date_time(self):
        x=self.sample_txt.split()
        date=x[0].split('/')
        self.day=int(date[0])
        self.month=int(date[1])
        self.year=int(date[2])
        
        time=x[1].split(':')
        self.hour=int(time[0])
        self.minute=int(time[1])

def read_samples_from_file(file_path):
    with open(file_path) as f:
        lines = f.readlines()

    samples=[]
    for line in lines:
        if line[-1]=='\n':
            samples.append(line[:-1])
        else:
            samples.append(line)
    f.close()
    return samples

def update_file(path, samples):
        write_sample=[]
        for sample in samples:
            if not is_invalide_sample_input(sample):
                write_sample.append(sample+"\n")

        a_file = open(path, "w")
        a_file.writelines(write_sample)
        a_file.close()


def is_invalide_sample_input(sample):
        wrong_format=False
        x = sample.split()
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

        return wrong_format
    
    
def  add_sample_to_file(path, sample_txt):
    # Open the file in append & read mode ('a+')
    with open(path, "a+") as file_object:
        # Move read cursor to the start of file.
        file_object.seek(0)
        # If file is not empty then append '\n'
        data = file_object.read(100)
        if len(data) > 0 :
            file_object.write("\n")
        # Append text at the end of file
        file_object.write(sample_txt)
    file_object.close()
