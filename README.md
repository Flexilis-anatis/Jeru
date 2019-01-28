# Jeru
### A Forth-based interpreted language

A toy language I'm making. Here's an example of a fibbonacci calculator!

```Forth
[
    [
        copy 1 - fibo *
    ] copy 1 > if
] word fibo
```
Alt. without newlines:
```Forth
[ [ copy 1 - fibo * ] copy 1 > if ] word fibo
```
If you called this word with `3 fibo` the stack would progress as follows:
```Python
[3] #enter into fibo
[3, 3] #copy
[3, 3, 1] #push 1
[3, 1] #test >
[3] #run code block
[3, 3] #copy
[3, 3, 1] #push 1
[3, 2] #subtract
[3, 2, 2] #re-enter into fibo: copy
[3, 2, 2, 1] #push 1
[3, 2, 1] #test >
[3, 2] #enter code block
[3, 2, 2] #copy
[3, 2, 2, 1] #push 1
[3, 2, 1] #subtract
[3, 2, 1, 1] #re-enter into fibo: copy
[3, 2, 1, 1, 1] #push 1
[3, 2, 1, 0] #test >
[3, 2, 1] #do NOT run code block: multiply
[3, 2] #multiply
[6] #resul!
```

This is approx. equivilent to the following python code:

```Python
def fibo(n):
    if n > 1:
        return fibo(n-1) * n
    return 1
```

Alt. without newlines:

```Python
fibo = lambda n : fibo(n-1) * n if n > 1 else 1
```

A little backwards, I know, but it's Forth! :D
