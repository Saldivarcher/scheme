#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

enum class object_t { FIXNUM, BOOLEAN, CHARACTER, STRING };

struct object {
  object_t type;
  std::variant<char, long, std::string> data;
};

// TODO: Figure out a way to use `unique_ptr` rather than `shared_ptr`.
//       That might be impossible due to the state holding those objects.
using objectptr_t = std::shared_ptr<object>;

objectptr_t make_object() { return std::make_shared<object>(); }

objectptr_t make_object(object_t type, auto &&data) {
  auto obj = make_object();
  obj->type = type;
  obj->data = std::move(data);
  return obj;
}

// Rather than using global variables for the true and false objects,
// lets keep them in a state.
class State {
public:
  State() {
    true_object = make_object(object_t::BOOLEAN, 't');
    false_object = make_object(object_t::BOOLEAN, 'f');
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
  std::fprintf(stderr, "%s", msg);
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
        return make_object(object_t::CHARACTER, '\n');
      }
    }
    break;
  case 's':
    // Lets look for space
    if (input.peek() == 'p') {
      if (eat_input_and_compare("pace", 5)) {
        if (!is_delimiter(input.peek()))
          error("Invalid character!\n", 4);
        return make_object(object_t::CHARACTER, ' ');
      }
    }
    break;
  default:
    break;
  }
  if (!is_delimiter(input.peek()))
    error("Invalid character!\n", 4);
  return make_object(object_t::CHARACTER, ch);
}

objectptr_t read(std::istringstream &&input, State &s) {

  auto get_or_getchar = [&](const char &ch) -> char {
    // So when a user enters a string and starts a newline before
    // entering a '"'. We need to get the next incoming line and insert
    // it into the istringstream object.
    if (ch == '\n') {
      std::string future_input;
      std::getline(std::cin, future_input);
      input.clear();
      input.str(future_input + "\n");
      return input.get();
    }
    return input.get();
  };

  eat_whitespace(input);

  char ch = input.get();

  if (std::isdigit(ch) || (ch == '-' && std::isdigit(input.peek()))) {
    short sign = 1;
    long num = 0;
    if (ch == '-')
      sign = -1;
    else
      input.unget();

    while (std::isdigit(ch = input.get()))
      num = (num * 10) + (ch - '0');

    num *= sign;

    if (is_delimiter(ch)) {
      input.unget();
      return make_object(object_t::FIXNUM, num);
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
  } else if (ch == '"') {
    std::string s;
    while ((ch = get_or_getchar(ch)) != '"') {
      if (ch == '\\') {
        if (input.peek() == '"') {
          // Eat '\' char
          s += ch;

          ch = input.get();

          // Eat '"' char
          s += ch;
          continue;
        }
      }
      if (ch == EOF)
        error("There shouldn't be a null-terminated character here!", 5);
      s += ch;
    }

    return make_object(object_t::STRING, s);
  } else
    error("Bad input!\n", 3);

  error("Illegal state\n", 4);
  return nullptr;
}

objectptr_t eval(objectptr_t obj) { return obj; }

void write(objectptr_t obj) {
  switch (obj->type) {
  case object_t::BOOLEAN:
    std::printf("#%c", std::get<char>(obj->data));
    break;
  case object_t::CHARACTER: {
    std::printf("#\\");
    char ch = std::get<char>(obj->data);
    // These are the cases where the user just inputs '#\ ' or '#\'
    switch (ch) {
    case '\n':
      std::printf("newline");
      break;
    case ' ':
      std::printf("space");
      break;
    default:
      std::printf("%c", ch);
      break;
    }
  } break;
  case object_t::FIXNUM:
    std::printf("%ld", std::get<long>(obj->data));
    break;
  case object_t::STRING: {
    std::string data = std::get<std::string>(obj->data);
    std::printf("\"");
    for (const auto &ch : data) {
      switch (ch) {
      case '\n':
        std::printf("\\n");
        break;
      default:
        std::printf("%c", ch);
        break;
      }
    }
    std::printf("\"");
    break;
  }
  default:
    error("Unknown type!\n", 1);
  }
}

int main() {
  std::string input;
  State s;

  std::printf("> ");
  while (std::getline(std::cin, input)) {
    write(eval(read(std::istringstream(input + '\n'), s)));
    std::printf("\n");
    std::printf("> ");
  }
}
