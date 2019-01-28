# Jeru
### A Forth-based interpreted language

A toy language I'm making. Here's an example of a fibbonacci calculator! (disclaimer: doesn't work yet)

```Forth
[
    [
        copy 1 - fibo *
    ] 2 < if
] word fibo
```
Alt. without newlines:
```Forth
[ [ copy 1 - fibo * ] 2 < if ] word fibo
```
If you called this word with `3 fibo` the stack would progress as follows:
```Python
[3] # about to call fibo
[3, 3, 1] # copys top of stack and pushes one
[3, 2] # subtracts one from the stack
[3, 2, 2, 1] # calls fibo AGAIN and repeats the last two steps
[3, 2, 1] # if doesn't fire this time. does nothing
[3, 2, 1] # multiplies 2*1
[3, 2] # multiplies 3*2
[6] # result!
```

This is approx. equivilent to the following python code:

```Python
def fibo(n):
    if n < 2: # or n <= 1
        return 1
    return fibo(n-1) * n
```

Alt. without newlines:

```Python
fibo = lambda n : 1 if n < 2 else fibo(n-1) * n
```
