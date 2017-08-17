#!/usr/bin/python
#
# Capri Debug Shell (debug_shell.py)
# Ajeer Pudiyapura (Pensando Systems)

from cmd2 import Cmd
import os
import sys

cur_path = os.path.abspath(__file__)
cur_path = os.path.split(cur_path)[0]

class debugCmd(Cmd):

    prompt = "debug> "
    intro = "This is a debug CLI. Theres no protection against bad arguments!"

    def __init__(self):
        Cmd.__init__(self)
        self.exclude_from_help.append('do_py')
        self.exclude_from_help.append('do_set')
        self.exclude_from_help.append('do_run')
        self.exclude_from_help.append('do_EOF')
        self.exclude_from_help.append('do_quit')
        self.exclude_from_help.append('do_save')
        self.exclude_from_help.append('do_edit')
        self.exclude_from_help.append('do_show')
        self.exclude_from_help.append('do_shell')
        self.exclude_from_help.append('do_pyscript')
        self.exclude_from_help.append('do_shortcuts')
        self.exclude_from_help.append('do__relative_load')
        self.exclude_from_help.append('do_cmdenvironment')

    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit

    iris_path = cur_path + '/gen/iris/cli'
    if os.path.isfile(iris_path + '/_p4pd.so'):
        sys.path.append(iris_path)
        from p4pd_cli import init as iris_init
        iris_init()
        def do_iris(self, args):
            """Interact with Iris Tables"""
            from p4pd_cli import rootCmd as iris_root
            cmd = iris_root()
            cmd.cmdloop()


if __name__ == '__main__':

    cmd = debugCmd()
    cmd.cmdloop()

