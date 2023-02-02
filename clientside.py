#!/usr/bin/python
# -*- coding: utf-8 -*-
import argparse
import math
import socket

# Getting the arguments:

parser = argparse.ArgumentParser(prog='Linux Memory Extractor')
parser.add_argument('host')
parser.add_argument('port')
parser.add_argument('filename')
args = parser.parse_args()

file = open(args.filename, 'rb')
fd = file.read()
file.close()

s = socket.socket()
s.connect((args.host, int(args.port)))

# Can be proven using the pigeonhole principle
P_NUM = math.ceil(len(fd) / 65535)

for i in range(P_NUM):
    pd = fd[65535 * i:(i + 1) * 65535]
    pd_len = min(65535, len(fd) - 65535 * i)
    # Flagging the last packet as the last:
    last_flag = bytes(chr(int(i + 1 == P_NUM)*256), encoding='ascii')

    packet = pd_len.to_bytes(2, 'little') + last_flag + pd

    print(f'Packet number {i+1}/{P_NUM} (Length = {int(pd_len)})')
    s.sendall(packet)

print('All packets sent successfully')