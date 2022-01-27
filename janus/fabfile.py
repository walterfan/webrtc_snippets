from asyncio import tasks
from fabric.api import *
from fabric.context_managers import *
from fabric.contrib.console import confirm
from datetime import date
from sys import platform
import os, subprocess

LOCAL_SRC_DIR = "~/Documents/workspace/webrtc/janus-gateway"
REMOTE_SRC_DIR = " /home/walter/webrtc/janus-gateway" 

env.hosts = ['10.224.112.73']
env.user = 'ubuntu'
env.key_filename = '~/Documents/workspace/walter/wfnote/backup/wbxivr.pem'

@task
def remote_version():
    run('cat /etc/issue')

@task
def remote_uname():
    run('uname -a')

@task
def update_mirrors(old_host, new_host):
    cmd = "sed -i -e 's/http:\/\/%s/http:\/\/%s/' /etc/apt/sources.list" % (old_host, new_host)
    sudo(cmd)
    sudo("apt-get update")

@task
def changelist():
    stdoutput = subprocess.check_output("cd %s && git status -s -uno" % LOCAL_SRC_DIR, shell=True)
    git_changed_files = stdoutput.decode('utf-8')

    file_list=[]
    i = 0
    for git_changed_file in git_changed_files.split('\n'):
        filenames = git_changed_file.strip().split()
        nameLen = len(filenames)
        if nameLen > 1:
            i = i + 1

            if filenames[0] == 'D' or filenames[0] == 'R':
                continue
            print("%d %s: %s" % (i, filenames[0], filenames[nameLen - 1]))
            file_list.append(filenames[nameLen - 1].strip())


    return file_list

@task
def copyfiles():
    """
    - fab copyfiles [-H 10.224.172.32 -u root]
    """
    file_list = changelist()

    if(len(file_list) == 0):
        file_list = tmp_file_list


    for filename in file_list:
        local_file  = LOCAL_SRC_DIR  + "/" + filename
        remote_file = REMOTE_SRC_DIR + "/" + filename

        #print("will copy ", local_file, " to ", remote_file)
    
        with settings(warn_only=True):    #when upload error,continue
            result = put(local_file, remote_file, use_sudo=True)

        if result.failed and not confirm("put file failed,Continue[Y/N]?"):
            abort("Aborting file put task!")


@task
def install_deps():
    cmd = "apt install -y libjansson-dev \
    libssl-dev libsrtp-dev libsofia-sip-ua-dev libglib2.0-dev \
    libopus-dev libogg-dev libcurl4-openssl-dev liblua5.3-dev \
    libconfig-dev libnice-dev pkg-config gengetopt libtool automake"
    sudo(cmd)
    print("need install libmicrohttpd-dev libsrtp usrsctp manually")
"""
# Add jitter
sudo tc qdisc add dev eth0 root netem rate 1000mbit delay 0ms 50ms 0%
# Change jitter
sudo tc qdisc change dev eth0 root netem rate 1000mbit delay 0ms 100ms 0%
# Delete jitter
sudo tc qdisc del dev eth0 root netem rate 1000mbit delay 0ms 100ms 0%

"""

@task
def add_delay(ms, ifth="ens3"):
    cmd = "tc qdisc add dev %s root netem delay %sms" % (ifth, ms)
    print(cmd)

@task
def del_delay(ms, ifth="ens3"):
    cmd = "tc qdisc del dev %s root netem delay %sms" % (ifth, ms)
    print(cmd)    

"""
# Add loss
sudo tc qdisc add dev eth0 root netem loss 20%
# Change loss
sudo tc qdisc change dev eth0 root netem loss 30%
# Delete loss
sudo tc qdisc del dev eth0 root netem loss 30%
"""
@task
def add_loss(ratio, ifth="ens3"):
    """
    add loss: 1, 5, 10, 20
    """
    cmd = "tc qdisc add dev {} root netem loss {}%".format(ifth, ratio)
    sudo(cmd)

@task
def change_loss(ratio, ifth="ens3"):
    """
    change loss: 1, 5, 10, 20
    """
    cmd = "tc qdisc change dev {} root netem loss {}%".format(ifth, ratio)
    sudo(cmd)

@task
def del_loss(ratio, ifth="ens3"):
    """
    delete loss
    """
    cmd = "tc qdisc del dev {} root netem loss {}%".format(ifth, ratio)
    sudo(cmd)

@task
def add_rate_limit(bw, ifth="ens3"):
    """
    limit bandwidth: 500k, 1m, 2m
    """
    cmd = "tc qdisc add dev {} root netem rate {}bit".format(ifth,bw)
    sudo(cmd)

@task
def show_tc():
    """
    Show current settings
    """
    sudo("tc qdisc show")

def remove_tc(ifth="ens3"):
    """
    Delete all impairments    
    """
    sudo("tc qdisc del dev {} root".format(ifth))    