
class Icmp:
    def __init__(self, **kwargs):
        pass

    def __str__(self):
        return ""

    def __eq__(self, obj):
        return isinstance(obj, Icmp)

    def match(self, **kwargs):
        return True

    def getRandom(self, seed=None):
        return {}
