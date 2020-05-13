# renju

This is an open source backgammon AI program developed by an individual, which implements the PVS and MTD search technology, and it can reach a good depth for everyone to learn from. If you have any suggestions, you can send it to my mailbox tydhhzc@163.cm .

If you feel good, a star is best reward for me.

checklists:

1. 简单的棋盘表示方法，我使用了二维的棋盘表示，而不是一维，否则可能为初学者带来认知上的困难。

2. 简单的控件逻辑，仅仅使用了Qt示例中的例子，同时我注意到退出程序时，qApp->quit();可能绕过了其他窗口的关闭逻辑，为此采取了一些措施，也许能够奏效。

3. 内存增强探测法（MTD, Memory-enhanced Test Driver）的一个简单例子。网络上能够搜索到许多信息，但并没有实在的实现，本项目可以作为一个学习的对象。

4. Zobrist哈希表，为MTD算法的效率提供保障。

5. 关于剪枝技术，本项目中没有采用杀手启发与历史启发，主要采用了棋型剪枝和ab算法的beta剪枝。