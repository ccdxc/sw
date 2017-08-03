import abc
import queue as Queue
import threading
from infra.common.logging import logger
from infra.common.utils import log_exception


class InfraThreadTime(object):
    WAIT_INTERVAL = 2
    MAX_RUN_TIME = 5
    MAX_HANG_TIME = 120
    MAX_BYPASS_TIME = 1200


class InfraThreadExitException(Exception):
    pass


class EvQueue(object):

    def __init__(self, evTimeOutInSec=0):
        self._queue = Queue.Queue()
        self._max_idle_interval = evTimeOutInSec

    def enqueueEv(self, evType):
        self._queue.put(evType)

    def dequeueEv(self):
        event = None
        try:
            event = self._queue.get(timeout=self._max_idle_interval)
        except Queue.Empty as empty:
            raise empty

        return event

    def set_wait_interval(self, evTimeOutInSec=0):
        self._max_idle_interval = evTimeOutInSec


class InfraThread(object):

    def __init__(self, name):
        self.name = name
        self._eventQueue = EvQueue()

    def entry(self):
        pass

    @abc.abstractmethod
    def execute(self, event):
        pass

    # Exception Handler
    def exception(self):
        pass

    # If no event received for wait interval.
    def eventTimeout(self):
        pass

    # Exit Handler when thread leaving.
    def exit(self):
        pass

    def set_wait_interval(self, evTimeOutInSec=0):
        self._eventQueue.set_wait_interval(evTimeOutInSec)


class InfraThreadHandler(InfraThread):

    def __init__(self, name,
                 max_idle_interval=InfraThreadTime.WAIT_INTERVAL,
                 max_time_per_run=InfraThreadTime.MAX_RUN_TIME):
        super(InfraThreadHandler, self).__init__(name)
        self.thread = None
        self.started = False
        self.max_time_per_run = max_time_per_run
        self.max_idle_interval = max_idle_interval
        self.set_wait_interval(max_idle_interval)

    def threadWork(self):
        event = None
        try:
            event = self._eventQueue.dequeueEv()
            if event == InfraThreadExitException:
                raise InfraThreadExitException
        except Queue.Empty:
            self.eventTimeout()
        self.execute(event)

    def threadLooper(self):
        self.entry()
        logger.info("%s thread starts" % (self.name))
        while True:
            try:
                self.threadWork()
            except InfraThreadExitException:
                break
            except Exception as e:
                log_exception()
                logger.critical("Exception in thread %s" % e)
                break
        self.exit()
        self.started = False
        logger.info("%s thread exits" % (self.name))

    def startThread(self):
        if not self.started:
            logger.info("%s thread new instance" % (self.name))
            self.thread = threading.Thread(target=self.threadLooper)
            self.thread.daemon = True
            self.thread.start()
            self.started = True

    def eventEnqueue(self, event):
        self._eventQueue.enqueueEv(event)

    def stopThread(self):
        self._eventQueue.enqueueEv(InfraThreadExitException)
