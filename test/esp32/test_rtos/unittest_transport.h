#ifndef UNITTEST_TRANSPORT_H
#define UNITTEST_TRANSPORT_H

#include <stdio.h>

inline void unittest_uart_begin() {}
inline void unittest_uart_putchar(char c) { putchar(c); }
inline void unittest_uart_flush() { fflush(stdout); }
inline void unittest_uart_end() {}

#endif
