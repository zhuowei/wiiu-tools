# I swear to potato, if you make one BL joke...
import sys

sprs = {}

def load_sprs():
	with open("spr.txt", "r") as sprfile:
		for l in sprfile:
			if l.startswith("//") or not " = " in l:
				continue
			parts = l.strip().split(" = ")
			name = parts[0]
			id = int(parts[1][0:parts[1].find(",")])
			sprs[id] = name

load_sprs()

def filter_spr(line):
	if "mfspr   " in line:
		start = line.find("mfspr   ")
		spr = int(line[line.find(",", start)+1:])
		sprname = sprs[spr] if spr in sprs else "unknown SPR!"
		print(line, "//", sprname)
		return True
	elif "mtspr   " in line:
		start = line.find("mtspr   ")
		spr = int(line[start+len("mtspr   "):line.find(",", start)])
		sprname = sprs[spr] if spr in sprs else "unknown SPR!"
		print(line, "//", sprname)
		return True
	else:
		return False

def printable(a):
	return a >= 20 and a < 0x7f

def getfourcc(theval):
	return chr((theval >> 24) & 0xff) + chr((theval >> 16) & 0xff) + chr((theval >> 8) & 0xff) + chr(theval & 0xff)

def filter_load(line, lastline):
	if not "ori     " in line and not "addi " in line:
		return False
	if not "lis " in lastline:
		return False
	if "addi " in line:
		lineregstart = line.find("addi ") + len("addi    ")
	else:
		lineregstart = line.find("ori     ")+len("ori     ")
	linereg = line[lineregstart:line.find(",", lineregstart)]
	lastlineregstart = lastline.find("lis     ")+len("lis     ")
	lastlinereg = lastline[lastlineregstart:lastline.find(",", lastlineregstart)]
	if linereg != lastlinereg:
		return False
	lineval = int(line[line.rfind(",")+1:])
	if lineval < 0:
		lineval = 0x10000+lineval
	lastlineval = int(lastline[lastline.rfind(",")+1:])
	if lastlineval < 0:
		lastlineval = 0x10000+lastlineval
	theval = lastlineval << 16 | lineval
	anno = hex(theval)
	if printable(theval & 0xff) and printable((theval >> 8) & 0xff) and printable((theval >> 16) & 0xff) and printable((theval >> 24) & 0xff):
		anno += " " + getfourcc(theval)
	print(line, "//", anno)
	return True

with open(sys.argv[1], "r") as intext:
	indata = intext.read().split("\n")

funclist = {}

for l in indata:
	if "bl      " in l:
		blindex = l.find("bl      ") + len("bl      ")
		endindex = l.find(" ", blindex)
		if endindex == -1:
			endindex = len(l)
		addr = int(l[blindex:endindex], 16)
		ownaddr = int(l[0:l.find(":\t")], 16)

		if not addr in funclist:
			funclist[addr] = [ownaddr]
		else:
			funclist[addr].append(ownaddr)

#funclist.sort()

lastline = ""

for l in indata:
	if ":\t" not in l:
		print(l)
		continue
	addr = int(l[0:l.find(":\t")], 16)
	if addr in funclist:
		print("sub_" + hex(addr)[2:] + ":")
		print("xrefs:", ", ".join([hex(a)[2:] for a in funclist[addr]]))
	if filter_spr(l):
		pass
	elif filter_load(l, lastline):
		pass
	else:
		print(l)
	lastline = l