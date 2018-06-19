# Delphi Shared Memory

Delphi shared memory implementation is built in three layers on top of linux shared memory.

```
   +------------------+-------------------+
   |  Delphi Metrics  |  Delphi Objects   |
   |                  |   (in future)     |
   +------------------+-------------------+
   |           Key-Value store            |
   +--------------------------------------+
   |            Slab Allocator            |
   +--------------------------------------+
   |         Linux Shared Memory          |
   +--------------------------------------+

```

1. Bottom most layer is a simple slab allocator thats built on linux shared memory.
2. There is a Hash table / Key-value store built on top of the slab allocator.
3. Delphi Metrics API sits on top of the key-value store API. In future we might also store some delphi objects (like routes, endpoints etc) in the key value store.


### Slab Allocator

Shared memory slab allocator is a multi-thread/multi-process safe shared memory allocator that can be used to allocate shared memory space from multiple processes without needing a central coordinator. It uses shared memory spin-locks to protect some critical shared data-structures across processes.
This is roughly how the slab allocator works:

Its largely based on original slab allocator paper:
https://www.usenix.org/legacy/publications/library/proceedings/bos94/full_papers/bonwick.a

Entire shared memory space is broken down into equal sized pages typically
of length 64KB. Page-0 is reserved for shared memory metadata and allocator state.
Rest of the pages are used for allocating memory.

 Shared memory:
 ```
  +------------+------------+------------+------------+------------+---
  |   Page 0   |   Page 1   |   Page 2   |   Page 3   |   Page 4   | ..
  +------------+------------+------------+------------+------------+---
```

Each page is broken up into multiple equal sized chunks. This is called a slab.
At the end of the page, we maintain a slab info data structure that contains information
about all the chunks in the page.

 Slab (or page):
 ```
  +-----------+-----------+-----------+-----------+-----------+------------------+
  |  Chunk 1  |  Chunk 2  |  Chunk 3  |  Chunk 4  |  Chunk 5  |        Slab info |
  +-----------+-----------+-----------+-----------+-----------+------------------+
```

slab info keeps track of the chunk size, number of free chunks and a linked list
of currently available chunks. When an alloc request comes in, first chunk
from the free list of chunks is allocated and the pointer is returned

Shared memory metadata contains a memory pool data-structure. This data-structure
contains a freelist which is a linked list of slabs(i.e pages). Allocator also maintains
cache of slabs for various size of objects. These are called layers. Each layer contains
a linked list of slabs that are allocated to that layer. Currently there is a layer for
object sizes: 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 20480.
When an alloc request comes in, nearest layer thats bigger than the required length
is picked and a chunk is allocated from that layer. If the layer has no free chunk,
new page is allocated to that layer from the free list of slabs.

Slab Pool info:
```
 +------------------+
 |  Pool info       |
 +------------------+
 |                  |     +--------+     +--------+
 |   Freelist       +---->+ Slab 1 +---->+ Slab 2 |
 |                  |     +--------+     +--------+
 +------------------+
 |                  |     +--------+     +--------+
 |    Layer 1       +---->+ Slab 3 +---->+ Slab 4 |
 |                  |     +--------+     +--------+
 +------------------+
 |                  |     +--------+     +--------+
 |    Layer 2       +---->+ Slab 5 +---->+ Slab 6 |
 |                  |     +--------+     +--------+
 +------------------+
 |    Layer ...     |
 +------------------+
```

### Key-value store

Shared memory key-value store is implemented as a two level hash table. Shared memory hash table is multi-thread and multi-process safe. Its optimized for single writer and multiple reader per hash table. It maintains reader ref-count per hash entry so that the memory is not freed while a reader is still accessing it.

This is the hash table structure:

```
    Hash Table
  +--------------+
  |  htable_t    |
  +--------------+          Hash Entry
  |  bucket 0    +------->+-------------------+
  +--------------+        |        Next       +--->
  |  bucket 1    |        +-------------------+
  +--------------+        | key len | val len |
  |    ......    |        +-------------------+
  +--------------+        |       key         |
  |  bucket N    |        |      .....        |
  +--------------+        +-------------------+
                          |  Ref count        |
                          +-------------------+
                          |       Value       |
                          |       ....        |
                          +-------------------+
```

A hash table can have up to 5K buckets in the first level buckets that are
embedded into the hash table(5Kx4 = 20KB is the largest chunk size we can allocate in shared memory).
When a hash table needs to grow bigger than 5K buckets, hash table creates
second level buckets. In this case, first level buckets become pointers to
second level buckets(called tablets).

```
  +--------------+
  |  htable_t    |
  +--------------+                        Tablet 0
  |  tablet 0    +--------------------->+----------+
  +--------------+       Tablet 1       | Bucket 0 |
  |  tablet 1    +-->+--------------+   +----------+
  +--------------+   | Bucket M + 1 |   | Bucket 1 |
  |    ......    |   +--------------+   +----------+
  +--------------+   | Bucket M + 2 |   | .....    |
  |  tablet N    |   +--------------+   +----------+
  +--------------+   |   .....      |   | Bucket M |
                     +--------------+   +----------+

```

maximum number of buckets in tablet is also 5K. This means, maximum
size of hash table is (5K * 5K) = 25 million hash buckets.

There are two levels of hash table in key-value store. First one for looking up kinds and second one to lookup key.

```
   Root Table
  +-----------+                                           Kind-1 Table
  |  Kind-1   +----------------------------------------->+-----------+
  +-----------+                       Kind-2 Table       |   Key  1  |
  |  Kind-2   +--------------------->+-----------+       +-----------+
  +-----------+     Kind-3 Table     |   Key 1   |       |   ....    |
  |  Kind-3   +--->+-----------+     +-----------+
  +-----------+    |   Key 1   |     |   ....    |
  |  Kind 4   |    +-----------+
  +-----------+    |  Key 2    |
  |  Kind 5   |    +-----------+
  +-----------+    |  ....     |
  |  .......  |
  +-----------+

```

### Delphi Metrics API

Delphi metrics API provides convenient wrappers for creating metrics objects and modifying counters, gauges and histograms. Most of this code is generated by delphi compiler for each kind of metrics. Please see Delphi developer guide for more details.
