/*------------------------------------------------------------------------------
  N T H - P R I M E . C

  Ausgabe der n-ten Primzahl
  
  Wenn nur ein Argument (n) angegeben wird, dann wird genau diese n-te Primzahl
  ausgegeben.
  Wenn zwei Argumente (n0,n) angegeben werden und n0 <= n ist, dann werden alle
  Primzahlen von der n0-ten bis zu n-ten ausgegeben.
  Wenn zwei Argumente (n,n0) angegeben werden und n > n0 ist, dann werden die n0
  Primzahlen vor der n-ten (einschließlich) ausgegeben.

  Aufruf: nth-prime Nummer (> 0) [Nummer (> 0)] 

  Compile: cc -O2 -lm -o nth-prime nth-prime.c
     oder: cl /nologo /O2 /Fe: nth-prime.exe nth-prime.c
------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define round_up_to_next_power_of_2(v) \
  { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; }

void sieve_primes(unsigned long long int n0, unsigned long long int n);
unsigned long long int inverse_pi(unsigned long long int n);

/*------------------------------------------------------------------------------
  Beginn der Verarbeitung 
------------------------------------------------------------------------------*/
void main(int argc, char* argv[]) {
  unsigned long long int n0, n;
 
  if (   argc != 2 && argc != 3
      || argc == 2 && (n = 1, n0 = atoll(argv[1])) < 1
      || argc == 3 && ((n0 = atoll(argv[1])) < 1 || (n = atoll(argv[2])) < 1)) {
     fprintf(stderr, "usage: nth-prime Number (>0) [Number (>0)]\n");
     exit(1);
  }

  if (n < n0) {
    n0 = n0 - (n - 1);
    n  = n0 + (n - 1);
  }
 
  if (n0 == 1) {
    printf("1. prime = 2\n");
    n0 += 1;
  }

  if (n0 <= n) {
    sieve_primes(n0, n);
  }

  exit(0);
}

/*------------------------------------------------------------------------------
  Mit dem Sieb des Eratosthenes alle Primzahlen zwischen der n0-ten und der
  n-ten ausgeben
------------------------------------------------------------------------------*/
void sieve_primes(unsigned long long int n0, unsigned long long int n) {
  unsigned long long int nth_prime_estimated = inverse_pi(n);
  if (nth_prime_estimated == 0) {
    fprintf(stderr, "value %llu too large\n", n);
    exit(2);
  }
  unsigned int nth_prime_estimated_square_root = (unsigned int) sqrt((double) nth_prime_estimated);
  unsigned int* sieve;
  unsigned int sieve_width = ((nth_prime_estimated_square_root + 1) / 2) * 2 * 3;
  round_up_to_next_power_of_2(sieve_width);
  size_t sieve_size = sieve_width * sizeof(sieve[0]);
  sieve = malloc(sieve_size);
  if (sieve == NULL) {
    perror("memory error");
    exit(3);
  }
  memset(sieve, 0, sieve_size);

  unsigned long long int count_primes = 1;
  unsigned long long int number = 1;
  unsigned int i = 0;
  while (1) {
    i = (i + 1) & (sieve_width - 1);
    number += 2;
    unsigned int factor;
    if (sieve[i] == 0) {
      count_primes += 1;
      if (count_primes >= n0) {
        printf("%llu. prime = %llu\n", count_primes, number);
        if (count_primes == n) {
          return;
        }
      }
      if (number > nth_prime_estimated_square_root) {
        continue;
      }
      factor = (unsigned int) number;
    } else {
      factor = sieve[i];
      sieve[i] = 0;
    }
    unsigned int j = i;
    do {
      j = (j + factor) & (sieve_width - 1);
    } while (sieve[j] != 0);
    sieve[j] = factor;
  }
}

/*------------------------------------------------------------------------------
  inverse_pi(n) ist eine (obere) Abschätzung für n-te Primzahl.

  pi(x) ist die Funktion, die angibt, wie viele Primzahlen <= x es gibt.
  x/ln(x) ist eine Näherung für pi(x). Dabei gilt:
    x/ln((x) * 0.95 <= pi(x) <= x/ln(x) * 1.05
  Wenn das Ergebnis zu groß ist, wird 0 zurückgegeben.

  Vermutlich aus Gründen der Rechenungenauigkeit ist der größte mögliche
  Eingabewert nur = 395.037.107.592.253.344.
  In diesem Fall ist der Ausgabewert = 18.446.744.073.709.548.545 (knapp 2^64).

  Um mit 4 GB Speicher auszukommen, wird die maximal berechnete Größe beschränkt.
  Der jetzt größte mögliche Eingabewert = 3.101.005.152.064.519.
------------------------------------------------------------------------------*/
unsigned long long int inverse_pi(unsigned long long int n) {
  if (n <= 3) {
    return n == 3 ? 5
         : n == 2 ? 3
         :          2;
  }

  unsigned long long int x = 0;
  for (unsigned long long int delta = ~((~0ULL) >> 1); delta > 0; delta /= 2) {
    if ((x - delta) / log((double) (x - delta)) >= n) {
      x -= delta;
    }
  }
  x += x / 20;
  return (x <= 128102389162151481ULL) ? x : 0;
}
