#ifndef PRINT_H
#define PRINT_H

// Redefine the newlib libc _write() function so you can use printf in your code
extern "C" {
  int _write(int fd, const void *buf, size_t count) {
    // STDOUT
    if (fd == 1)
      SerialUSB.write((char*)buf, count);
    return 0;
  }
}

#endif

