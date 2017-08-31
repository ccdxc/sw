//:: import os, pdb
//:: from collections import OrderedDict
//:: csr = _context['csr']
//:: #pdb.set_trace()
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# Capri Debug Shell (debug_shell.py)
# Ajeer Pudiyapura (Pensando Systems)
#

from cmd2 import Cmd
import os
import sys

//::    for l0 in csr:
class ${l0}(Cmd):

    prompt = "${l0}> "
    intro = "Interact with ${l0}"

    def __init__(self):
        Cmd.__init__(self)

//::        for l1 in csr[l0]:
    class ${l1}(Cmd):

        prompt = "${l1}> "
        intro = "Interact with ${l1}"

        def __init__(self):
            Cmd.__init__(self)

//::            for l2 in csr[l0][l1]:
//::                if csr[l0][l1][l2]['is_array']:
//::                    name = l2.split('[')[0]
//::                    usage = 'Usage: %s <index>' % (name)
//::                    docu = usage
//::                    num_args = 1
//::                else:
//::                    name = l2
//::                    usage = 'Usage: %s' % (name)
//::                    docu = usage
//::                    num_args = 0
//::                #endif
        class ${name}(Cmd):

            prompt = "${name}> "
            intro = "Interact with ${name}"

            def __init__(self):
                Cmd.__init__(self)

            def do_quit(self, args):
                """Exits to the previous level"""
                return True

            do_EOF = do_quit

            do_exit = do_quit

        def do_${name}(self, args):
            """${docu}"""

            values = args.split()
            if len(values) != ${num_args}:
                print('${usage}')
                return

            cmd = ${l0}.${l1}.${name}()
            cmd.cmdloop()

//::            #endfor

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit

    def do_${l1}(self, args):
        """Usage: ${l1}"""

        values = args.split()
        if len(values) != 0:
            print('Usage: ${l1}')
            return

        cmd = ${l0}.${l1}()
        cmd.cmdloop()

//::        #endfor


    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit
//::    #endfor

class rootCmd(Cmd):

    prompt = "capri> "
    intro = "Explore the Capri chip"

    def __init__(self):
        Cmd.__init__(self)

    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit

//::    for l0 in csr:
    def do_${l0}(self, args):
        """Usage: ${l0}"""

        values = args.split()
        if len(values) != 0:
            print('Usage: ${l0}')
            return

        cmd = ${l0}()
        cmd.cmdloop()
//::    #endfor

def init():
    pass

def cleanup():
    pass

if __name__ == '__main__':

    init()
    cmd = rootCmd()
    cmd.cmdloop()
    cleanup()
