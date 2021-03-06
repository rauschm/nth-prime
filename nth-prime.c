/*------------------------------------------------------------------------------
  N T H - P R I M E . C

  Ausgabe aller Primzahlen (< 2^64) zwischen der n_start-ten und der n-ten

  Wenn nur ein Argument (n) angegeben wird, dann wird genau diese n-te Primzahl
  ausgegeben.
  Wenn zwei Argumente (n0,n1) angegeben werden und n0 <= n1 ist, dann werden alle
  Primzahlen von der n0-ten bis zur n1-ten ausgegeben.
  Wenn zwei Argumente (n,c) angegeben werden und n > c ist, dann werden die c
  Primzahlen vor der n-ten (einschlie?lich) ausgegeben.

  Aufruf: nth-prime Nummer (> 0) [Nummer (> 0)] 

  Compile: cc -O2 -o nth-prime nth-prime.c -lm
     oder: cl /nologo /O2 /Fe: nth-prime.exe nth-prime.c
------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------------
  Datentypen
------------------------------------------------------------------------------*/
typedef unsigned long long int uint64;
typedef unsigned int           uint32;

typedef struct {
  uint64 n_start;
  uint64 n;
} Parameters;

/*------------------------------------------------------------------------------
  Prototypen
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv);
Parameters reinterprete_parameters(Parameters p);
void print_primes(uint64 n);
void print_prime(uint64 prime_number);
uint32 calc_square_roots(uint64 n, uint32* sqrts);
uint32* build_primes(uint32 prime_factors_count_estimated);
char* build_sieve(uint32 sqrt_n);
uint32 calc_prime_factors(uint32 sqrts_top, uint32* sqrts, uint32* primes, char* sieve);
void calc_remaining_primes(uint64 n, uint32 sqrt_n, uint32 primes_top, uint32* primes, char* sieve);
uint64 inverse_pi(uint64 n);
uint64 atoul(const char* str);
uint32 integer_square_root(uint64 x);
uint32 estimate_number_of_primes_up_to(uint32 x);

/*------------------------------------------------------------------------------
  globale Variablen
------------------------------------------------------------------------------*/
uint64 n_start;

/*------------------------------------------------------------------------------
  Macros
------------------------------------------------------------------------------*/
#define odd(n) ((n - 1) | 1)

/*------------------------------------------------------------------------------
  Beginn der Verarbeitung
------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
  Parameters p = get_parameters(argc, argv);
  n_start = p.n_start;
  print_primes(p.n);
  return 0;
}

/*------------------------------------------------------------------------------
  n_start und n aus den Kommandozeilen-Parametern ermitteln
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv) {
  Parameters p;

  if (   argc != 2 && argc != 3
      || argc == 2 && (   (p.n_start = p.n = atoul(argv[1])) < 1)
      || argc == 3 && (   (p.n_start =       atoul(argv[1])) < 1
                       || (            p.n = atoul(argv[2])) < 1)) {
     fprintf(stderr, "usage: nth-prime Number (in (0,2^64)) [Number/Count (in (0,2^64))]\n");
    exit(1);
  }
  p = reinterprete_parameters(p);
  return p;
}

/*------------------------------------------------------------------------------
  Wenn die Parameter nicht in der ben?tigten Form (Startwert,Endwert), sondern
  in der Form (Endwert,Anzahl) ?bergeben wurden, werden sie in die andere Form
  umgerechnet.
------------------------------------------------------------------------------*/
Parameters reinterprete_parameters(Parameters p) {
  if (! (p.n_start <= p.n)) {
    uint64 count = p.n;
    p.n = p.n_start;
    p.n_start = p.n - (count - 1);
  }
  return p;
}

/*------------------------------------------------------------------------------
  Gibt alle Primzahlen bis zur n-ten aus.
------------------------------------------------------------------------------*/
void print_primes(uint64 n) {
  uint64 p = inverse_pi(n);
  if (p == 0) {
    fprintf(stderr, "value %llu too large\n", n);
    exit(2);
  }

  print_prime(2);
  if (n == 1) {
    return;
  }

  uint32 sqrts[5];
  uint32 sqrts_top = calc_square_roots(p, sqrts);
  uint32 sqrt_p = sqrts[0];

  uint32 prime_factors_count_estimated = estimate_number_of_primes_up_to(sqrt_p);

  uint32* primes = build_primes(prime_factors_count_estimated);
  char* sieve = build_sieve(sqrt_p);

  uint32 primes_top = calc_prime_factors(sqrts_top, sqrts, primes, sieve);
  if (n > 2) {
    calc_remaining_primes(n, sqrt_p, primes_top, primes, sieve);
  }
}

