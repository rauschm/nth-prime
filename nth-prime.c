/*------------------------------------------------------------------------------
  N T H - P R I M E . C

  Ausgabe aller Primzahlen (< 2^64) zwischen der n_start-ten und der n-ten

  Wenn nur ein Argument (n) angegeben wird, dann wird genau diese n-te Primzahl
  ausgegeben.
  Wenn zwei Argumente (n0,n1) angegeben werden und n0 <= n1 ist, dann werden alle
  Primzahlen von der n0-ten bis zur n1-ten ausgegeben.
  Wenn zwei Argumente (n,c) angegeben werden und n > c ist, dann werden die c
  Primzahlen vor der n-ten (einschließlich) ausgegeben.

  Aufruf: nth-prime Nummer (> 0) [Nummer (> 0)] 

  Compile: cc -O2 -o nth-prime nth-prime.c -lm
     oder: cl /nologo /O2 /Fe: nth-prime.exe nth-prime.c
------------------------------------------------------------------------------*/
#include <errno.h>
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

typedef struct {
  uint64  width_mask;
  uint32* data;
} Sieve;

/*------------------------------------------------------------------------------
 Prototypen
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv);
Parameters reinterprete_parameters(Parameters p);
void print_primes(uint64 n);
void print_prime(uint64 prime_number);
Sieve build_sieve(uint32 sqrt_n);
void sieve_primes(uint64 n, uint32 sqrt_p, uint64 sieve_width_mask, uint32* sieve_data);
uint64 inverse_pi(uint64 n);
uint64 atoul(const char* s);
uint32 integer_square_root(uint64 x);
uint64 round_up_to_next_power_of_2(uint64 x);

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
  Wenn die Parameter nicht in der benötigten Form (Startwert,Endwert), sondern
  in der Form (Endwert,Anzahl) übergeben wurden, werden sie in die andere Form
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
  uint32 sqrt_p = odd(integer_square_root(p));
  Sieve sieve = build_sieve(sqrt_p);
  sieve_primes(n, sqrt_p, sieve.width_mask, sieve.data);
}

/*------------------------------------------------------------------------------
  Gibt die Primzahl und deren Nummer aus.

  Primzahlen < der n_start-ten werden nicht ausgegeben.
------------------------------------------------------------------------------*/
void print_prime(uint64 prime_number) {
  static uint64 primes_count = 0;
  primes_count += 1;
  if (primes_count >= n_start) {
    printf("%llu. prime = %llu\n", primes_count, prime_number);
  }
}

/*------------------------------------------------------------------------------
  Baut ein Sieb auf, das ausreichend groß und mit Nullen initialisiert ist.

  Speicher wird in einer Größenordnung von 2 mal der Wurzel von n benötigt.
------------------------------------------------------------------------------*/
Sieve build_sieve(uint32 sqrt_p) {
  Sieve sieve;

  uint64 sieve_width = round_up_to_next_power_of_2((uint64) sqrt_p * 2);
  uint64 sieve_size = sieve_width * sizeof(sieve.data[0]);
  sieve.width_mask = sieve_width - 1;
  
  if (   sieve_size > (size_t) sieve_size && (errno = ENOMEM)
      || (sieve.data = malloc((size_t) sieve_size)) == NULL) {
    perror("memory error");
    exit(3);
  }
  memset(sieve.data, 0, (size_t) sieve_size);
  return sieve;
}

/*------------------------------------------------------------------------------
  Berechnet mit dem Algorithmus des Eratosthenes alle Primzahlen ab 3 bis zur
  n-ten und gibt sie aus.
------------------------------------------------------------------------------*/
void sieve_primes(uint64 n, uint32 sqrt_p, uint64 sieve_width_mask, uint32* sieve_data) {
  uint64 count_primes = 1;
  uint64 i = 0;

  for (uint64 number = 3; count_primes < n; number += 2) {
    i = (i + 1) & sieve_width_mask;
    uint32 factor;
    if (sieve_data[i] == 0) {
      count_primes += 1;
      print_prime(number);
      if (number > sqrt_p) {
        continue;
      }
      factor = (uint32) number;
    } else {
      factor = sieve_data[i];
      sieve_data[i] = 0;
    }

    uint64 j = i;
    do {
      j = (j + factor) & sieve_width_mask;
      if (sieve_data[j] == 0) {
        break;
      }
      if (sieve_data[j] < factor) {
        uint32 smaller_factor = sieve_data[j];
        sieve_data[j] = factor;
        factor = smaller_factor;
      }
    } while (1);
    sieve_data[j] = factor;
  }
}

/*==============================================================================
  allgemeine Funktionen
==============================================================================*/

/*------------------------------------------------------------------------------
  inverse_pi(n) ist eine (obere) Abschätzung für n-te Primzahl.

  pi(x) ist die Funktion, die angibt, wie viele Primzahlen <= x es gibt.
  x/ln(x) ist eine Näherung für pi(x). Mit INV_xlnx als Umkehrfunktion gilt:

       x/ln(x)           < pi(x)       = n (für 10 < x < 2^64)
  <=>  INV_xlnx(x/ln(x)) < INV_xlnx(n)
  <=>  x                 < INV_xlnx(n)

  Die Schätzung ist mindestens 2% zu groß (ab n > 3).

  Wenn das Ergebnis zu groß ist, wird 0 zurückgegeben.

  Vermutlich aus Gründen der Rechenungenauigkeit ist der größte mögliche
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
  zur nächsten Potenz von 2 aufrunden
------------------------------------------------------------------------------*/
uint64 round_up_to_next_power_of_2(uint64 x) {
  x -= 1;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  x += 1;
  return x;
}
