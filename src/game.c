#include "game.h"

extern task_struct cur_task;

void printa()
{
    while(1)
        printf("A");
}

void printb()
{
    while(1)
        printf("B");
}

void printaandb()
{
    int task_create(void* start_routin, uint32_t counter, uint32_t priority, uint32_t tty, uint32_t father, int argc, char **argv);
    task_create(printa, 5, 0, 0, cur_task.pid, 0, NULL);
    task_create(printb, 5, 0, 0, cur_task.pid, 0, NULL);
}

// -----------------------------------------------------------------------------------
// 汉诺塔
#define H 7

typedef struct { int *x, n; } tower;
static tower *new_tower(int cap) {
  int size = sizeof(tower) + sizeof(int) * cap;
  tower *t = page_alloc(1);
  memset(t, 0, size);
  t->x = (int*)(t + 1);
  return t;
}

static tower *t[3];

static void text(int y, int i, int d, const char *s) {
  int yy = H - y + 1;
  int xx = (H + 1) * (2 * i + 1) - d;
  while (d--) {
    for (const char *p = s; *p; p ++) {
      print_char(*p, yy, xx ++);
    }
  }
}

static void add_disk(int i, int d) {
  t[i]->x[t[i]->n++] = d;
  text(t[i]->n, i, d, "==");
  screen_refresh();
  usleep(100000);
}

int remove_disk(int i) {
  int d = t[i]->x[--t[i]->n];
  text(t[i]->n + 1, i, d, "  ");
  return d;
}

void hmove(int n, int from, int to, int via) {
  if (!n) return;

  hmove(n - 1, from, via, to);
  add_disk(to, remove_disk(from));
  hmove(n - 1, via, to, from);
}

void hanoi() {
    
  screen_clear();

  int c;
  for (c = 0; c < 3; c++)	 t[c] = new_tower(H);
  for (c = H; c; c--) add_disk(0, c);

  hmove(H, 0, 2, 1);
}

// -----------------------------------------------------------------------------------
// 生命游戏
static int w = 40, h = 25;
static unsigned *new_array = NULL;
static unsigned *univ_array = NULL;

void show(void *u) {
  int x,y;
  int (*univ)[w] = u;
  screen_clear();
  for (y=0;y<h;y++) {
    for (x=0;x<w;x++)
      print_char(univ[y][x] ? 'o' : ' ', y, x);
  }
  screen_refresh();
}

void evolve(void *u) {
  int x,y,x1,y1;
  unsigned (*univ)[w] = u;
  unsigned (*new)[w] = (void *)new_array;

  for (y=0;y<h;y++) for (x=0;x<w;x++) {
    int n = 0;
    for (y1 = y - 1; y1 <= y + 1; y1++)
      for (x1 = x - 1; x1 <= x + 1; x1++)
        if (univ[(y1 + h) % h][(x1 + w) % w])
          n++;

    if (univ[y][x]) n--;
    new[y][x] = (n == 3 || (n == 2 && univ[y][x]));
  }
  for (y=0;y<h;y++) for (x=0;x<w;x++) univ[y][x] = new[y][x];
}

void game_of_life(void) {
  w = screen_tile_width();
  h = screen_tile_height();
  univ_array = malloc(h * w * sizeof(unsigned));
  new_array  = malloc(h * w * sizeof(unsigned));
  unsigned (*univ)[w] = (void *)univ_array;

  int x,y;
  for (x=0;x<w;x++)
    for (y=0;y<h;y++) 
      univ[y][x] = rand() % 2;
  while (1) {
    show(univ);
    evolve(univ);
    usleep(200000);
  }
}

// -----------------------------------------------------------------------------------
// 3D甜甜圈
#define R(mul,shift,x,y) \
  _=x; \
  x -= mul*y>>shift; \
  y += mul*_>>shift; \
  _ = (3145728-x*x-y*y)>>11; \
  x = x*_>>10; \
  y = y*_>>10;

static char b[1760];
static signed char z[1760];

void donut(void) {
  int sA = 1024, cA = 0, sB = 1024, cB = 0, _;

  while(1) {
    screen_clear();

    memset(b, 32, 1760);  // text buffer
    memset(z, 127, 1760);   // z buffer
    int sj = 0, cj = 1024;
    for (int j = 0; j < 90; j++) {
      int si = 0, ci = 1024;  // sine and cosine of angle i
      for (int i = 0; i < 324; i++) {
        int R1 = 1, R2 = 2048, K2 = 5120*1024;

        int x0 = R1*cj + R2,
            x1 = ci*x0 >> 10,
            x2 = cA*sj >> 10,
            x3 = si*x0 >> 10,
            x4 = R1*x2 - (sA*x3 >> 10),
            x5 = sA*sj >> 10,
            x6 = K2 + R1*1024*x5 + cA*x3,
            x7 = cj*si >> 10,
            x = 25 + 30*(cB*x1 - sB*x4)/x6,
            y = 12 + 15*(cB*x4 + sB*x1)/x6,
            N = (((-cA*x7 - cB*((-sA*x7>>10) + x2) - ci*(cj*sB >> 10)) >> 10) - x5) >> 7;

        int o = x + 80 * y;
        signed char zz = (x6-K2)>>15;
        if (22 > y && y > 0 && x > 0 && 80 > x && zz < z[o]) {
          z[o] = zz;
          b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
        }
        R(5, 8, ci, si)  // rotate i
      }
      R(9, 7, cj, sj)  // rotate j
    }
    int y = 0, x = 0;
    for (int k = 0; 1761 > k; k++) {
      if (k % 80) {
        if (x < 50) print_char(b[k], y, x);
        x ++;
      } else {
        y ++;
        x = 1;
      }
    }
    R(5, 7, cA, sA);
    R(5, 8, cB, sB);
    
    screen_refresh();
    usleep(10000);
  }
}