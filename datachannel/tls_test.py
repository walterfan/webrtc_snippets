#!/usr/bin/env python3

import os
import platform
import shutil
import argparse

from datetime import datetime, timedelta
import socket, ssl

DEFAULT_CA_FILE = "ca.crt"
DEFAULT_CERT_FILE = "localhost.pem"

def check_tls_version(hostname='www.fanyamin.com'):

    context = ssl.create_default_context()

    with socket.create_connection((hostname, 443)) as sock:
        with context.wrap_socket(sock, server_hostname=hostname) as ssock:
            print(ssock.version())
            

def start_client(host, port, cafile=None):

    purpose = ssl.Purpose.SERVER_AUTH
    context = ssl.create_default_context(purpose, cafile=cafile)

    raw_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    raw_sock.connect((host, port))
    print('Connected to host {!r} and port {}'.format(host, port))
    ssl_sock = context.wrap_socket(raw_sock, server_hostname=host)

    while True:
        data = ssl_sock.recv(1024)
        if not data:
            break
        print(repr(data))
        
def start_server(host, port, certfile, cafile=None):
    print("start server...")
    purpose = ssl.Purpose.CLIENT_AUTH
    context = ssl.create_default_context(purpose, cafile=cafile)
    context.load_cert_chain(certfile)

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((host, port))
    listener.listen(1)
    print('Listening at interface {!r} and port {}'.format(host, port))
    raw_sock, address = listener.accept()
    print('Connection from host {!r} and port {}'.format(*address))
    ssl_sock = context.wrap_socket(raw_sock, server_side=True)

    ssl_sock.sendall('Practice makes perfect.'.encode('ascii'))
    ssl_sock.close()


if __name__ == '__main__':

    parser = argparse.ArgumentParser()

    parser.add_argument('-t', action='store', dest='type', help='client|server')
    parser.add_argument('--host', action='store', dest='host', help='specify hostname')
    parser.add_argument('--port', action='store', dest='port', help='specify port')

    parser.add_argument('--ca', action='store', dest='cafile', help='specify CA file')
    parser.add_argument('--ce', action='store', dest='cefile', help='specify Certificate')

    args = parser.parse_args()
    if(not args.type):
        print("Usage: ./tls_test -t <type> -h <port> -p <port>")
    else:

        ca_file = DEFAULT_CA_FILE
        cert_file = DEFAULT_CERT_FILE

        if args.cafile:
            ca_file = args.cafile

        if args.cefile:
            cert_file = args.cefile

        if args.type == "client":
            start_client(args.host, int(args.port), ca_file)
        elif args.type == "server":
            start_server(args.host, int(args.port), cert_file)
        else:
            print("unknown command %s" % args.command)
            check_tls_version()