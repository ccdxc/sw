import sys

filename = sys.argv[1]

def check_invalid_rd_wr():
    log = open(filename, "r")
    count = 0
    print_next_lines = False
    error = False
    for line in log:
        if 'Invalid read of size' in line or 'Invalid write of size' in line:
            print('ERROR: Found Invalid read/write in ', filename)
            print(line)
            error = True
            print_next_lines = True
            count = 0
        if print_next_lines is True:
            print(line);
            count = count + 1
        if (count >= 20):
            count = 0
            print_next_lines = False
    if error is True:
        sys.exit(1)

def check_for_errors():
    check_invalid_rd_wr()

def main():
    check_for_errors()
    sys.exit(0)

main()
