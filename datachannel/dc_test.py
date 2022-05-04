#!/usr/bin/env python3

import argparse
import asyncio
import logging
import time

from aiortc import RTCIceCandidate, RTCPeerConnection, RTCSessionDescription
from aiortc.contrib.signaling import BYE, add_signaling_arguments, create_signaling


def start_client(host, port):

    print("start client {} on {}".format(host, port))

def start_server(host, port):
    print("start server {} on {}".format(host, port))


def check_channel(args):
    print("check channel")
    signaling = create_signaling(args)
    pc = RTCPeerConnection()


    # run event loop
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete()
    except KeyboardInterrupt:
        pass
    finally:
        pass

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


        if args.type == "client":
            start_client(args.host, int(args.port))
        elif args.type == "server":
            start_server(args.host, int(args.port))
        else:
            print("unknown command %s" % args.command)
            check_channel(args)