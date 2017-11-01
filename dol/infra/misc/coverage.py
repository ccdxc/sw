import os
import shutil
from infra.common.glopts    import GlobalOptions


def Init(module):
    #Prepare the directory so that model data can be moved to this directory.
    if GlobalOptions.coveragerun:
        os.makedirs("/".join([os.environ["MODEL_DATA_DEST_DIR"], "_".join(module.feature.split("/")), module.name]),
                      exist_ok=True)

class TestCaseCoverageHelper(object):
    def __init__(self, tc):
        self._tc = tc

    def Process(self):
        if GlobalOptions.coveragerun:
            try:
                os.rename(os.environ["MODEL_DATA_OUT_DIR"],
                           "/".join([os.environ["MODEL_DATA_DEST_DIR"],
                                    "_".join(self._tc.module.feature.split("/")),
                                    self._tc.module.name,
                                    str(self._tc.ID())]))
            except:
                #Ignore for now as there are more than 1 dol run for same feature.
                #Remove directory as well as we want to discard this data.
                shutil.rmtree(os.environ["MODEL_DATA_OUT_DIR"])
            #Restore this directory so that model can dump more data.
            os.mkdir(os.environ["MODEL_DATA_OUT_DIR"])
