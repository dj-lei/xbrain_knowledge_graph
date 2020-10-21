import clang.cindex
from clang.cindex import *
import platform
import pandas as pd


class BaseCodeHandle(object):
    def __init__(self):
        if platform.platform().startswith("Windows"):
            self.libClangPath = "C:\\Program Files\\LLVM\\bin\\libclang.dll"
        else:
            self.libClangPath = ''
        Config.set_library_file(self.libClangPath)
        self.index = Index.create()


class CPPCodeHandle(BaseCodeHandle):
    def __init__(self):
        super(CPPCodeHandle, self).__init__()

    def get_header_file_architecture(self, file_path):
        tu = self.index.parse(file_path, ['-x', 'c++'])
        data = []
        for i in tu.cursor.walk_preorder():
            data.append((i.kind, i.spelling, i.location, i.raw_comment))
        return pd.DataFrame(data,columns=['kind', 'spelling', 'location', 'raw_comment'])