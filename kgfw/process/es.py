from abc import abstractmethod
from elasticsearch import Elasticsearch


class Base_es(object):
    def __init__(self, addr, port=9200):
        # self.es_database = Graph(addr, auth=auth)
        # self.symbol = self.es_database.begin()
        # self.nodes = NodeMatcher(self.es_database)
        self.es = Elasticsearch([{'host': addr, 'port': port}])

    @abstractmethod
    def insert(self, index, doc_type, doc):
        self.es.index(index=index, doc_type=doc_type, body=doc)

    @abstractmethod
    def match(self):
        pass

    @abstractmethod
    def delete(self):
        pass


class Ru_es(Base_es):
    def __init__(self, addr):
        super(Ru_es, self).__init__(addr)



