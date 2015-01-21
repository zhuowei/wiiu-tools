from elftools.elf.sections import *
import io
import zlib

class RPLSection(Section):
    """ Base class for ELF sections. Also used for all sections types that have
        no special functionality.

        Allows dictionary-like access to the section header. For example:
         > sec = Section(...)
         > sec['sh_type']  # section type
    """
    def __init__(self, header, name, stream, elffile):
        Section.__init__(self, header, name, stream)
        self.elffile = elffile
        self.elfstructs = self.elffile.structs

    def data(self):
        """ The section data from the file.
        """
        if self['sh_flags'] & 0x08000000 == 0:
            return Section.data(self)
        self.stream.seek(self['sh_offset'])
        uncompressed_len = self.elfstructs.Elf_xword("uncompressed_len").parse(self.stream.read(4))
        compressed = self.stream.read(self['sh_size'] - 4)
        return zlib.decompress(compressed)

class RPLStringTableSection(RPLSection):
    """ ELF string table section.
    """
    def __init__(self, header, name, stream, elffile):
        super(RPLStringTableSection, self).__init__(header, name, stream, elffile)

    def get_string(self, offset):
        """ Get the string stored at the given offset in this string table.
        """
        s = parse_cstring_from_stream(io.BytesIO(self.data()), offset)
        return s