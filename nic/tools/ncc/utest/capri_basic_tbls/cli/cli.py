#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten! 
#
# p4pd CLI
#
from cmd2 import Cmd
import p4pd

class level1Cmd(Cmd):
    prompt = "p4pd/level1> "

    def __init__(self):
        Cmd.__init__(self)

    def do_main(self, args):
        """Calls p4pd unit test."""
        p4pd.main()

    def do_back(self, args):
        """Go back one level."""
        return True

    do_EOF = do_back

class rootCmd(Cmd):

    prompt = "p4pd> "
    intro = "This is a debug CLI. Theres no protection against bad arguments!"

    def __init__(self):
        Cmd.__init__(self)

    def do_level1(self, args):
        """Go to level1."""
        cmd = level1Cmd()
        cmd.cmdloop()

if __name__ == '__main__':
    cmd = rootCmd()
    cmd.cmdloop()
