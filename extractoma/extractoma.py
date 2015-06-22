import zlib
import sys
import struct
import os

# unpacks OMA archives (there's two in the Browser app: initial.oma and preload.oma)
# OMA is zlib-compressed data at front, and short footer at end; last value points to file table in decompressed data
# decompressed file table entries are (??, nameoffset, offset, length)

def u32(buf, off):
	return struct.unpack(">i", buf[off:off+4])[0]

def u32l(buf, off):
	return struct.unpack("<i", buf[off:off+4])[0]

def bufstr(buf, off):
	return buf[off:buf.find(b"\x00", off)]

with open(sys.argv[1], "rb") as infile:
	indataraw = infile.read()
indata = zlib.decompress(indataraw)
descoff = u32l(indataraw, len(indataraw)-4)
desccount = u32(indata, descoff)
files = []
descp = descoff + 4
for i in range(desccount):
	file = struct.unpack(">iiii", indata[descp:descp+16])
	descp += 16
	files.append(file)

namestart = descp

for file in files:
	name = str(bufstr(indata,namestart + file[1]), 'utf-8')
	outpath = os.path.join(sys.argv[2], name)
	mydir = os.path.dirname(outpath)
	if not os.path.exists(mydir):
		os.makedirs(mydir)
	with open(outpath, "wb") as outfile:
		outfile.write(indata[file[2]:file[2]+file[3]])