# nth-prime
shows the nth prime number

If only one argument (n) is given, nth-prime just shows the nth prime number.

If there are two arguments (n,m) and n <= m then nth-prime shows all prime numbers starting with the nth prime number up to the mth prime number.
If there are two arguments (n,m) and n > m then nth-prime shows all prime numbers starting with the (n - m + 1)th prime number up to the nth prime number.

The code limits the memory usage to a little bit more than 4 GB. So the program has to be a 64 bit program.
Because of that memory limitation, the argument value (the larger one) has to be at most 3.101.005.152.064.519.
