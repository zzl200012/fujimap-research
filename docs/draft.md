## fujimap 调研报告

### 原理

对于传统的 hash map，无论使用 linear probe 还是 chaining 处理冲突，都会导致一定的空间浪费，尤其是在 key/value 尺寸较小的情况下。为了减少内存损耗，研究人员提出了最小完美哈希函数，即通过计算出特定的哈希函数，从而将 key/value 紧凑地存储在数组中而不浪费任何空间。然而，这个方法一个致命的缺陷是 key/value 数目和内容必须已知，因此在动态变化的场景下需要不停地重建最小完美哈希。一个直观的做法是设定一个固定大小的 buffer，每次 buffer 插满就对其所包含的 key/value 批量建立最小完美哈希，这正是 fujimap 的核心思想之一。

在此基础上，如果我们的应用场景允许出现 false positive（例如我们想用来减小点查的读放大，只需要对于存在的主键能返回正确的位置即可。就算返回了假阳结果，也只是多读一次实际数据的开销），还可以进一步引入近似计算技术来极端地压缩空间占用，同时提供比较可观的查询性能，当然代价是构建 succinct 结构的耗时相对比较长，并且如果 key/value set 太大，计算最小完美哈希的耗时也会很长，空间占用会比较高。

结合以上两点，fujimap 的原理就很容易理解了。首先自然是采用上文提到的 buffer hashtable 来缓存插入的 key/value pair，插满以后将整个 buffer 中所有 raw key/value 进行处理，去重后转化成有序的 edges，即后续计算最小完美哈希时用来构建 hypergraph 的中间结构。为了进一步缩减空间占用，fujimap 在 immutable part 中并不实际存储 key 值，因此整个空间占用只和 value 大小以及假阳率有关。构建最小完美哈希时，如果直接对整个 buffer 的所有数据进行计算，内存占用会非常大，并且耗时会比较长。fujimap 的做法是先用一个简单的哈希函数将数据分区，将分区数记为 BlockN。每次 buffer 满时，将每个 key/value pair 划分到某个 block 中，再对 BlockN 个分区分别构建最小完美哈希。

总结一下 fujimap 涉及到的技术：

* 分层结构。buffer 输入，批量转化成空间、查询友好的 immutable part；immutable part 新旧分层，查询时从新往旧查
* 近似计算。通过允许假阳性，极端压缩空间占用，同时保证较好的查询性能。从表现上来看，immutable part 类似一个能存 value 的 bloom filter
* 数据分区。为了避免过高的内存峰值占用和计算耗时，将一个 batch 的数据分区后分别构建最小完美哈希+succinct 结构，从而缓解压力。另一个角度来讲，分成若干个无依赖关系的 partition 在构建时也能具备更好的并行性

### 实现

#### 术语

* `Fujimap`：索引的主体，对外主要提供如下 API：
  * `setInteger`：insert or update 给定 key/value，key 为变长 string，value 为定长整型。可以指定输入是否需要立即可见
  * `setString`：insert or update 给定 key/value，key、value 为变长 string。实际是为 value 额外维护了一层 string -> hash 的映射，本质上插入 fujimap 的 value 还是定长整型
  * `getInteger`、`getString`：功能类似，先在内存的 buffer 中查找，如果找不到再到 blocks 中查找，还找不到则返回 `NOTFOUND`
  * `build`：把当前内存 buffer 中的 key/value pair 全部处理成 edges 并刷入 temp file 中，再对当前的 temp file 进行分区最小完美哈希构建。原则上内存 buffer 达到阈值后会自动触发该函数，除此之外只有用户手动调用来进行构建
  * `load`、`save`：全量序列化并持久化/读入全量索引数据并反序列化
* `KeyFile`：一个用来存放不需要立即可见的 key/value 的中间层。相比于插入内存 buffer，直接将 key/value 写入 key file 要更为快速，并且可以避免 build 时重新从 buffer 导入 key file（原版代码的逻辑是，所有 build 都以 key file 为 source，在 build 前会把 buffer 中所有的 key/value 先写入 key file）。从行为上可以理解为内存 buffer 的 spill part，主要目的是降低内存使用，让一次 build 尽可能包括更多数据
* `TmpEdges`：即上文提到的内存 buffer，本质上就是一个常驻内存的 hashtable，当然也可以根据 workload 替换其他任何支持点查的内存结构，或者其他 hashtable 变种
* `FujimapBlock`：fujimap 的核心结构，最小完美哈希+succinct 表示，包括一次 build 中的一个分区的 key/value。构建最小完美哈希时使用了[Simple and Space-Efficient Minimal Perfect Hash Functions](http://cmph.sourceforge.net/papers/wads07.pdf)中的方法，其他相关细节将在后续解析
* `KeyEdge`：用于构建 block 的一种中间结构，代表一个 key/value pair。具体来讲其中包括 value 本身以及额外 R 个 uint64 类型的标记位（R 是人为定义的，表示）
* `Batch`：这个并不是原版代码中的概念，为了后续讲解方便，用 batch 指代一次 build 过程中所有参与构建的数据，即 key file 中的所有分区；也可能指一次 build 产生的若干个（BlockN）block

#### 流程

##### 更新

Todo

<img src="./img/update.svg" width="50%" height="50%" />

##### 查询

Todo

<img src="./img/query.svg" width="50%" height="50%" />

### 性能

### 总结





