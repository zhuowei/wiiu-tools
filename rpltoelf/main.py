from rplfile import *
import io
import copy
from elftools.elf.elffile import ELFFile
import sys
import os

def convertone(infilename, outfilename):
	with open(infilename, "rb") as rplstream, open(outfilename, "wb") as outstream:
		rpl = rplstream.read()
		rplstream.seek(0)
		a = RPLFile(rplstream)
		rplstream = io.BytesIO(rpl)
		headerbytes = bytearray(rplstream.read(a.header["e_ehsize"]))
		headerbytes[0x7] = 0 # sysV
		headerbytes[0x8] = 0 # abi v0
		headerbytes[0x10] = 0
		headerbytes[0x11] = 3 # dyn

		outstream.write(headerbytes)
		b = 0
		crushed_data = io.BytesIO()
		crushed_indexes = []
		post_headers = a.header["e_shoff"] + (a.header["e_shentsize"] * a.header["e_shnum"])
		if (a.structs.Elf_Shdr.sizeof() != a.header["e_shentsize"]):
			print("FAIL", a.structs.Elf_Shdr.sizeof(), a.header["e_shentsize"])
		for i in a.iter_sections():
			data = i.data()
			crushed_indexes.append(crushed_data.tell())
			crushed_data.write(data)
			outstream.seek(a.header["e_shoff"] + (a.header["e_shentsize"] * b))
			newheader = copy.copy(i.header)
			newheader["sh_size"] = len(data)
			newheader["sh_offset"] = crushed_indexes[b] + post_headers
			newheader["sh_flags"] &= ~0x08000000
			if newheader["sh_type"] == "SHT_SYMTAB":
				newheader["sh_info"] = 0# newheader["sh_info"] // newheader["sh_entsize"]
			outstream.write(a.structs.Elf_Shdr.build(newheader))
			b = b + 1
		outstream.write(crushed_data.getvalue())

def convertdir(indirname):
	filenames = os.listdir(indirname)
	for f in filenames:
		p = os.path.join(indirname, f)
		if not os.path.isfile(p):
			continue
		if f in ["..", "."]:
			continue
		if f.endswith(".elf"):
			continue
		with open(p, "rb") as temp:
			if temp.read(4) != b"\x7fELF":
				continue
			temp.seek(0x7)
			if temp.read(2) != b"\xca\xfe":
				continue
		convertone(p, p + ".elf")

def main():
	if len(sys.argv) < 2:
		print("Usage: ", sys.argv[0], "<rpl> [outputname]")
		return
	infilename = sys.argv[1]
	if os.path.isdir(infilename):
		convertdir(infilename)
		return
	outfilename = sys.argv[1] + ".elf"
	if len(sys.argv) >= 3:
		outfilename = sys.argv[2]
	convertone(infilename, outfilename)

if __name__ == "__main__":
	main()