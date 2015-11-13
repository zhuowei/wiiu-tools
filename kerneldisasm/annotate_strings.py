import sys
with open("kernel.out", "rb") as infile:
	kerneldat = infile.read()

kernelbase = 0xffe00000
s2start = 0xffec1220
s13start = 0xffeb1780

def readstr(d, off):
	if off < 0 or off >= len(d):
		return ""
	out = ""
	i = off
	while i < len(d) and ((d[i] >= 0x20 and d[i] <= 0x7e) or d[i] == 0x0a):
		out += chr(d[i])
		i += 1
	return out

with open(sys.argv[1], "r") as infile:
	for l in infile:
		l = l.rstrip("\n")
		if "!!str:" in l:
			print(l)
			continue
		if "0xff" in l and not "+0xff" in l:
			addrs = l.find("0xff")
			addr = int(l[addrs:addrs+10], 16) - kernelbase
			mystr = readstr(kerneldat, addr)
			if len(mystr) >= 4:
				print(l, "!!str", mystr)
			else:
				print(l)
		elif "addi    " in l and (",r2," in l or ",r13," in l):
			addendoff = l.find(",", l.find(",r") + 1) + 1
			endone = l.find(" ", addendoff)
			if endone == -1:
				endone = len(l)
			addend = int(l[addendoff:endone])
			start = s2start if ",r2," in l else s13start
			addr = start + addend
			anno = "// " + hex(addr)
			mystr = readstr(kerneldat, addr-kernelbase)
			if len(mystr) >= 4:
				anno += " !!str " + repr(mystr)
			print(l, anno)
		else:
			print(l)
