#!/usr/bin/python
# Client for Veggente Conan OWL repository

__version__='0.0.1'

def usage():
    print "Veggente project: Conan OWL repository client"
    print "owlrepository [-h] [-d] [-v] [-p n]"
    print "-h: print this help"
    print "-v: show version"
    print "-d: debug flag"
    print "-p port: start the server on a specified port"
 
def version():
    print "Veggente project: Conan OWL repository client v. "+__version__


if __name__=='__main__':
    try:
        opts, args=getopt.getopt(sys.argv[1:],"hp:vd",['help','=','version','debug'])
    except getopt.GetoptError:
        usage()
        sys.exit(-1)
    for opt, val in opts:
        if opt in ('-h','--help'):
            usage()
            sys.exit(0)
        if opt in ('-v','--version'):
            version()
            sys.exit(0)
