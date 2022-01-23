from asyncio import tasks
from fabric.api import *
from fabric.context_managers import *
from fabric.contrib.console import confirm
from datetime import date
from sys import platform
import os, subprocess


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
def install_deps():
    cmd = "apt install -y libmicrohttpd-dev libjansson-dev \
    libssl-dev libsrtp-dev libsofia-sip-ua-dev libglib2.0-dev \
    libopus-dev libogg-dev libcurl4-openssl-dev liblua5.3-dev \
    libconfig-dev pkg-config gengetopt libtool automake"
    sudo(cmd)