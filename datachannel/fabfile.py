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
DEFAULT_PORT  = 5004
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
def datachannel_server(c, host="localhost", port=DEFAULT_PORT, ce_file = "localhost.pem"):
    """ start data channel server """
    cmd = "./dc_test.py -t server --host {} --port {} --ce={}".format(host, port, ce_file)
    run_cmd(c, cmd)

@task(hosts=DEFAULT_HOSTS)
def datachannel_client(c, host="localhost", port=DEFAULT_PORT, ce_file = "localhost.pem"):
    """ start data channel client """
    cmd = "./dc_test.py -t client --host {} --port {} --ce={}".format(host, port, ce_file)
    run_cmd(c, cmd)


@task(hosts=DEFAULT_HOSTS)
def dtls_server_openssl(c, port=DEFAULT_PORT, cert_file = "cert.pem", key_file = "key.pem"):
    """ start DTLS server """
    cmd = "openssl s_server -dtls1_2 -cert {} -key {} -accept {}".format(cert_file, key_file, port)
    run_cmd(c, cmd)

@task(hosts=DEFAULT_HOSTS)
def dtls_client_openssl(c, host="127.0.0.1", port=DEFAULT_PORT, cert_file = "cert.pem", key_file = "key.pem"):
    """ start DTLS client """
    cmd = "openssl s_client -dtls1_2 -connect {}:{} -debug -cert {} -key {}".format(host, port, cert_file, key_file, port)
    run_cmd(c, cmd)


@task(hosts=DEFAULT_HOSTS)
def dtls_server_psk(c, port=DEFAULT_PORT, psk = "abcd1234"):
    """ start DTLS server with PSK """
    cmd = "openssl s_server -dtls1_2 -accept {} -nocert -psk {} -cipher PSK-AES128-CCM8".format(port, psk)
    run_cmd(c, cmd)

@task(hosts=DEFAULT_HOSTS)
def dtls_client_psk(c, host="localhost", port=DEFAULT_PORT, psk = "abcd1234"):
    """ start DTLS client with PSDK """
    cmd = "openssl s_client -dtls1_2 -connect {}:{} -psk {} -cipher PSK-AES128-CCM8".format(host, port, psk)
    run_cmd(c, cmd)

@task(hosts=DEFAULT_HOSTS)
def tls_client(c, host="localhost", port=DEFAULT_PORT, ca_file = "ca.crt"):
    """ start TLS client """
    cmd = "./tls_test.py -t client --host {} --port {} --ca={}".format(host, port, ca_file)
    run_cmd(c, cmd)

@task(hosts=DEFAULT_HOSTS)
def tls_server(c, host="localhost", port=DEFAULT_PORT, ce_file = "localhost.pem"):
    """ start TLS server """
    cmd = "./tls_test.py -t server --host {} --port {} --ce={}".format(host, port, ce_file)
    run_cmd(c, cmd)