#include <iostream>
#include <memory>
#include <sstream>
#include <string>

enum class object_t { FIXNUM };

struct object {
  object_t type;
  union data {
    struct fixnum {
      long value;
    } fixnum;
  } data;
};

using objectptr_t = std::unique_ptr<object>;

objectptr_t make_object() { return std::make_unique<object>(); }

objectptr_t make_fixnum(long value) {
  auto obj = make_object();
  obj->type = object_t::FIXNUM;
  obj->data.fixnum.value = value;
  return obj;
}

void error(const char *msg, int8_t status) {
  fprintf(stderr, "%s", msg);
  exit(status);
}

void eat_whitespace(std::istringstream &input) {
  char ch;
  while ((ch = input.get()) != EOF) {
    if (std::isspace(ch))
      continue;
    else if (ch == ';') {
      while (((ch = input.get()) != EOF) && (ch != '\n'))
        ;
      continue;
    }
    input.unget();
    break;
  }
}

bool is_delimiter(char ch) {
  return isspace(ch) || ch == EOF || ch == '(' || ch == ')' || ch == '"' ||
         ch == ';';
}

objectptr_t read(std::istringstream &&input) {
  eat_whitespace(input);

  char ch = input.get();

  if (isdigit(ch) || (ch == '-' && isdigit(input.peek()))) {
    short sign = 1;
    long num = 0;
    if (ch == '-')
      sign = -1;
    else
      input.unget();

    while (isdigit(ch = input.get()))
      num = (num * 10) + (ch - '0');

    num *= sign;

    if (is_delimiter(ch)) {
      input.unget();
      return make_fixnum(num);
    } else
      error("Number not followed by delimiter\n", 2);
  } else
    error("Bad input!\n", 3);

  error("Illegal state\n", 4);
  return nullptr;
}

objectptr_t eval(objectptr_t obj) { return obj; }

void write(objectptr_t obj) {
  switch (obj->type) {
  case object_t::FIXNUM:
    printf("%ld\n", obj->data.fixnum.value);
    break;
  default:
    error("Unknown type!\n", 1);
  }
}

int main() {
  std::string input;

  printf("> ");
  while (std::cin >> input) {
    write(eval(read(std::istringstream(input))));
    printf("> ");
  }
}
