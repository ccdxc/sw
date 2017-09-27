#include "dol/test/storage/log.hpp"

namespace log {

void dump(uint8_t *buf) {
  int i;

  for (i = 0; i < 64; i++) {
    printf("%2.2x ", buf[i]);
    if ((i & 7) == 7) {
      printf(" ");
    }
    if ((i & 0xf) == 0xf) {
      printf("\n");
    }
  }
}

}  // namespace log

