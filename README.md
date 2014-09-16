# 技术之路 - 无锁

标签：技术 Linux 开发 程序员

[TOC]

---

**CAS**，Compare - And - Swap，是一种操作。
具体到不同架构，有不同的实现方式，例如Intel的cmpxchg8指令。

"**无锁**", Lock-free，并不是真正意义上的无锁算法，只是加锁层级中的一层：

![加锁层级图][1]

图中标注为红色字体的方案为 Blocking synchronization，黑色字体为 Non-blocking synchronization。Lock-based 和 Lockless-based 两者之间的区别仅仅是加锁粒度的不同。图中最底层的方案就是大家经常使用的 mutex 和 semaphore 等方案，代码复杂度低，但运行效率也最低。

“**自旋锁**”，无锁算法的一种，获取锁失败时不挂起，而是CPU死循环空转等待。适用场景：锁持有时间小于将一个线程阻塞和唤醒所需时间的场合。引入pthread库即可。

除了自旋锁外，这里还有其他三种无锁算法：读写锁、顺序锁seqlock、RCU，分别适用其他多读多写、读多谢少、读写可以并行的场景。

内核无锁第四层级 — **免锁** - ring buffer就是一种，在一读一写的条件下，可以做到绝对无锁。
![环形缓冲区实现原理图][2]

FIFO的内核实现用的是Ring buffer?

  [1]: http://www.ibm.com/developerworks/cn/linux/l-cn-lockfree/image001.jpg "加锁层级图"
  [2]: http://www.ibm.com/developerworks/cn/linux/l-cn-lockfree/image002.jpg "环形缓冲区实现原理图"
  
  冲突检测机制基本也就是CAS，这里CAS被称之为乐观锁，与之相对应的肯定还有悲观锁，也就是不管三七二十一，先把锁加了再说，可想而知，性能肯定不是不行的。
  冲突解决机制，基本就是：Retry。也很简单。
