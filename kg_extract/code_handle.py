from clang.cindex import *
import platform
import re


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
        tmp = []
        for cursor in tu.cursor.walk_preorder():
            location = dict()
            location['source'], location['line'], location['column'] = re.findall('file (.*?), line (\d+), column (\d+)', str(cursor.location))[0]
            raw_comment = re.sub("[\*\/\\r\\n]", '', str(cursor.raw_comment)).strip().replace('  ', ' ')
            data = {'kind': cursor.kind, 'spelling': cursor.spelling, 'location': location, 'raw_comment': raw_comment}
            tmp.append(data)
        result = {'name': file_path.split('/')[-1], 'cursors': tmp}
        return result
