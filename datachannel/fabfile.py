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
DEFAULT_ALGORITHM = "RSA"

def run_cmd(c, cmd):
    print("execute: %s" % cmd)
    c.local(cmd)


@task(hosts=DEFAULT_HOSTS)
def generate_cert(c, key_file = "key.pem", cert_file = "cert.pem", csr_file = "server.csr"):
    """ generate key and certificate file"""
    cmd1 = "openssl ecparam -out {} -name prime256v1 -genkey".format(key_file)
    cmd2 = "openssl req -new -sha256 -key {} -out {}".format(key_file, csr_file)
    cmd3 = "openssl x509 -req -sha256 -days 365 -in {} -signkey {} -out {}".format(csr_file, key_file, cert_file)

    if DEFAULT_ALGORITHM == 'RSA':
        cmd0 = 'openssl req \
        -new \
        -newkey rsa:4096 \
        -days 365 \
        -nodes \
        -x509 \
        -subj "/C=CN/ST=AH/L=HF/O=HOME/CN=www.fanyamin.com" \
        -keyout {} \
        -out {}'.format(key_file, cert_file)
    else:
        cmd0 = 'openssl req \
        -new \
        -newkey ec \
        -pkeyopt ec_paramgen_curve:prime256v1 \
        -days 365 \
        -nodes \
        -x509 \
        -subj "/C=CN/ST=AH/L=HF/O=HOME/CN=www.fanyamin.com" \
        -keyout {} \
        -out {}'.format(key_file, cert_file)

    run_cmd(c, cmd0)

@task(hosts=DEFAULT_HOSTS)
def view_cert(c, cert_file = "cert.pem"):
    """ view certificate file"""
    cmd = 'openssl x509 -noout -text -in {}'.format(cert_file)
    run_cmd(c, cmd)



@task(hosts=DEFAULT_HOSTS)
def tls_server(c, port=4444, cert_file = "cert.pem", key_file = "key.pem"):
    """ start TLS server """
    cmd = "openssl s_server -dtls1_2 -cert {} -key {} -accept {}".format(cert_file, key_file, port)
    run_cmd(c, cmd)

@task(hosts=DEFAULT_HOSTS)
def tls_client(c, host="127.0.0.1", port=4444, cert_file = "cert.pem", key_file = "key.pem"):
    """ start TLS client """
    cmd = "openssl s_client -dtls1_2 -connect {}:{} -debug -cert {} -key {}".format(host, port, cert_file, key_file, port)
    run_cmd(c, cmd)

"""
// Use with examples/dial/psk/main.go
  openssl s_server -dtls1_2 -accept 4444 -nocert -psk abc123 -cipher PSK-AES128-CCM8

  // Use with examples/listen/psk/main.go
  openssl s_client -dtls1_2 -connect 127.0.0.1:4444 -psk abc123 -cipher PSK-AES128-CCM8
"""