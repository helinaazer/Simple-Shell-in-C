**UNIX Shell**

This project consists of designing a C program to serve as a shell interface that accepts user commands and then executes each command in a separate process.  Your implementation will support input and output redirection, as well as pipes as a form of IPC between a pair of commands.  Completing this project will involve using the UNIX fork(), exec(), wait(), dup2(), and pipe() system calls and can be completed on any Linux, UNIX, or macOS system.

However, it will be tested in the Linux lab so you should make sure your shell.c works there.

**This project is organized into several parts:**

1. Creating the child process and executing the command in the child
2. Providing a history feature
3. Adding support of input and output redirection
4. Allowing the parent and child processes to communicate via a pipe
