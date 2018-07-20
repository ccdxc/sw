from stream import *
from parsers import *
from chars import *
from combinators import *

def list_to_number(l):
    num = 0
    for i in l:
        num *= 10
        num += int(i) - int('0')
    return num

def Base10():
    return Map(Many1(Digit()), list_to_number)
    