/*------------------------------------------------------------------------------
  Gibt eine Primzahl und deren Nummer aus.

  Zahlen deren Nummer kleiner als n_start ist, werden nicht ausgegeben.
------------------------------------------------------------------------------*/
void print_prime(uint64 prime_number) {
  static uint64 primes_count = 0;
  primes_count += 1;
  if (primes_count >= n_start) {
    printf("%llu. prime = %llu\n", primes_count, prime_number);
  }
}

/*------------------------------------------------------------------------------
  Berechnet alle ungeraden Quadratwurzeln von p, solange bis der Wert 3 erreicht.
  Statt 1 (== sqrt(3..8)) wird 3 verwendet. 
  Zur?ckgegeben wird der Index der kleinsten (= Anzahl - 1).
------------------------------------------------------------------------------*/
uint32 calc_square_roots(uint64 p, uint32* sqrts) {
  uint32 top = -1;
  do {
    sqrts[++top] = p = odd(integer_square_root(p));
  } while (p > 3);
  if (sqrts[top] == 1) {
    sqrts[top] = 3;
  }
  return top;
}

/*------------------------------------------------------------------------------
  Baut ein Array f?r die Primfaktoren auf, das ausreichend gro? ist.
------------------------------------------------------------------------------*/
uint32* build_primes(uint32 prime_factors_count_estimated) {
  uint32* primes;
  size_t primes_size = sizeof(primes[0]) * prime_factors_count_estimated;
  if ((primes = malloc(primes_size)) == NULL) {
    perror("memory error");
    exit(3);
  }
  return primes;
}

/*------------------------------------------------------------------------------
  Baut ein Sieb auf, das ausreichend gro? und mit Nullen initialisiert ist.

  Speicher wird in einer Gr??enordnung der Wurzel von n ben?tigt.
------------------------------------------------------------------------------*/
char* build_sieve(uint32 sqrt_p) {
  char* sieve;
  size_t sieve_size = sizeof(sieve[0]) * sqrt_p;  
  if ((sieve = malloc(sieve_size)) == NULL) {
    perror("memory error");
    exit(4);
  }
  memset(sieve, 0, sieve_size);
  return sieve;
}

/*------------------------------------------------------------------------------
  Berechnet alle ungeraden Primzahlen <= sqrt(p).
  Die Primzahlen werden auch ausgegeben.
  Zur?ckgegeben wird der Index der gr??ten Primzahl (= Anzahl - 2).
------------------------------------------------------------------------------*/
uint32 calc_prime_factors(uint32 sqrts_top, uint32* sqrts, uint32* primes, char* sieve) {
  uint32 primes_top = 0;
  primes[0] = 3;
  print_prime(3);

  while (sqrts_top > 0) {
    sqrts_top -= 1;

    /* Nicht-Primzahlen markieren */
    for (uint32 i = 0; i <= primes_top; i++) {
      for (uint32 j = (sqrts[sqrts_top + 1] - 3) / 2 + primes[i]
                    - (sqrts[sqrts_top + 1] - primes[i]) / 2 % primes[i];
           j <= (sqrts[sqrts_top] - 3) / 2;
           j += primes[i]) {
        sieve[j] = 1;
      }
    }

    /* Primzahlen notieren und ausgeben */
    for (uint32 j = (sqrts[sqrts_top + 1] - 1) / 2; j <= (sqrts[sqrts_top] - 3) / 2; j++) {
      if (sieve[j] == 0) {
        primes[++primes_top] = j * 2 + 3;
        print_prime(primes[primes_top]);
      } else {
        sieve[j] = 0;
      }
    }
  }

  return primes_top;
}

