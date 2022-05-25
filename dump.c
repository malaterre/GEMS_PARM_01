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

static void bswap_vect(uint32_t *vect, const size_t size) {
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

void print_header1(struct header1 *h1) {

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
  static const uint32_t sig1[] = {1430323200, 44, 131072, 44};
  assert(vect_equal(h1->unk1, sig1, sizeof sig1));

  // unk2:
  assert(0x10000 == h1->unk2[1]);
  assert(0x0800 == h1->unk2[3]); // 2048
  assert(0x0000 == h1->unk6[1]);

  // unk3:
  assert(0x8 == h1->unk3[2]);
  assert(0x0 == h1->unk3[3]);

  // unk4 / unk5:
  assert(vect_equal(h1->unk4, h1->unk5, sizeof h1->unk4));

  // unk6:
  assert(0x0 == h1->unk6[1]);
  assert(0x2 == h1->unk6[3]);
}

static void process_2420(FILE *in) {
  assert(is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);
  bswap_vect(h1.unk1, sizeof h1.unk1);
  bswap_vect(h1.unk2, sizeof h1.unk1);
  bswap_vect(h1.unk3, sizeof h1.unk1);
  bswap_vect(h1.unk4, sizeof h1.unk1);
  bswap_vect(h1.unk5, sizeof h1.unk1);
  bswap_vect(h1.unk6, sizeof h1.unk1);

  print_header1(&h1);
}

static void process_2428(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16
  fread(&h1, 1, sizeof h1, in);

  print_header1(&h1);
}

static void process_3600(FILE *in) {
  assert(!is_big_endian(in));
  struct header1 h1;
  assert(sizeof(h1) == 0x60); // 6x16

  fread(&h1, 1, sizeof h1, in);
  print_header1(&h1);
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
  default:
    ret = 1;
    assert(0);
  }

  fclose(in);
  return ret;
}
