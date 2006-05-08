#!/usr/bin/python

import RDF
from swap import formula 
from RDFSink import FORMULA, LITERAL, LITERAL_DT, LITERAL_LANG, ANONYMOUS, SYMBOL

class VFS(object):
    """
    Redland pseudo vfs for CWM
    """
    db_dir=''
    db_name=''
    model=None
    storage=None
    store=None
    formula=None
    anonymousNodes={}
    asIfFrom=None
    
    def __init__(self,dbname,db='.'):
        self.db_dir=db
        self.db_name=dbname
        print "dir: "+self.db_dir+" dbname: "+self.db_name
        self.storage=RDF.Storage(storage_name="hashes",
                name=self.db_name,
                options_string="write='false',hash-type='bdb',contexts='yes',dir='"+self.db_dir+"'")
        if self.storage==None:
            raise "Failed opening storage"
        self.model=RDF.Model(self.storage)
        if self.model==None:
            raise "Failed opening RDF model"

    def parse(self,uri,cwm_store,cwm_model,as_if_from):
        """
        Translates a set of statements from Redland Model to CWM Formula
        """
        # CWM variables
        self.store=cwm_store
        self.formula=cwm_model
        self.asIfFrom=as_if_from
        
        if (uri==None) or (uri==''): 
            return cwm_model
        context_stream=self.model.as_stream_context(RDF.Node(RDF.Uri(uri)))
#        context_stream=self.model.as_stream_context()
        if context_stream is None:
            print "URI"+uri+"Not found"
        else:
            for (st,con) in context_stream:
                self.formula.add(self.convert(st.subject),self.convert(st.predicate),self.convert(st.object))
        return self.formula

    def convert(self, s):
        lang = None
        dt = None
        datatype=''
        uri_c=[]
        if s.is_literal():
            what =  str(s._get_literal_value()['string'])
            lang = str(s._get_literal_value()['language'])
            datatype=s._get_literal_value()['datatype']
            if (datatype!=None)and(datatype!=''):
                dt = self.store.newSymbol(str(datatype))
            if (lang == '') or (lang==None):
                lang=None
        elif s.is_blank():
            b_uri=s._get_blank_identifier()
            try:
                what = self.anonymousNodes[b_uri]
            except KeyError:
                self.anonymousNodes[b_uri] = self.store.newBlankNode(self.formula,uri=b_uri)
                what = self.anonymousNodes[b_uri]
        elif s.is_resource():
            uri=str(s._get_uri())
            if ':' in uri:
                what = (SYMBOL, uri)
            else:  #if s[0] == '#':
                what = (SYMBOL, self.asIfFrom + s._get_uri())
        return self.store.intern(what,dt=dt, lang=lang)
