# API gtests

Each API is tested independently and combinations of different kinds of API
are tested under DOL/IOTA. All the workflows described below deal with one
kind of API at a time.

* Legend:
    * Workflow : represents a testcase
    * Workflow B<1..> : Basic work flows, tests deal with one object at at time
    * Workflow <1...> : Postive test cases
    * Workflow N<1..> : Negative test cases
    * "[ ]" represents a batch containing api objects
    * Create/Delete/Read/Update : CRUD operations on the API
    * One : one object
    * Set : group of objects (Ex: Set1, Set2, Set3 are mutually exclusive sets
        of objects)
    * SetMax : max set of objects supported for a given API

Read operation is always called after every batch to sanitize the current state
of the system

## Workflow B1
    [ Create One ] - Read - [ Delete One ] - Read

## Workflow B1
    [ Create One ] - Read - [ Update One ] - Read

## Workflow 1
    [ Create SetMax - Delete SetMax ] - Read

## Workflow 2
    [ Create SetMax - Delete SetMax - Create SetMax ] - Read

## Workflow 3
    [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read

## Workflow 4
    [ Create SetMax ] - Read - [ Delete SetMax ] - Read

## Workflow 5
    [ Create Set1, Set2 ] - Read - [ Delete Set1 - Create Set3 ] - Read

## Workflow 6
    [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ] - Read

## Workflow 7
    [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax] - Read -
        [ Update SetMax ] - Read

## Workflow 8
    [ Create SetMax - Update SetMax ] - Read - [ Update SetMax ] - Read -
        [ Delete SetMax ] - Read

## Workflow 9
    [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read

## Workflow 10
    [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read -
        [ Update Set3 - Delete Set2 - Create Set4 ] - Read

## Workflow N1
    [ Create SetMax ] - [ Create SetMax ] - Read

## Workflow N2
    [ Create SetMax+1 ] - Read

## Workflow N3
    Read NonEx - [ Delete NonExMax ] - Read - [ Update NonExMax ] - Read

## Workflow N4
    [ Create Set1 ] - Read - [ Delete Set1, Set2 ] - Read

## Workflow N5
    [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read

## Workflow N6
    [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read

## Workflow N7
    [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read

## Workflow N8
    [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
