#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h> // memcmp

#include <byteswap.h>
// uint16_t bswap_16(uint16_t x);
// uint32_t bswap_32(uint32_t x);
// uint64_t bswap_64(uint64_t x);

static size_t file_length(FILE *in) {
  size_t sz;
  fseek(in, 0L, SEEK_END);
  sz = ftell(in);
  fseek(in, 0L, SEEK_SET);
  return sz;
}

static int fpeek(FILE *stream) {
  int c;

  c = fgetc(stream);
  ungetc(c, stream);

  return c;
}

static bool is_big_endian(FILE *in) {
  const int first_byte = fpeek(in);
  assert(first_byte == 0x0 || first_byte == 'U');
  const bool doswap = first_byte == 'U';
  return doswap;
}

static void bswap_vect(void *vvect, const size_t size) {
  uint32_t *vect = vvect;
  const size_t nelem = size / sizeof(*vect);
  for (size_t n = 0; n < nelem; ++n) {
    vect[n] = bswap_32(vect[n]);
  }
}

static bool vect_equal(const uint32_t *vec1, const uint32_t *vec2,
                       const size_t size) {
  return memcmp(vec1, vec2, size) == 0;
}

struct header1 {
  uint32_t unk1[4];
  uint32_t unk2[4];
  uint32_t unk3[4];
  uint32_t unk4[4];
  uint32_t unk5[4];
  uint32_t unk6[4];
};

void print_vect(const uint32_t *vect, const size_t size) {
  const size_t nelem = size / sizeof(*vect);
  for (size_t n = 0; n < nelem; ++n) {
    if (n)
      printf(",");
    printf("%d", vect[n]);
  }
  printf("\n");
}

struct header1_magic {
  const uint32_t *sig1;
  uint32_t unk2_3;
  uint32_t unk3_2;
  uint32_t unk6_1;
};

void print_header1(struct header1 *h1, const struct header1_magic *magic) {

  print_vect(h1->unk1, sizeof h1->unk1);
  print_vect(h1->unk2, sizeof h1->unk1);
  print_vect(h1->unk3, sizeof h1->unk1);
  print_vect(h1->unk4, sizeof h1->unk1);
  print_vect(h1->unk5, sizeof h1->unk1);
  print_vect(h1->unk6, sizeof h1->unk1);

  // 1: 1430323200,44,131072,44
  // 2: 320,65536,364,2048
  // 3: 196608,2412,8,0
  // 4: 11,27,98,638685450
  // 5: 11,27,98,638685450
  // 6: 971489109,0,0,2

  // 1: 1430323200,44,131072,44
  // 2: 1500,65536,1544,2048
  // 3: 196608,3592,8,0
  // 4: 13,27,102,638697750
  // 5: 13,27,102,638697750
  // 6: 999913710,0,1325690852,2

  // unk1:
  assert(vect_equal(h1->unk1, magic->sig1, sizeof h1->unk1));

  // unk2:
  assert(0x10000 == h1->unk2[1]);
  assert(magic->unk2_3 == h1->unk2[3]); // 2048 - 4096
  // assert(magic->unk6_1 == h1->unk6[1]); // 0x0

  // unk3:
  assert(magic->unk3_2 == h1->unk3[2]); // 8 - 12
  // assert(0x0 == h1->unk3[3]);

  // unk4 / unk5:
  assert(vect_equal(h1->unk4, h1->unk5, sizeof h1->unk4));

  // unk6:
  assert(0x0 == h1->unk6[1]);
  assert(0x2 == h1->unk6[3]);
}

static const uint32_t sig1[] = {1430323200, 44, 131072, 44};
static const uint32_t sig2[] = {1430323200, 56, 131072, 56};

static const char zero268[268];

