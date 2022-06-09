# nth-prime
shows the nth prime number

If only one argument (n) is given, nth-prime just shows the nth prime number.

If there are two arguments (n,m) and n <= m then nth-prime shows all prime numbers starting with the nth prime number up to the mth prime number.
If there are two arguments (n,m) and n > m then nth-prime shows all prime numbers starting with the (n - m + 1)th prime number up to the nth prime number.

The code limits the memory usage to a little bit more than 1 GB (8 GB for 64 bit programs).
Because of that memory limitation, the argument value (the larger one) has to be at most 481.283.032.799.095 (27.721.902.337.570.149 for 64 bit).

Some interesting values:
203280221. prime = 4294967291 < 2<sup>32</sup> < 203280222. prime = 4294967311
