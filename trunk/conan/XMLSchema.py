class Element:
    name=''
    type=''
    minOccurs=1
    maxOccurs=1
    nillable='true'

class Attribute:
    name=''
    type=''
    default=None
    fixed=None
    use='optional'

class ComplexType:
    sequence=[]
    group=[]
    all=[]
    attributes=[]

class XMLSchema:
    types={}
    entry_point=None
