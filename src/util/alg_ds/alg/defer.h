/*
MIT License

Copyright (c) 2021 Mikel Irazabal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef DEFER_H
#define DEFER_H

/*
 * Defer mechanism taken from http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2542.pdf
 * It may become part of the C2X standard in <stddefer.h>
 */

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )

#if defined __GNUC__ // nested function called directly by the cleanup attribute

#define DEFER_IMPL(name, ...) \
  void name(int* defer_unused_arg_) { (void)defer_unused_arg_; __VA_ARGS__ } \
  __attribute__((__cleanup__(name))) int MACRO_CONCAT(name, _var) = 0

#define defer(...) DEFER_IMPL(MACRO_CONCAT(DF_, __COUNTER__), __VA_ARGS__)

#endif

#endif

