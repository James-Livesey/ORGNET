import struct
import argparse
import os

def convert(inpath, outpath):
    data = open(inpath, "rb").read() + b"FFFF"
    result = b"OPK" + struct.pack(">I", len(data))[1:] + data

    open(outpath, "wb").write(result)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Convert a raw datapack binary file to the OPK format")

    parser.add_argument("infile", help = "Raw binary file path")
    parser.add_argument("outfile", help = "OPK file path")

    args = parser.parse_args()

    convert(args.infile, args.outfile)