import os
import sys

from parsercomb import *

STRUCT = 'struct'
UNION = 'union'

class Printer(object):
    def __init__(self):
        self._ident = 0
        self._buffer = ''
    
    def add(self, s):
        self._buffer += ' ' * self._ident + s + '\n'

    def incr(self):
        self._ident += 4

    def decr(self):
        if self._ident >= 4:
            self._ident -= 4

class CAttribute(object):
    def __init__(self, value):
        self._value = value

class CEnum(object):
    def __init__(self, name, fields):
        self._name = name
        self._fields = fields

    def regen(self, pr, trailing_semicolon=True):
        pr.add('enum ')
        if self._name:
            pr.add('%s ' % (self._name))
        pr.add('{')
        pr.incr()
        for f in self._fields:
            pr.add('%s' % (f._name))
            if f._value != None:
                pr.add(' = %s' % (f._value))
            pr.add(',')
        pr.decr()
        pr.add('}')
        if trailing_semicolon:
            pr.add(';')

class CEnumField(object):
    def __init__(self, name, value):
        self._name = name
        self._value = value

class CType(object):
    def __init__(self, name=None, embedded=None):
        self._name = name
        self._basic = embedded == None
        self._embedded = embedded
        self._resolved = None

    def __repr__(self):
        return "Ctype: Name: %s Basic: %s Embedded: %s" % (self._name, 
            self._basic, self._embedded)

    def resolve(self, defs):
        if self._basic:
            self._resolved = defs[self._name]
            self._resolved.resolve_defs(defs)
        else:
            self._embedded.resolve_defs(defs)

    def comp(self, other):
        if self._name != other._name:
            print('Warning: Type name mismatch: %s vs %s' % 
                (self._name, other._name))
        if self._embedded:
            self._embedded.comp(other._embedded)
        if self._resolved:
            self._resolved.comp(other._resolved)

class CScalar(object):
    def __init__(self, name=None, size=0):
        self._name = name
        self._size = size

    def __repr__(self):
        return 'CScalar: %s %s' % (self._name, self._size)

    def resolve_defs(self, defs):
        pass

    def comp(self, other):
        if self._size != other._size:
            print('Error: Scalar type size mismatch %s vs %s' % 
                (self._size, other._size))
        if self._name != other._name:
            print('Warning: Scalar type name mismatch %s vs %s' % 
                (self._name, other._name))

class CTypedef(object):
    def __init__(self, name=None, ctype=None):
        self._name = name
        self._ctype = ctype
        
    def __repr__(self):
        return "CTypedef: %s %s" % (self._name, self._ctype)

    def regen(self, pr):
        pr.add('typedef ')
        self._ctype.regen(pr, False)
        pr.add(' %s;' % (self._name))

class CCompound(object):
    def __init__(self, kind, name=None, fields=[], attr=None):
        self._kind = kind
        self._name = name
        self._fields = fields
        self._attr = attr

    def is_union(self):
        return False

    def is_struct(self):
        return True

    def __repr__(self):
        return "CCompound: %s %s %s" % (self._kind, self._name, self._fields)

    def resolve_defs(self, defs):
        for f in self._fields:
            f.resolve_def(defs)

    def comp(self, other):
        if self._kind != other._kind:
            print('Error: Different kind of compount type %s vs %s',
                (self._kind, other._kind))
        if len(self._fields) != len(other._fields):
            print('Error: Different number of fields for %s vs %s' % 
                (self._name, other._name))
        for i in range(0, min(len(self._fields), len(other._fields))):
            self._fields[i].comp(other._fields[i])

    def regen(self, pr, trailing_semicolon=True):
        pr.add(self._kind + " ")
        if self._attr:
            pr.add("__attribute__((%s)) " % (self._attr._value))
        if self._name:
            pr.add(self._name + " ")
        pr.add("{")
        pr.incr()
        for f in self._fields:
            f.regen(pr)
        pr.decr()
        pr.add('}')
        if trailing_semicolon:
            pr.add(';')

