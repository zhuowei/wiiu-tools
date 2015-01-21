#-------------------------------------------------------------------------------
# elftools: elf/elffile.py
#
# ELFFile - main class for accessing ELF files
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from elftools.elf.elffile import ELFFile
from structs import RPLStructs
from sections import *


class RPLFile(ELFFile):
    """ Creation: the constructor accepts a stream (file-like object) with the
        contents of an ELF file.

        Accessible attributes:

            stream:
                The stream holding the data of the file - must be a binary
                stream (bytes, not string).

            elfclass:
                32 or 64 - specifies the word size of the target machine

            little_endian:
                boolean - specifies the target machine's endianness

            header:
                the complete ELF file header

            e_ident_raw:
                the raw e_ident field of the header
    """
    def __init__(self, stream):
        self.stream = stream
        self._identify_file()
        self.structs = RPLStructs(
            little_endian=self.little_endian,
            elfclass=self.elfclass)
        self.header = self._parse_elf_header()

        self.stream.seek(0)
        self.e_ident_raw = self.stream.read(16)

        self._file_stringtable_section = self._get_file_stringtable()
        self._section_name_map = None

    def _make_section(self, section_header):
        """ Create a section object of the appropriate type
        """
        name = self._get_section_name(section_header)
        sectype = section_header['sh_type']
        return RPLSection(section_header, name, self.stream, self)


    def _get_file_stringtable(self):
        """ Find the file's string table section
        """
        stringtable_section_num = self['e_shstrndx']
        return RPLStringTableSection(
                header=self._get_section_header(stringtable_section_num),
                name='',
                stream=self.stream, elffile=self)