/*------------------------------------------------------------------------------
  Berechnet alle ungeraden Primzahlen > sqrt(p) und <= der n-ten.
  Die Primzahlen werden auch ausgegeben.
------------------------------------------------------------------------------*/
void calc_remaining_primes(uint64 n, uint32 sqrt_p, uint32 primes_top, uint32* primes, char* sieve) { 
  uint64 count_primes = primes_top + 2;

  for (uint64 z = 2ULL + sqrt_p; ; z += 2ULL * sqrt_p) {

    /* Nicht-Primzahlen markieren */
    for (uint32 i = 0; i <= primes_top; i++) {
      uint32 j = primes[i] - 1 - (z - 2 - primes[i]) % (primes[i] * 2) / 2;
      while (1) {
        sieve[j] = 1;
        if (j >= sqrt_p - primes[i]) {
          break;
        }
        j += primes[i];
      }
    }
    
    /* Primzahlen notieren und ausgeben */
    for (uint32 j = 0; j < sqrt_p; j++) {
      if (sieve[j] == 0) {
        print_prime(z + 2ULL * j);
        count_primes += 1;
        if (count_primes >= n) {
          return;
        }
      } else {
        sieve[j] = 0;
      }
    }
  }
}

/*==============================================================================
  allgemeine Funktionen
==============================================================================*/

/*------------------------------------------------------------------------------
  inverse_pi(n) ist eine (obere) Absch?tzung f?r n-te Primzahl.

  pi(x) ist die Funktion, die angibt, wie viele Primzahlen <= x es gibt.
  x/ln(x) ist eine N?herung f?r pi(x). Mit INV_xlnx als Umkehrfunktion gilt:

       x/ln(x)           < pi(x)       = n (f?r 10 < x < 2^64)
  <=>  INV_xlnx(x/ln(x)) < INV_xlnx(n)
  <=>  x                 < INV_xlnx(n)

  Die Sch?tzung ist mindestens 2% zu gro? (ab n > 3).

  Wenn das Ergebnis zu gro? ist, wird 0 zur?ckgegeben.

  Vermutlich aus Gr?nden der Rechenungenauigkeit ist der gr??te m?gliche
  Eingabewert nur = 415.828.534.307.635.104.
  In diesem Fall ist der Ausgabewert = 18.446.744.073.709.548.545 (knapp 2^64).
------------------------------------------------------------------------------*/
uint64 inverse_pi(uint64 n) {
  if (n <= 2) {
    return n == 1 ? 2 : 3;
  }

  uint64 x = 0;
  for (uint64 delta = ~((~0ULL) >> 1); delta > 0; delta /= 2) {
    if ((x - delta) / log((double) (x - delta)) >= n) {
      x -= delta;
    }
  }
  return x;
}

/*------------------------------------------------------------------------------
  convert a string to an unsigned long integer
------------------------------------------------------------------------------*/
uint64 atoul(const char* str) {
  uint64 ull = 0;

  while (*str != 0) {
    if ((*str < '0' || *str > '9')
      || ull > 1844674407370955161ULL
      || (ull *= 10) > 18446744073709551615ULL - (*str - '0')) {
      return 0;
    }
    ull += (*str++ - '0');
  }
  return ull;
}

/*------------------------------------------------------------------------------
  Berechnet ISQRT = die ganzzahlige 32-Bit Qudratwurzel einer 64-Bit-Zahl.
  Es gilt: ISQRT^2 <= x.
------------------------------------------------------------------------------*/
uint32 integer_square_root(uint64 x) {
  uint32 y = (uint32) sqrt((double) x);
  return ((uint64) y * (uint64) y <= x) ? y : (y - 1);
}

/*------------------------------------------------------------------------------
  Berechnet eine Absch?tzung EPRIM f?r die Anzahl der Primzahlen <= x.
  Es gilt: EPRIM >= pi(x)
------------------------------------------------------------------------------*/
uint32 estimate_number_of_primes_up_to(uint32 x) {
  return (uint32) (158 + (double) x
                         / (log(x) - 1.052400915 - (log(4294967295U) - log(x))
                                                 * 0.08149));
//return (uint32) (158 + (double) x / (log(x) * 1.08149 - 2.859906955));
}