class CField(object):
    def __init__(self, name=None, ctype=None, count=1, bits=None):
        self._name = name
        self._ctype = ctype
        self._count = count # Array size count "uint32 x[<COUNT>];"
        self._bits = bits # Case of "uint32_t x:<BITS>;"

    def __repr__(self):
        return "CField: Name: %s CType: %s" % (self._name, self._ctype)

    def resolve_def(self, defs):
        self._ctype.resolve(defs)

    def comp(self, other):
        if self._name != other._name:
            print('Warning: Field names differ: %s vs %s' %
                (self._name, other._name))
        if self._count != other._count:
            print('Error: Field elmenet count(array) differ: %s vs %s' %
                (self._name, other._name))
        if self._bits != other._bits:
            print('Error: Field elmenet bits differ: %s vs %s' %
                (self._name, other._name))
        self._ctype.comp(other._ctype)

    def regen(self, pr):
        if self._ctype._basic:
            pr.add('%s' % (self._ctype._name))
        else:
            self._ctype._embedded.regen(pr, False)
        if self._name:
            pr.add(' %s' % (self._name))
        if self._count:
            pr.add('[%s]' % (self._count))
        if self._bits:
            pr.add(' : %s' % (self._bits))
        pr.add(';')

# Currenlty we ignore escaped characters. Fix me
def Escaped():
    return Map(Sequence([Char('\\'), Any()]), lambda _: None)

def NewLine():
    return Map(Char('\n'), lambda _: ('NewLine', None))

def Whitespaces():
    return Map(Many1(WhiteSpace()), lambda _: None)

def CommentBlockStart():
    return Map(String('/*'), lambda _: ('CommentBlockStart', None))

def CommentBlockEnd():
    return Map(String('*/'), lambda _: ('CommentBlockEnd', None))

def CommentLineStart():
    return Map(String('//'), lambda _: ('CommentLineStart', None))

def Constant():
    return Map(Base10(), lambda t: ('Constant', t))

def Identifier():
    return Map(Many1(Choice([AlphaNumeric(), Char('_')])), 
        lambda ts: ('ID', ''.join(ts)))

def Operator():
    return Map(Any(), lambda t: ('Operator', t))

TokenList = [
    Escaped(),
    NewLine(),
    Whitespaces(),
    CommentBlockStart(),
    CommentBlockEnd(),
    CommentLineStart(),
    Constant(),
    Identifier(),
    Operator(),
]

def Tokenizer():
    return Map(Many1(Choice(TokenList)), lambda ts: [t for t in ts if t])

def Token(name, value):
    def apply(stream):
        s = stream.clone()
        t = s.consume()
        if not t:
            raise NoMatch()
        if t[0] == name and t[1] == value:
            return (t, s)
        raise NoMatch()
    return apply

def TokenName(name):
    def apply(stream):
        s = stream.clone()
        t = s.consume()
        if not t:
            raise NoMatch()
        if t[0] == name:
            return (t, s)
        raise NoMatch()
    return apply

def CommentBlock():
    return Map(Between(TokenName('CommentBlockStart'), TokenName('CommentBlockEnd')),
        lambda _: None)

def CommentLine():
    return Map(Between(TokenName('CommentLineStart'), TokenName('NewLine')),
        lambda _: None)

def Directive():
    return Map(Between(Token('Operator', '#'), TokenName('NewLine')), 
        lambda _: None)

# Gets rid of comments('/*...*/', '//...'), directives('#...') and newlines
def DeNoiser():
    return Map(Many1(Choice([CommentBlock(), CommentLine(), Directive(), Any()])),
        lambda ts: [t for t in ts if t and t[0] != 'NewLine'])

