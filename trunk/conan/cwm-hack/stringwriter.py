class StringWriter:
    string=''
    def write(self,str):
        self.string=self.string+' '+str
    def getContent(self):
        return self.string
