from rplfile import *
import io
import copy
from elftools.elf.elffile import ELFFile
from elftools.elf.constants import *
from elftools.construct.lib import Container
import sys
import os

def shtopflags(a):
	b = P_FLAGS.PF_R
	if a & SH_FLAGS.SHF_WRITE:
		b |= P_FLAGS.PF_W
	if a & SH_FLAGS.SHF_EXECINSTR:
		b |= P_FLAGS.PF_X
	return b

def convertone(infilename, outfilename):
	with open(infilename, "rb") as rplstream, open(outfilename, "wb") as outstream:
		rpl = rplstream.read()
		rplstream.seek(0)
		a = RPLFile(rplstream)
		rplstream = io.BytesIO(rpl)

		newehdr = copy.copy(a.header)
		newehdr["e_ident"]["EI_OSABI"] = 0
		newehdr["e_ident"]["EI_ABIVERSION"] = 0
		newehdr["e_type"] = 3 # dyn
		newehdr["e_phnum"] = newehdr["e_shnum"]
		newehdr["e_phoff"] = newehdr["e_shoff"]
		newehdr["e_phentsize"] = a.structs.Elf_Phdr.sizeof()
		newehdr["e_shoff"] += newehdr["e_phentsize"] * newehdr["e_phnum"]

		outstream.write(a.structs.Elf_Ehdr.build(newehdr))
		b = 0
		crushed_data = io.BytesIO()
		crushed_section = io.BytesIO()
		crushed_indexes = []
		post_headers = newehdr["e_shoff"] + (newehdr["e_shentsize"] * newehdr["e_shnum"])
		if (a.structs.Elf_Shdr.sizeof() != newehdr["e_shentsize"]):
			print("FAIL", a.structs.Elf_Shdr.sizeof(), newehdr["e_shentsize"])
		for i in a.iter_sections():
			data = i.data()
			crushed_indexes.append(crushed_data.tell())
			crushed_data.write(data)
			crushed_section.seek((newehdr["e_shentsize"] * b))
			newheader = copy.copy(i.header)
			newheader["sh_size"] = len(data)
			newheader["sh_offset"] = crushed_indexes[b] + post_headers
			newheader["sh_flags"] &= ~0x08000000
			if newheader["sh_type"] == "SHT_SYMTAB":
				newheader["sh_info"] = 0# newheader["sh_info"] // newheader["sh_entsize"]
			crushed_section.write(a.structs.Elf_Shdr.build(newheader))

			phdr = Container()
			phdr["p_type"] = "PT_LOAD" if newheader["sh_flags"]&SH_FLAGS.SHF_ALLOC else "PT_NULL"
			phdr["p_offset"] = newheader["sh_offset"]
			phdr["p_vaddr"] = phdr["p_paddr"] = newheader["sh_addr"]
			phdr["p_filesz"] = newheader["sh_size"] if newheader["sh_type"] != "SHT_NOBITS" else 0
			phdr["p_memsz"] = newheader["sh_size"]
			phdr["p_flags"] = shtopflags(newheader["sh_flags"])
			phdr["p_align"] = newheader["sh_addralign"]
			outstream.seek(newehdr["e_phoff"] + (newehdr["e_phentsize"] * b))
			outstream.write(a.structs.Elf_Phdr.build(phdr))

			b = b + 1
		outstream.seek(newehdr["e_shoff"])
		outstream.write(crushed_section.getvalue())
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