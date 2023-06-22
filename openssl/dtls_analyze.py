#!/usr/bin/env python3
import pyshark
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
from tabulate import tabulate
import argparse
from datetime import datetime


dtls_content_types = {
    20: "change_cipher_spec",
    21: "alert",
    22: "handshake",
    23: "application_data"
}

dtls_handshake_types = {
    0: "hello_request",
    1: "client_hello",
    2: "server_hello",
    3: "hello_verify_request",
    4: "new_session_ticket",
    11: "certificate",
    12: "server_key_exchange",
    13: "certificate_request",
    14: "server_hello_done",
    15: "certificate_verify",
    16: "client_key_exchange",
    20: "finished"
}

def get_description(contentType):
    return dtls_content_types.get(contentType, "unknown")

"""
enum {
        client_hello(1),
        server_hello(2),
        new_session_ticket(4),
        end_of_early_data(5),
        encrypted_extensions(8),
        request_connection_id(9),           /* New */
        new_connection_id(10),              /* New */
        certificate(11),
        certificate_request(13),
        certificate_verify(15),
        finished(20),
        key_update(24),
        message_hash(254),
        (255)
    } HandshakeType;
"""

def get_handshake_type(handshakeType):
    return dtls_handshake_types.get(handshakeType, "unknown")

class DtlsAnalyzer:

    def __init__(self, input_file, config_file):
        self._pcap_file = input_file
        self._config_file = config_file

    def read_pcap(self, display_filter, count, debug=False):

        dataList = []
        packets = pyshark.FileCapture(self._pcap_file, display_filter=display_filter)
        i = 0
        for packet in packets:
            dataItem = {}
            dataItem["highest_layer"] = packet.highest_layer
            if str(packet.highest_layer) != "DTLS":
                continue

            dataItem["ip.src"] = packet.ip.src
            dataItem["ip.dst"] = packet.ip.dst

            if str(packet.transport_layer) == "UDP":
                dataItem["packet_size"] = int(packet.udp.length)
                dataItem["udp.srcport"] = packet.udp.srcport
                dataItem["udp.dstport"] = packet.udp.dstport


            dataItem["dtls.record_content_type"] = int(packet.dtls.record_content_type)
            if dataItem["dtls.record_content_type"] == 22:
                dataItem["dtls.handshake_type"] = packet.dtls.handshake_type
                dataItem["dtls.handshake_type_str"] = get_handshake_type(int(packet.dtls.handshake_type))
                if debug:
                    print("{},{}: {}".format(i,dataItem["dtls.handshake_type_str"], packet.dtls))
            dataItem["dtls"] = packet.dtls

            dataList.append(dataItem)

            i += 1
            if i >= count:
                break

        dataFrame = pd.DataFrame(dataList)

        return dataFrame

    def print_table(self, df, filter=None):
        if filter:
            print(tabulate(df[filter], headers='keys', tablefmt='github'))
        else:
            print(tabulate(df, headers='keys', tablefmt='github'))

    def read_dtls_per_ports(self, df):
        udp_ports = set()
        i = 0
        for udp_port in pd.unique(df["udp.srcport"]):
            udp_ports.add(udp_port)
            if int(udp_port) == 5004:
                continue
            i = i + 1
            print("# {}. {} udp port={} {}".format(i, "-"* 20, udp_port, "-"* 20))
            filter1 = (df["udp.srcport"] == udp_port) | (df["udp.dstport"] == udp_port)
            filter2 = df["dtls.record_content_type"] == 22
            #print(df[df["dtls.record_content_type"] == 22])
            print(df[filter1 & filter2])

    def draw_chart(self, chart_file, df, x, y):
        plt.style.use('seaborn-v0_8-whitegrid')

        fig = plt.figure(figsize=(36, 18))
        font = {'size': 16}

        plt.plot(x, y, data=df)
        #plt.show()
        fig.savefig(chart_file)
        plt.close()

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', action='store', dest='input_file', help='specify input file')
    parser.add_argument('-o', action='store', dest='config_file', default="config.yml", help='specify config file')
    parser.add_argument('-f', action='store', dest='filter', default="dtls", help='specify filter expression')
    parser.add_argument('-c', action='store', dest='count', default=10, help='specify packet count')
    args = parser.parse_args()

    if not args.input_file:

        print("usage: ./dtls_analyze.py -i <pcap_file> -f <filter_expression>")
        print('such as: ./dtls_analyze.py -i /Users/yafan/Downloads/dtls_records_1.pcapng -f "dtls and ip.host==173.37.55.171" -c 1000')
        exit(0)

    dtlsAnalyzer = DtlsAnalyzer(args.input_file, args.config_file)

    df = dtlsAnalyzer.read_pcap(args.filter, int(args.count), True)
    if args.filter:
        dtlsAnalyzer.read_dtls_per_ports(df)
    else:
        dtlsAnalyzer.print_table(df)
