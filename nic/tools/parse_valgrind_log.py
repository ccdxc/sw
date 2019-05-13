import sys

filename = sys.argv[1]

def check_invalid_rd_wr():
    log = open(filename, "r")
    for line in log:
        if 'Invalid read of size' in line or 'Invalid write of size' in line:
            print('ERROR: Found Invalid read/write in ', filename)
            print(line)
            sys.exit(1)

def check_for_errors():
    check_invalid_rd_wr()

def main():
    check_for_errors()
    sys.exit(0)

main()