def Expression():
    def apply(stream):
        s = stream.clone()
        ((_, var_a), s) = TokenName('Constant')(s)
        ((_, op), s) = TokenName('Operator')(s)
        ((_, var_b), s) = TokenName('Constant')(s)
        if op == '/':
            return (('Constant', var_a // var_b), s)
        raise NoMatch()
    return apply

def ArrayLiteral():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('Operator', '[')(s)
        (length, s) = Option(Choice([Expression(), TokenName('Constant')]))(s)
        if length:
            (_, length) = length
        (_, s) = Token('Operator', ']')(s)
        return (('ArrayLiteral', length), s)
    return apply

def FieldType():
    def apply(stream):
        s = stream.clone()
        (embedded, s) = Option(Choice([Struct(), Union(), Enum()]))(s)
        if embedded:
            return (CType(embedded._name, embedded), s)
        (name, s) = Option(Sequence([Token('ID', STRUCT), TokenName('ID')]))(s)
        if name:
            return (CType(name[1][1]), s)
        ((_, name), s) = TokenName('ID')(s)
        return (CType(name), s)
    return apply

def FieldDeclaration():
    def apply(stream):
        s = stream.clone()
        (ctype, s) = FieldType()(s)
        (name, s) = Option(TokenName('ID'))(s)
        if name:
            (_, name) = name
        (spec, s) = Option(Choice([ArrayLiteral(), StructFieldBitSize()]))(s)
        (_, s) = Token('Operator', ';')(s)
        count = None
        bitsize = None
        if spec:
            (extra, value) = spec
            if extra == 'ArrayLiteral':
                count = value
            if extra == 'Bitsize':
                bitsize = value
        return (CField(name, ctype, count, bitsize), s)
    return apply

def Attribute():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('ID', '__attribute__')(s)
        (_, s) = Token('Operator', '(')(s)
        (_, s) = Token('Operator', '(')(s)
        ((_, attr), s) = TokenName('ID')(s)
        (_, s) = Token('Operator', ')')(s)
        (_, s) = Token('Operator', ')')(s)
        return (CAttribute(attr), s)
    return apply

def StructFieldBitSize():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('Operator', ':')(s)
        ((_, value), s) = TokenName('Constant')(s)
        return (('Bitsize', value), s)
    return apply

def Struct():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('ID', 'struct')(s)
        (attr, s) = Option(Attribute())(s)
        (name, s) = Option(TokenName('ID'))(s)
        if name:
            (_, name) = name
        (_, s) = Token('Operator', '{')(s)
        (fields, s) = Many(FieldDeclaration())(s)
        (_, s) = Token('Operator', '}')(s)
        return (CCompound(STRUCT, name, fields, attr), s)
    return apply

def EnumFieldAssignement():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('Operator', '=')(s)
        ((_, value), s) = TokenName('Constant')(s)
        return (value, s)
    return apply

def EnumField():
    def apply(stream):
        s = stream.clone()
        ((_, name), s) = TokenName('ID')(s)
        (value, s) = Option(EnumFieldAssignement())(s)
        return (CEnumField(name, value), s)
    return apply

def Enum():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('ID', 'enum')(s)
        ((_, name), s) = TokenName('ID')(s)
        (_, s) = Token('Operator', '{')(s)
        (fields, s) = SeparatedBy(EnumField(), Token('Operator', ','))(s)
        (_, s) = Token('Operator', '}')(s)
        return (CEnum(name, fields), s)
    return apply

def Union():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('ID', 'union')(s)
        (name, s) = Option(TokenName('ID'))(s)
        if name:
            (_, name) = name
        (_, s) = Token('Operator', '{')(s)
        (fields, s) = Many(FieldDeclaration())(s)
        (_, s) = Token('Operator', '}')(s)
        return (CCompound(UNION, name, fields), s)
    return apply

def Typedef():
    def apply(stream):
        s = stream.clone()
        (_, s) = Token('ID', 'typedef')(s)
        (t, s) = Choice([Struct(), Enum(), Union()])(s)
        ((_, name), s) = TokenName('ID')(s)
        (_, s) = Token('Operator', ';')(s)
        return (CTypedef(name, t), s)
    return apply

def AST():
    return Many(Choice([Typedef(), Struct(), Enum(), Union(), Any()]))
    
def parse_code(code):
    s = Stream(code)
    (tokens, s) = Tokenizer()(s)
    s = Stream(tokens)
    (tokens, s) = DeNoiser()(s)
    s = Stream(tokens)
    (tokens, s) = AST()(s)
    return tokens
    
def parse_file(filename):
    f = open(filename, 'r')
    return parse_code(f.read())

def parse_files(filelist):
    asts = []
    for f in filelist:
        print('Parsing %s' % (f))
        asts.append(parse_file(f))
    defs = {
        'uint8_t': CScalar('uint8_t', 8),
        'uint16_t': CScalar('uint16_t', 16),
        'uint32_t': CScalar('uint32_t', 32),
        'uint64_t': CScalar('uint64_t', 64),
    }
    for ast in asts:
        for t in ast:
            if isinstance(t, CCompound):
                defs[t._name] = t
            if isinstance(t, CTypedef) and isinstance(t._ctype, CCompound):
                defs[t._name] = t._ctype

    for t in defs:
        defs[t].resolve_defs(defs)
    return (defs, asts)
    
    