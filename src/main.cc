#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

enum class object_t { FIXNUM, BOOLEAN, CHARACTER };

struct object {
  object_t type;
  union data {
    struct boolean {
      char value;
    } boolean;

    struct character {
      char value;
    } character;

    struct fixnum {
      long value;
    } fixnum;
  } data;
};

// TODO: Figure out a way to use `unique_ptr` rather than `shared_ptr`.
//       That might be impossible due to the state holding those objects.
using objectptr_t = std::shared_ptr<object>;

objectptr_t make_object() { return std::make_shared<object>(); }

objectptr_t make_fixnum(long value) {
  auto obj = make_object();
  obj->type = object_t::FIXNUM;
  obj->data.fixnum.value = value;
  return obj;
}

objectptr_t make_character(char value) {
  auto obj = make_object();
  obj->type = object_t::CHARACTER;
  obj->data.character.value = value;
  return obj;
}

// Rather than using global variables for the true and false objects,
// lets keep them in a state.
class State {
public:
  State() {
    true_object = make_object();
    true_object->type = object_t::BOOLEAN;
    true_object->data.boolean.value = 't';

    false_object = make_object();
    false_object->type = object_t::BOOLEAN;
    false_object->data.boolean.value = 'f';
  }

  State(const State &) = delete;
  State &operator=(const State &) = delete;
  ~State() = default;

  objectptr_t get_true_object() { return true_object; }
  objectptr_t get_false_object() { return false_object; }

private:
  objectptr_t true_object;
  objectptr_t false_object;
};

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
  return std::isspace(ch) || ch == EOF || ch == '(' || ch == ')' || ch == '"' ||
         ch == ';';
}

objectptr_t read_character(std::istringstream &input) {
  auto eat_input_and_compare = [&](const char *expected, int size) -> bool {
    char buffer[size];
    input.get(buffer, size);
    return std::string(buffer) == expected;
  };

  char ch = input.get();
  switch (ch) {
  case 'n':
    // Lets look for newline
    if (input.peek() == 'e') {
      if (eat_input_and_compare("ewline", 7)) {
        if (!is_delimiter(input.peek()))
          error("Invalid character!\n", 4);
        return make_character('\n');
      }
    }
    break;
  case 's':
    // Lets look for space
    if (input.peek() == 'p') {
      if (eat_input_and_compare("pace", 5)) {
        if (!is_delimiter(input.peek()))
          error("Invalid character!\n", 4);
        return make_character(' ');
      }
    }
    break;
  default:
    break;
  }
  if (!is_delimiter(input.peek()))
    error("Invalid character!\n", 4);
  return make_character(ch);
}

objectptr_t read(std::istringstream &&input, State &s) {
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
  } else if (ch == '#') {
    ch = input.get();
    switch (ch) {
    case 't':
      return s.get_true_object();
    case 'f':
      return s.get_false_object();
    case '\\':
      return read_character(input);
    default:
      error("Unexpected character after '#'!", 4);
    }
  } else
    error("Bad input!\n", 3);

  error("Illegal state\n", 4);
  return nullptr;
}

objectptr_t eval(objectptr_t obj) { return obj; }

void write(objectptr_t obj) {
  switch (obj->type) {
  case object_t::BOOLEAN:
    printf("#%c", obj->data.boolean.value);
    break;
  case object_t::CHARACTER: {
    printf("#\\");
    char ch = obj->data.character.value;
    // These are the cases where the user just inputs '#\ ' or '#\'
    switch (ch) {
    case '\n':
      printf("newline");
      break;
    case ' ':
      printf("space");
      break;
    default:
      printf("%c", ch);
      break;
    }
  } break;
  case object_t::FIXNUM:
    printf("%ld", obj->data.fixnum.value);
    break;
  default:
    error("Unknown type!\n", 1);
  }
}

int main() {
  std::string input;
  State s;

  printf("> ");
  while (std::getline(std::cin, input)) {
    write(eval(read(std::istringstream(input + '\n'), s)));
    printf("\n");
    printf("> ");
  }
}
