import os
import subprocess

def _exe(command):
    os.system("bash -c '" + command + "&'>/dev/null")


def pipe(command, kind='str'):
    if kind == 'str':
        return subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.readlines()[
                   0][1:-2]
    else:
        return subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.readlines()

def data_filter(data,keys):
    data_dup = {}
    for d in data.iterkeys():
        if d not in keys:
            data_dup[d]=data[d]

    return data_dup
