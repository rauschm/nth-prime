/*------------------------------------------------------------------------------
  N T H - P R I M E . C

  Ausgabe der n-ten Primzahl

  Wenn nur ein Argument (n) angegeben wird, dann wird genau diese n-te Primzahl
  ausgegeben.
  Wenn zwei Argumente (n0,n1) angegeben werden und n0 <= n1 ist, dann werden alle
  Primzahlen von der n0-ten bis zu n1-ten ausgegeben.
  Wenn zwei Argumente (n,c) angegeben werden und n > c ist, dann werden die c
  Primzahlen vor der n-ten (einschließlich) ausgegeben.

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
typedef struct {
  unsigned long long int n_start;
  unsigned long long int n;
} Parameters;

typedef struct {
  unsigned int  width;
  unsigned int* data;
} Sieve;

/*------------------------------------------------------------------------------
 Prototypen
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv);
Parameters reinterprete_parameters(Parameters p);
void print_primes(unsigned long long int n0, unsigned long long int n);
Sieve build_sieve(unsigned int nth_prime_estimated_square_root);
void sieve_primes(unsigned long long int n_start, unsigned long long int n,
                  unsigned int nth_prime_estimated_square_root,
                  unsigned int sieve_width, unsigned int* sieve);
unsigned int estimate_square_root_of_nth_prime(unsigned long long int n);
unsigned long long int inverse_pi(unsigned long long int n);
unsigned int round_up_to_next_power_of_2(unsigned int v);

/*------------------------------------------------------------------------------
  Beginn der Verarbeitung 
------------------------------------------------------------------------------*/
void main(int argc, char* argv[]) {
  Parameters p = get_parameters(argc, argv);

  if (p.n_start == 1) {
    printf("1. prime = 2\n");
    if (p.n == 1) {
      exit(0);
    }
    p.n_start = 2;
  }

  print_primes(p.n_start, p.n);

  exit(0);
}

/*------------------------------------------------------------------------------
  n_start und n aus den Kommandozeilen-Parametern ermitteln
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv) {
  Parameters p;

  if (   argc != 2 && argc != 3
      || argc == 2 && (   strlen(argv[1]) > 19 || (p.n_start = p.n = atoll(argv[1])) < 1)
      || argc == 3 && (   strlen(argv[1]) > 19 || (p.n_start =       atoll(argv[1])) < 1
                       || strlen(argv[2]) > 19 || (            p.n = atoll(argv[2])) < 1)) {
     fprintf(stderr, "usage: nth-prime Number (in (0,1e19)) [Number (in (0,1e19))]\n");
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
    unsigned long long int count = p.n;
    p.n = p.n_start;
    p.n_start = p.n - (count - 1);
  }
  return p;
}

/*------------------------------------------------------------------------------
  Mit dem Sieb des Eratosthenes werden alle Primzahlen zwischen der n_start-ten
  und der n-ten ausgeben
------------------------------------------------------------------------------*/
void print_primes(unsigned long long int n_start, unsigned long long int n) {
  unsigned int nth_prime_estimated_square_root = estimate_square_root_of_nth_prime(n);
  Sieve sieve = build_sieve(nth_prime_estimated_square_root);
  sieve_primes(n_start, n, nth_prime_estimated_square_root, sieve.width, sieve.data);
}

/*------------------------------------------------------------------------------
  liefert die Quadratwurzel der Schätzung für die n-te Primzahl
  (die nächst kleinere ungerade ganze Zahl)
------------------------------------------------------------------------------*/
unsigned int estimate_square_root_of_nth_prime(unsigned long long int n) {
  unsigned long long int nth_prime_estimated = inverse_pi(n);
  if (nth_prime_estimated == 0) {
    fprintf(stderr, "value %llu too large\n", n);
    exit(2);
  }
  return ((unsigned int) sqrt((double) nth_prime_estimated) - 1) | 1;
}

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
unsigned long long int inverse_pi(unsigned long long int n) {
  if (n <= 2) {
    return n == 1 ? 2 : 3;
  }

  unsigned long long int x = 0;
  for (unsigned long long int delta = ~((~0ULL) >> 1); delta > 0; delta /= 2) {
    if ((x - delta) / log((double) (x - delta)) >= n) {
      x -= delta;
    }
  }
  return x;
}

/*------------------------------------------------------------------------------
  Ein Sieb wird aufgebaut, das ausreichend groß und mit Nullen initialisiert ist

  Speicher wird in einer Größenordnung der Wurzel der n-ten Primzahl benötigt.

  Abhängig vom verfügbaren Speicher beträgt der maximale Eingabewert:
  1 GB :    481.283.032.799.095 (32 Bit)
  2 GB:   1.856.377.398.776.709
  4 GB :  7.169.457.514.106.638
  8 GB : 27.721.902.337.570.149 (64 Bit)
------------------------------------------------------------------------------*/
Sieve build_sieve(unsigned int nth_prime_estimated_square_root) {
  Sieve sieve;
  const unsigned int max_root = 1024 / sizeof(sieve.data[0]) / 2
                              * ((sizeof(void*) == 4) ? 1 : 8) * 1024 * 1024 - 1;

  if (nth_prime_estimated_square_root > max_root) {
    fprintf(stderr, "value would need too much memory\n");
    exit(3);
  }

  sieve.width = round_up_to_next_power_of_2(nth_prime_estimated_square_root * 2);
  size_t sieve_size = sieve.width * sizeof(sieve.data[0]);
  sieve.data = malloc(sieve_size);
  if (sieve.data == NULL) {
    perror("memory error");
    exit(4);
  }
  memset(sieve.data, 0, sieve_size);
  return sieve;
}

/*------------------------------------------------------------------------------
  Hier werden mit dem Algorithmus des Eratosthenes alle Primzahlen ab 3 bis zur
  n-ten berechnet. Ab der n_start-ten Primzahl werden sie ausgegeben.
------------------------------------------------------------------------------*/
void sieve_primes(unsigned long long int n_start, unsigned long long int n,
                  unsigned int nth_prime_estimated_square_root,
                  unsigned int sieve_width, unsigned int* sieve) {
  unsigned long long int count_primes = 1;
  unsigned long long int number = 1;
  unsigned int i = 0;

  while (1) {
    i = (i + 1) & (sieve_width - 1);
    number += 2;
    unsigned int factor;
    if (sieve[i] == 0) {
      count_primes += 1;
      if (count_primes >= n_start) {
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
      if (sieve[j] == 0) {
        break;
      }
      if (sieve[j] < factor) {
        unsigned int smaller_factor = sieve[j];
        sieve[j] = factor;
        factor = smaller_factor;
      }
    } while (1);
    sieve[j] = factor;
  }
}

/*------------------------------------------------------------------------------
  zur nächsten Potenz von 2 aufrunden
------------------------------------------------------------------------------*/
unsigned int round_up_to_next_power_of_2(unsigned int v) {
  v -= 1;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v += 1;
  return v;
}
