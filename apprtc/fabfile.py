from asyncio import tasks
from fabric.api import *
from fabric.context_managers import *
from fabric.contrib.console import confirm
from datetime import date
from sys import platform
import os, subprocess
import psutil
import platform


BASE_PATH = os.path.dirname(__file__)
appserver_ports = {}

"""
Linux: Linux
Mac: Darwin
Windows: Windows
"""
@task
def test():

    os_type = platform.system()
    print(os_type)

@task
def start_server(local_ip=None):

    if not local_ip:
        cmd0 = "ifconfig en0 | grep inet | awk '$1==\"inet\" {print $2}'"

        stdoutput = subprocess.check_output(cmd0, shell=True)
        local_ip = stdoutput.decode('utf-8')
        print(local_ip)


    os_type = platform.system()
    if os_type == 'Darwin':
        cmd1 = """
            docker run --rm \
            -p 8080:8080 -p 8089:8089 -p 3478:3478 -p 3478:3478/udp -p 3033:3033 \
            -p 59000-65000:59000-65000/udp \
            -e PUBLIC_IP=%s \
            -it piasy/apprtc-server
        """
        local(cmd1.strip() % local_ip.strip())
    elif os_type == 'Linux':
        cmd1 = "docker run --rm --net=host \
                -e PUBLIC_IP=%s \
                -it piasy/apprtc-server"
        print(cmd1.strip() % local_ip.strip())