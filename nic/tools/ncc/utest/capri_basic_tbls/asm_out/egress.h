

/*
 * egress.h
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains assembly level data structures for all Egress processing
 * needed for MPU to read and act on action data and action input. 
 *
 * Every Egress P4table after match hit can optionally provide
 *   1. Action Data (Parameters provided in P4 action functions)
 *   2. Action Input (Table Action routine using data extracted
 *                    into PHV either from header or result of
 *                    previous table action stored in PHV)
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

/* TBD: In HBM case actiondata need to be packed before and after Key Fields
 * For now all actiondata follows Key 
 */