static void process_2420(FILE *in) {
  size_t nread;
  assert(is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  nread = fread(&h1, 1, sizeof h1, in);
  assert(nread == sizeof h1);
  bswap_vect(h1.unk1, sizeof h1.unk1);
  bswap_vect(h1.unk2, sizeof h1.unk1);
  bswap_vect(h1.unk3, sizeof h1.unk1);
  bswap_vect(h1.unk4, sizeof h1.unk1);
  bswap_vect(h1.unk5, sizeof h1.unk1);
  bswap_vect(h1.unk6, sizeof h1.unk1);

  struct header1_magic magic;
  magic.sig1 = sig1;
  magic.unk2_3 = 2048;
  magic.unk3_2 = 8;
  print_header1(&h1, &magic);

  char buf[268];
  nread = fread(buf, 1, sizeof buf, in);
  assert(nread == sizeof buf);
  long pos = ftell(in);
  assert(sizeof buf == sizeof zero268);
  assert(memcmp(buf, zero268, sizeof buf) == 0);

  pos = ftell(in);
  //  printf("D: %ld\n", pos);
  char buf1[10];
  nread = fread(buf1, 1, sizeof buf1, in);
  assert(nread == sizeof buf1);

  char buf2[34];
  nread = fread(buf2, 1, sizeof buf2, in);
  assert(nread == sizeof buf2);

  uint32_t buf3[44];
  nread = fread(buf3, 1, sizeof buf3, in);
  assert(nread == sizeof buf3);

  char buf4[184];
  nread = fread(buf4, 1, sizeof buf4, in);
  assert(nread == sizeof buf4);
  assert(sizeof buf4 <= sizeof zero268);
  assert(memcmp(buf4, zero268, sizeof buf4) == 0);

  uint32_t buf5[14];
  nread = fread(buf5, 1, sizeof buf5, in);
  assert(nread == sizeof buf5);

  char buf6[268];
  nread = fread(buf6, 1, sizeof buf6, in);
  assert(nread == sizeof buf6);
  assert(sizeof buf6 == sizeof zero268);
  assert(memcmp(buf6, zero268, sizeof buf6) == 0);

  pos = ftell(in);
  //  printf("D: %ld\n", pos);
  float buf7[57];
  nread = fread(buf7, 1, sizeof buf7, in);
  assert(nread == sizeof buf7);
  bswap_vect(buf7, sizeof buf7);

  char buf8[160];
  nread = fread(buf8, 1, sizeof buf8, in);
  assert(nread == sizeof buf8);
  assert(sizeof buf8 <= sizeof zero268);
  assert(memcmp(buf8, zero268, sizeof buf8) == 0);

  uint32_t buf9[23];
  nread = fread(buf9, 1, sizeof buf9, in);
  assert(nread == sizeof buf9);

  char buf10[844];
  nread = fread(buf10, 1, sizeof buf10, in);
  assert(nread == sizeof buf10);
  for (int i = 0; i < 4; ++i) {
    assert(memcmp(buf10 + i * 211, zero268, 211) == 0);
  }

  uint32_t buf11[1];
  nread = fread(buf11, 1, sizeof buf11, in);
  assert(nread == sizeof buf11);
  assert(buf11[0] == 512);

  pos = ftell(in);
  //  printf("cur: %ld\n", pos);
  assert(pos == 2420);
}

static void process_2428(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);

  struct header1_magic magic;
  magic.sig1 = sig1;
  magic.unk2_3 = 2048;
  magic.unk3_2 = 8;
  print_header1(&h1, &magic);
}

static void process_3600(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);

  struct header1_magic magic;
  magic.sig1 = sig1;
  magic.unk2_3 = 2048;
  magic.unk3_2 = 8;
  print_header1(&h1, &magic);
}

static void process_5648(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);

  struct header1_magic magic;
  magic.sig1 = sig1;
  magic.unk2_3 = 2 * 2048;
  magic.unk3_2 = 8;
  print_header1(&h1, &magic);
}

static void process_7336(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);

  struct header1_magic magic;
  magic.sig1 = sig1;
  magic.unk2_3 = 2048;
  magic.unk3_2 = 8;
  print_header1(&h1, &magic);
}

static void process_7480(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);

  struct header1_magic magic;
  magic.sig1 = sig2;
  magic.unk2_3 = 2 * 2048;
  magic.unk3_2 = 12;
  print_header1(&h1, &magic);
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;
  const char *filename = argv[1];
  FILE *in = fopen(filename, "rb");

  const size_t sz = file_length(in);
  int ret = 0;
  switch (sz) {
  case 2420:
    process_2420(in);
    break;
  case 2428:
    process_2428(in);
    break;
  case 3600:
    process_3600(in);
    break;
  case 5648:
    process_5648(in);
    break;
  case 7336:
    process_7336(in);
    break;
  case 7480:
    process_7480(in);
    break;
  default:
    ret = 1;
    assert(0);
  }

  fclose(in);
  return ret;
}
