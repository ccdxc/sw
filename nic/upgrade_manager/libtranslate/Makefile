.PHONY = test clean

SRC:=$(wildcard *_test.py)
TEST:=$(SRC:%.py=%)

test: $(SRC)
	python -m unittest $(TEST)

clean:
	rm -rf *.pyc