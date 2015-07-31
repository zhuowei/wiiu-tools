import sys
import zlib
with open(sys.argv[1], "rb") as infile:
	indata = infile.read()

outfile = open(sys.argv[2], "wb")

pos = -1

def u16(a, b):
	return a[b] << 8| a[b+1]

def u16l(a, b):
	return a[b]| a[b+1] << 8
def commonit(a, b):
	if a == None:
		return list(b)
	out = list(a)
	for i in range(len(a)):
		if out[i] == "x":
			continue
		if out[i] != b[i]:
			out[i] = "x"
	return out

def stringify(a):
	ret = ""
	for i in a:
		if i == "x":
			ret += "XX "
		else:
			ret += hex(i|0x100)[3:] + " "
	return ret

def writeu32(a):
	return bytes([(a >> 24) & 0xff, (a >> 16) & 0xff, (a >> 8) & 0xff,
		a & 0xff])

idlist = []
iddict = {}
idlengthdict = {}
idcommon = {}
counttotal = 0
decomplengths = []
while True:
	pos = indata.find(b"\x32\xab\x98\x64", pos+1)
	if pos == -1:
		break
	counttotal += 1
	packetlen = u16(indata, pos+14)
	if packetlen < 2:
		continue
	pid = u16(indata, pos+32)
	if not pid in iddict:
		idlist.append(pid)
		iddict[pid] = 1
		idlengthdict[pid] = [packetlen]
		idcommon[pid] = {}
		idcommon[pid][packetlen] = None 
	else:
		iddict[pid] += 1
		if not packetlen in idlengthdict[pid]:
			idlengthdict[pid].append(packetlen)
			idcommon[pid][packetlen] = None
	idcommon[pid][packetlen] = \
		commonit(idcommon[pid][packetlen], indata[pos+32:pos+32+packetlen])
	if pid == 0x3d3:
		# ZLIB WHY THE F does this use deflated packets
		packetdat = indata[pos+32:pos+32+packetlen]
		thedat = zlib.decompress(packetdat[12:])
		outfile.write(writeu32(len(thedat)))
		outfile.write(thedat)
		if not len(thedat) in decomplengths:
			decomplengths.append(len(thedat))
print(counttotal)
for i in idlist:
	print(hex(i), iddict[i], sorted(idlengthdict[i]))
	#for length in idcommon[i]:
	#	print(length)
	#	print(stringify(idcommon[i][length]))

print(sorted(decomplengths))
