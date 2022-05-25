#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

size_t file_length(FILE *in) {
  size_t sz;
  fseek(in, 0L, SEEK_END);
  sz = ftell(in);
  fseek(in, 0L, SEEK_SET);
  return sz;
}

int fpeek(FILE *stream) {
  int c;

  c = fgetc(stream);
  ungetc(c, stream);

  return c;
}

void process_3600(FILE *in) {}

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;
  const char *filename = argv[1];
  FILE *in = fopen(filename, "rb");

  const size_t sz = file_length(in);
  const int first_byte = fpeek(in);
  assert(first_byte == 0x0 || first_byte == 'U');
  const bool doswap = first_byte == 'U';
  int ret = 0;
  switch (sz) {
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
