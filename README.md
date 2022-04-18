# TinyThread

A M:N user thread library that consist of coroutine and task scheduling

# Lessons

Initially, i was trying to build a M:N thread library with shared stack coroutine and continuation-stealing scheduler.

But later i found there was a big design error when i was trying to steal a coroutine from other workers. We need to resume the stack on current worker in order to launch it at current thread, but the stack/execution context has already embedded many address that is related to previous worker. So it's hard for us to migrate the context perfectly from one context to another. Later i decided to use the independent stack which is easy to manage and finally finished the implementation. Still, the shared stack is avaliable when using just one worker, so you can refer my code to build your own coroutine library.

I didn't apply much optimizations while writing the code, such as cache-line aware, memory pool etc. You can try it yourself if you have interest on it.

Code is simple, so i would recommend newbie to learn the code to see how coroutine is implemented. If you have new ideas on implementing such a library, i'm glad to have a discussion with you.

Happy coding!

# benchmark

![20220418142747](https://picsheep.oss-cn-beijing.aliyuncs.com/pic/20220418142747.png)

I didn't optimize this very deeply, such as optimization on wait group, and dynamic memory allocation

Also, since i'm not using shared stack, i think the space consumption would be huge

But i believe with some optimization on memory management and cache-line aware partitioning, we could reach linear speed up in this algorithm