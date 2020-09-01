import sys
import time
import zlib
import copy
import struct
import binascii
import hashlib
import argparse
import math
import zipfile, tempfile
import json
import re
import os

def open_terminal(_port="COM13", reset=1):
    control_signal = '0' if reset else '1'
    control_signal_b = not reset
    import serial.tools.miniterm
    # For using the terminal with MaixPy the 'filter' option must be set to 'direct'
    # because some control characters are emited
    sys.argv = [sys.argv[0], _port, '115200', '--dtr='+control_signal, '--rts='+control_signal,  '--filter=direct']
    serial.tools.miniterm.main(default_port=_port, default_baudrate=115200, default_dtr=control_signal_b, default_rts=control_signal_b)
    sys.exit(0)

if __name__ == "__main__":
    open_terminal()