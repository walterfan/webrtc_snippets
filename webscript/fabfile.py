import os
import sys
from fabric import task
from fabric import Connection
import time
import json
import logging

from datetime import date
from sys import platform
import os, subprocess

DEFAULT_HOSTS = ["localhost"]
BASE_PATH = os.path.dirname(__file__)

SRC_FOLDER = "src"
SRC_PATH = BASE_PATH + "/" + SRC_FOLDER

TEST_FOLDER = "test"
TEST_PATH = BASE_PATH + "/" + TEST_FOLDER

@task(hosts=DEFAULT_HOSTS)
def usage(c):
    print("usage: fab -l")
    print("installation: npm install -g typescript")

@task(hosts=DEFAULT_HOSTS)
def test(c):
    output = c.local("which tsc")
    stdoutput = subprocess.check_output("which tsc", shell=True)
    lines = stdoutput.decode('utf-8')

    print(lines)
    #c.local("tsc test/hello --noEmitOnError true")