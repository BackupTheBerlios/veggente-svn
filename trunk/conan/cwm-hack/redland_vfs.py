#!/usr/bin/python

# 	Bridge between Redland (with BerkeleyDB as a backend) and CWM
#    
#    Copyright(c) 2006 Alessio Carenini <carenini@gmail.com>
# 	This program is free software; you can redistribute it and/or modify
# 	it under the terms of the GNU General Public License as published by
# 	the Free Software Foundation; either version 2 of the License, or
# 	(at your option) any later version.
# 
# 	This program is distributed in the hope that it will be useful,
# 	but WITHOUT ANY WARRANTY; without even the implied warranty of
# 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# 	GNU General Public License for more details.
#
# 	You should have received a copy of the GNU General Public License
# 	along with this program; if not, write to the Free Software
# 	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


import RDF
from swap import formula 
from RDFSink import FORMULA, LITERAL, LITERAL_DT, LITERAL_LANG, ANONYMOUS, SYMBOL

class VFS(object):
    """
    Redland pseudo vfs for CWM
    """
    db_uri=''
    db_name=''
    model=None
    storage=None
    store=None
    formula=None
    anonymousNodes={}
    asIfFrom=None
    
    def __init__(self,db_uri):
        """
        Constructor
        db_uri: string
            A MySQL db uri is in the form
                mysql:username:password@host:port/db_name
            While a SQLite db uri is in the form
                sqlite:db_name://db_dir
            None if you plan to fetch data from an in-memory storage
        """
        if db_uri is None:
            return None
        db_type=db_uri.split(':')[0]
        self.db_uri=db_uri[db_uri.find(':')+1:]
        # Database name   
        #print "dir: "+self.db_uri+" dbname: "+self.db_name
        if db_type=='sqlite':
            self.db_name=self.db_uri.split(':')[0]
            self.storage=RDF.Storage(storage_name="sqlite",
                    name=self.db_name,
                    options_string="write='false',contexts='yes',dir='"+self.db_uri+"'")
        elif db_type=='mysql':
            # Host / port
            db_host=(db_uri.split('@')[1]).split(':')[0]
            db_port=(db_uri.split('@')[1]).split(':')[1]
            # Username / password
            userpass=self.db_uri.split('@')[0]
            username=userpass.split(':')[0]
            passwd=userpass.split(':')[1]
            self.db_name=db_uri.split('/')[-1]
            storage_options="contexts='yes',write='false',database='"+self.db_name+"',host='"+db_host+"',port='"+db_port+"',user='"+username+"',password='"+passwd+"'"
            self.storage=RDF.Storage(storage_name="mysql",
                    name=self.db_name,
                    options_string=storage_options)
        if self.storage==None:
            raise "Failed opening storage"
        self.model=RDF.Model(self.storage)
        if self.model==None:
            raise "Failed opening RDF model"

    def parse(self,uri,cwm_store,cwm_model,as_if_from):
        """
        Translates a set of statements from a Redland Model to CWM Formula
        """
        # CWM variables
        self.store=cwm_store
        self.formula=cwm_model
        self.asIfFrom=as_if_from
        if (uri is None): 
            return self.formula
        if type(uri)==str:
            if (uri==''): 
                return cwm_model
            context_stream=self.model.as_stream_context(RDF.Node(RDF.Uri(uri)))
            if context_stream is None:
                print "URI"+uri+"Not found"
            else:
                for (st,con) in context_stream:
                    self.formula.add(self.convert(st.subject),self.convert(st.predicate),self.convert(st.object))
        elif (type(uri)==RDF.Model) or (isinstance(uri,RDF.Stream)):
            self.model=uri
            for st in self.model:
                self.formula.add(self.convert(st.subject),self.convert(st.predicate),self.convert(st.object))
        elif (type(uri)==tuple):
            self.model=uri[0]
            context=uri[1]
            if (uri==()) or (self.model is None):
                return self.formula
            if (context is None):
                for st in model:
                    self.formula.add(self.convert(st.subject),self.convert(st.predicate),self.convert(st.object))
            else:
                for (st,con) in self.model.as_stream_context(RDF.Node(RDF.Uri(context))):
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
