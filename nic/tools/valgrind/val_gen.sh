#!/usr/bin/env sh
valgrind --tool=memcheck --leak-check=yes --xml=yes --xml-file=hash_val.xml ../../obj/hash_test
valgrind --tool=memcheck --leak-check=yes --xml=yes --xml-file=tcam_val.xml ../../obj/tcam_test
valgrind --tool=memcheck --leak-check=yes --xml=yes --xml-file=indexer_val.xml ../../obj/indexer_test
valgrind --tool=memcheck --leak-check=yes --xml=yes --xml-file=met_val.xml ../../obj/met_test
# valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --xml=yes --xml-file=hash_val.xml ../../obj/hash_test
# valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --xml=yes --xml-file=tcam_val.xml ../../obj/tcam_test
# valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --xml=yes --xml-file=indexer_val.xml ../../obj/indexer_test
