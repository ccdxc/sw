

def get_byte(x, b):
    return (x & (0xff << b * 8)) >> b * 8


def get_bit(x, b):
    return (x & (1 << b)) >> b


def get_bits(x, s, e):
    assert (s <= e)
    return (x >> s) & ((1 << (e - s + 1)) - 1)


def set_bits(x, x_s, x_e, y, y_s, y_e):
    """
    Copy bits from y[y_s:y_e] into x[x_s:x_e]
    """
    assert (x_s <= x_e)
    assert (y_s <= y_e)
    assert (x_e - x_s == y_e - y_s)
    return x | (get_bits(y, y_s, y_e) << x_s)


def reverse_bytes(x):
    assert isinstance(x, int)
    # convert to hex-string and strip the '0x'
    s = hex(x)[2:]
    # prepend a zero if we have off number of digits
    s = '0' + s if len(s) % 2 != 0 else s
    assert len(s) % 2 == 0
    # split string every 2 characters to seperate into bytes
    s_bytes = [s[i:i + 2] for i in range(0, len(s), 2)]
    # reverse the bytes and join to create a byte string
    s_r = ''.join(s_bytes[::-1])
    # convert the byte swapped hex-string to integer and return
    return int('0x' + s_r, 16)
