from docx import Document
from abc import abstractmethod
import pandas as pd
import re
import zipfile


class BaseDocHandle(object):

    def __init__(self, path):
        self.word = ''
        self.proper_nouns = set()
        self.document = Document(path)
        # with open('kg_extract/google-10000-english-usa-no-swears.txt', 'r') as f:
        #     self.common_words = [token.strip() for token in f]

    def get_proper_nouns_csv(self):
        self.proper_nouns = set(pd.read_csv('proper_nouns.csv')['proper_nouns_name'].values)

    def get_doc_structure(self):
        count = 0
        doc_name = self.document.core_properties.title
        self.get_proper_nouns_csv()

        level = 0
        content = [0 for _ in range(0, 10)]
        content[0] = doc_name
        temp_name = ''
        data = pd.DataFrame([], columns=['s', 'p', 'o', 'key_words'])
        for i in range(0, len(self.document.paragraphs)):
            doc = self.document.paragraphs[i].text.strip()
            style_name = self.document.paragraphs[i].style.name
            if doc in ['', '\n']:
                continue

            key_words = str(list(self.proper_nouns.intersection(set(doc.split(' ')))))
            if style_name == 'Heading':
                data.loc[count, 's'] = content[0]
                data.loc[count, 'p'] = 'Heading 0'
                data.loc[count, 'o'] = doc
                data.loc[count, 'key_words'] = key_words
                temp_name = doc
            elif 'Heading' in style_name:
                level = int(style_name.split(' ')[1])
                content[level] = doc

                data.loc[count, 's'] = content[level-1]
                data.loc[count, 'p'] = style_name
                data.loc[count, 'o'] = doc
                data.loc[count, 'key_words'] = key_words
            else:
                if level == 0:
                    data.loc[count, 's'] = temp_name
                    data.loc[count, 'p'] = 'Heading ' + str(level + 1)
                    data.loc[count, 'o'] = doc
                    data.loc[count, 'key_words'] = key_words
                    count = count + 1
                    continue
                data.loc[count, 's'] = content[level]
                data.loc[count, 'p'] = 'Heading '+str(level+1)
                data.loc[count, 'o'] = doc
                data.loc[count, 'key_words'] = key_words
            count = count + 1

        data = data.reset_index(drop=True)

        return data

    def get_doc_images(self, docxpath, dstpath):
        doc = zipfile.ZipFile(docxpath)
        for info in doc.infolist():
            if info.filename.endswith((".png", ".jpeg", ".jpg", ".gif", ".emf")):
                doc.extract(info.filename, dstpath)
        doc.close()

    def get_doc_tables(self):
        table = self.document.tables[2]

        ls = []
        for row in table.rows:
            for cell in row.cells:
                for paragraph in cell.paragraphs:
                    ls.append(paragraph.text)

        row = 0
        column = 0
        df = pd.DataFrame(np.array(ls).reshape(row, column))  # reshape to the table shape
        new = docx.Document()
        word_table = new.add_table(rows=row, cols=column)
        for x in range(0, row, 1):
            for y in range(0, column, 1):
                cell = word_table.cell(x, y)
                cell.text = df.iloc[x, y]

        return new, df

    @abstractmethod
    def cut_special_symbols_and_filter_rules(self, word):
        pass

    @abstractmethod
    def get_proper_nouns(self):
        pass


class RuDocHandle(BaseDocHandle):

    def __init__(self, path):
        super(RuDocHandle, self).__init__(path)

    def cut_special_symbols_and_filter_rules(self, word):
        r_symbols = "[\s\.\!\$%^*\"\'\:\;\,\?\(\)\[\]\“\”\‘\’\、]"
        r_symbols_special = '[\-\/]$'
        r_filter = '[0-9A-Z]-[0-9A-Z]'
        # cut_special_symbols
        self.word = re.sub(r_symbols, '', word).strip()
        self.word = re.sub(r_symbols_special, '', self.word)
        # filter_rules
        if len(re.findall(r_filter, self.word)) > 0:
            return ''
        return self.word

    def get_proper_nouns(self):
        count = 0
        proper_nouns_name = ''
        data = pd.DataFrame([], columns=['proper_nouns_name', 'text'])
        for i in range(0, len(self.document.paragraphs)):
            doc = self.document.paragraphs[i].text.strip()
            if doc not in ['', '\n']:
                for word in doc.split(' '):
                    if word.isupper():
                        proper_nouns_name = word
                    elif len(re.findall('[\(](.*?)[\)]', word)) > 0:
                        proper_nouns_name = re.findall('[\(](.*?)[\)]', word)[0]
                    else:
                        continue

                    proper_nouns_name = self.cut_special_symbols_and_filter_rules(proper_nouns_name)
                    if len(proper_nouns_name) <= 1:
                        continue
                    data.loc[count, 'proper_nouns_name'] = proper_nouns_name
                    data.loc[count, 'text'] = doc
                    count = count + 1
        data['file_name'] = self.document.core_properties.title
        data = data.sort_values(by='proper_nouns_name').drop_duplicates(['proper_nouns_name']).reset_index(drop=True)

        return data
