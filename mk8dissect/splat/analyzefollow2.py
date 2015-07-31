import struct
with open("follow2.dat", "rb") as infile:
	indata = infile.read()

def u16l(a, b):
	return a[b]| a[b+1] << 8

def u32(buf, off):
	return struct.unpack(">i", buf[off:off+4])[0] & 0xffffffff

off = 0
lencommon = {}
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
lencount = {}
while off < len(indata):
	packetlen = u32(indata, off)
	off += 4
	print("START", packetlen)
	while packetlen > 0:
		packettype = indata[off]
		packetlen2 = u16l(indata, off + 1)
		print(stringify(indata[off:off+packetlen2]))
		oldcommon = lencommon[packetlen2] if packetlen2 in lencommon else None
		lencommon[packetlen2] = commonit(oldcommon, indata[off:off+packetlen2])
		lencount[packetlen2] = 1 if not packetlen2 in lencount else lencount[packetlen2] + 1
		off += packetlen2
		packetlen -= packetlen2
	if packetlen != 0:
		print(packetlen)
		exit(0)
for len in lencommon:
	print("lencommon", len)
	print(stringify(lencommon[len]))
print("lencount")
print(lencount)