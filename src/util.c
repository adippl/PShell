#include "util.h"

int getch() {
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}

void printColor(const char* string, color color, enum color_decorations color_decorations) {
  char command[13];

  sprintf(command, "%c[%d;%d;%dm", 0x1B, color_decorations, color.fg, color.bg);
  printf("%s", command);
  printf("%s", string);

  sprintf(command, "%c[%d;%d;%dm", 0x1B, 0, 37, 10);
  printf("%s", command);
}

string_array splitString(const char* string_to_split, char delimeter) {
  int start = 0;
  int j = 0;
  char** splitted_strings = (char**)calloc(strlen(string_to_split), sizeof(char*));
  string_array result;

  for (int i = 0;; i++) {
    if (string_to_split[i] == delimeter || string_to_split[i] == '\0') {
      splitted_strings[j] = (char*)calloc(i - start + 1, sizeof(char));
      memcpy(splitted_strings[j], &string_to_split[start], i - start);
      start = i + 1;
      j++;
    }
    if (string_to_split[i] == '\0')
      break;
  }
  result.len = j;
  result.values = splitted_strings;
  return result;
}

void free_string_array(string_array* arr) {
  if (arr->values == NULL)
    return;
  for (int i = 0; i < arr->len; i++) {
    free(arr->values[i]);
    arr->values[i] = NULL;
  }
  free(arr->values);
  arr->values = NULL;
}

void moveCursor(coordinates new_pos) {
  printf("\033[%d;%dH", new_pos.y, new_pos.x);
  fflush(stdin);
}

coordinates getTerminalSize() {
  coordinates size;
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  size.x = w.ws_col;
  size.y = w.ws_row;

  return size;
}

bool inArray(char* value, string_array array) {
  for (int i = 0; i < array.len; i++) {
    if (strcmp(value, array.values[i]) == 0) {
      return true;
    }
  }
  return false;
}

string_array removeDuplicates(string_array* matching_commands) {
  int j = 0;
  string_array no_dup_array;
  no_dup_array.values = calloc(matching_commands->len, sizeof(char*));
  no_dup_array.len = 0;

  for (int i = 0; i < matching_commands->len; i++) {
    if (!inArray(matching_commands->values[i], no_dup_array)) {
      no_dup_array.values[j] = calloc(strlen(matching_commands->values[i]) + 1, sizeof(char));
      strcpy(no_dup_array.values[j], matching_commands->values[i]);
      no_dup_array.len += 1;
      j++;
    }
  }
  free_string_array(matching_commands);

  return no_dup_array;
}

char* removeCharAtPos(char* line, int x_pos) {
  for (int i = x_pos - 1; i < strlen(line); i++) {
    line[i] = line[i + 1];
  }
  return line;
}

void backspaceLogic(char* line, int* i) {
  if (strlen(line) > 0 && i >= 0) {
    line = removeCharAtPos(line, *i);

    if (*i > 0) {
      *i -= 1;
    }
  }
}

void logger(enum logger_type type, void* message) {
  FILE* logfile = fopen("log.txt", "a");

  switch (type) {
  case integer: {
    fprintf(logfile, "%d", *((int*)message));
    break;
  }
  case string: {
    fprintf(logfile, "%s", (char*)message);
    break;
  }
  case character: {
    fprintf(logfile, "%c", *(char*)message);
    break;
  }
  default: {
    break;
  }
  }
  fclose(logfile);
}

coordinates getCursorPos() {
  char buf[1];
  char data[50];
  int y, x;
  char cmd[] = "\033[6n";
  coordinates cursor_pos = {.x = -1, .y = -1};
  struct termios oldattr, newattr;

  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  newattr.c_cflag &= ~(CREAD);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);

  write(STDIN_FILENO, cmd, sizeof(cmd));
  read(STDIN_FILENO, buf, 1);

  if (*buf == '\033') {
    read(STDIN_FILENO, buf, 1);
    if (*buf == '[') {
      read(STDIN_FILENO, buf, 1);
      for (int i = 0; *buf != 'R'; i++) {
        data[i] = *buf;
        read(STDIN_FILENO, buf, 1);
      }
      // check if string matches expected data
      int valid = sscanf(data, "%d;%d", &y, &x);
      if (valid == 2) {
        cursor_pos.x = x;
        cursor_pos.y = y;
      }
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return cursor_pos;
}

string_array getAllFilesInDir(string_array* directory_array) {
  struct dirent* file;
  string_array all_path_files;
  char** files = (char**)calloc(1024, sizeof(char*));
  int j = 0;
  int realloc_index = 1;

  for (int i = 0; i < directory_array->len; i++) {
    if (isDirectory(directory_array->values[i])) {
      DIR* dr = opendir(directory_array->values[i]);

      while ((file = readdir(dr)) != NULL) {
        if (j >= (1024 * realloc_index)) {
          realloc_index++;
          files = (char**)realloc(files, realloc_index * (1024 * (sizeof(char) * 24)));
          if (files == NULL) {
            exit(0);
          }
        }
        files[j] = (char*)calloc(strlen(file->d_name) + 1, sizeof(char));
        strcpy(files[j], file->d_name);
        j++;
      }
      closedir(dr);
    }
  }
  all_path_files.values = files;
  all_path_files.len = j;

  // free_string_array(directory_array);
  return all_path_files;
}

string_array filterMatching(char* line, const string_array PATH_BINS) {
  int buf_size = 24;
  int realloc_index = 1;
  char** matching_binaries = calloc(buf_size, sizeof(char*));
  string_array result;
  int j = 0;

  for (int i = 0; i < PATH_BINS.len; i++) {
    if (strncmp(PATH_BINS.values[i], line, strlen(line)) == 0) {
      if (j >= (realloc_index * buf_size)) {
        realloc_index++;
        matching_binaries = realloc(matching_binaries, realloc_index * buf_size * sizeof(char*));
      }
      matching_binaries[j] = calloc(strlen(PATH_BINS.values[i]) + 1, sizeof(char));
      strcpy(matching_binaries[j], PATH_BINS.values[i]);
      j++;
    }
  }
  result.values = matching_binaries;
  result.len = j;

  return result;
}

string_array getAllMatchingFiles(char* current_dir_sub, char* removed_sub) {
  char* temp_sub = calloc(strlen(current_dir_sub) + 1, sizeof(char));
  strcpy(temp_sub, current_dir_sub);

  string_array current_dir_array = {.len = 1, .values = &temp_sub};
  string_array all_files_in_dir = getAllFilesInDir(&current_dir_array);
  string_array filtered = filterMatching(removed_sub, all_files_in_dir);

  free_string_array(&all_files_in_dir);
  free(temp_sub);

  return filtered;
}

bool insertCharAtPos(char* line, int index, char c) {
  int len = strlen(line);
  if (index >= 0 && index <= strlen(line)) {
    for (int i = strlen(line) - 1; i >= index; i--) {
      line[i + 1] = line[i];
    }
    line[index] = c;
    line[len + 1] = '\0';
  } else {
    return false;
  }
  return true;
}

void insertStringAtPos(char* line, char* insert_string, int position) {
  if (strcmp(insert_string, "") == 0)
    return;
  insertCharAtPos(line, position, '%');
  insertCharAtPos(line, position + 1, 's');

  char* new_line = calloc(strlen(line) + strlen(insert_string) + 1, sizeof(char));
  sprintf(new_line, line, insert_string);
  strcpy(line, new_line);
  free(new_line);
}

int getWordEndIndex(char* line, int start) {
  int line_end = start;
  for (; line[start] != '\0' && line[start] != ' '; start++)
    line_end++;

  return line_end;
}

int isDirectory(const char* path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return 0;

  return S_ISDIR(statbuf.st_mode);
}

string_array copyStringArray(string_array arr) {
  string_array copy = {.len = arr.len, .values = calloc(arr.len, sizeof(char*))};
  for (int i = 0; i < arr.len; i++) {
    copy.values[i] = calloc(strlen(arr.values[i]) + 1, sizeof(char));
    strcpy(copy.values[i], arr.values[i]);
  }

  return copy;
}

file_string_tuple getFileStrings(char* current_word, char* current_path) {
  char* current_dir;
  char* removed_sub;

  switch (current_word[0]) {
  case '/': {
    current_dir = current_word;
    if (strlen(current_dir) == 1) {
      removed_sub = "";
    } else {
      removed_sub = &(current_dir[strlen(current_dir) - getAppendingIndex(current_dir, '/')]); // c_e
    }
    break;
  }
  case '~': {
    char* home_path = getenv("HOME"); // Users/username
    char* home_path_copy = calloc(strlen(home_path) + strlen(current_word) + 2, sizeof(char));
    strcpy(home_path_copy, home_path);

    char* current_path = strcat(home_path_copy, "/");       // Users/username/
    current_dir = strcat(current_path, &(current_word[1])); // Users/username/documents
    removed_sub = &(current_dir[strlen(current_dir) - getAppendingIndex(current_dir, '/')]); // documents
    break;
  }
  default: {
    current_dir = strcat(current_path, current_word); // documents/coding/c_e
    removed_sub = &(current_dir[strlen(current_dir) - getAppendingIndex(current_dir, '/')]); // c_e
    break;
  }
  }

  return (file_string_tuple){.removed_sub = removed_sub, .current_dir = current_dir};
}

int getAppendingIndex(char* line, char delimeter) {
  int j = 0;
  for (int i = strlen(line) - 1; i >= 0; i--) {
    if (line[i] == delimeter)
      return j;
    j++;
  }
  return -1;
}

void fileDirArray(string_array* filtered, char* current_dir_sub, char* removed_sub) {
  char* current_dir_sub_copy = calloc(strlen(current_dir_sub) + 256, sizeof(char));
  char* temp;
  char copy[512];
  strcat(current_dir_sub, "/");

  for (int i = 0; i < filtered->len; i++) {
    strcpy(current_dir_sub_copy, current_dir_sub);

    temp = strcpy(copy, strcat(current_dir_sub_copy, filtered->values[i]));

    if (isDirectory(temp)) {
      filtered->values[i] = realloc(filtered->values[i], strlen(filtered->values[i]) + 2);
      filtered->values[i][strlen(filtered->values[i]) + 1] = '\0';
      filtered->values[i][strlen(filtered->values[i])] = '/';
    }
    memset(copy, 0, strlen(copy));
    memset(temp, 0, strlen(temp));
    memset(current_dir_sub_copy, 0, strlen(current_dir_sub_copy));
  }
  free(current_dir_sub_copy);
  free(current_dir_sub);
}

int getCurrentWordPosInLine(string_array command_line, char* word) {
  for (int i = 0; i < command_line.len; i++) {
    if (strncmp(command_line.values[i], word, strlen(word)) == 0) {
      return i;
    }
  }

  return -1;
}
autocomplete_array fileComp(char* current_word) {
  char cd[256];
  file_string_tuple file_strings = getFileStrings(current_word, strcat(getcwd(cd, sizeof(cd)), "/"));

  char* current_dir_sub = calloc(strlen(file_strings.current_dir) + 2, sizeof(char));
  strncpy(current_dir_sub, file_strings.current_dir,
          strlen(file_strings.current_dir) - getAppendingIndex(file_strings.current_dir, '/'));
  string_array filtered = getAllMatchingFiles(current_dir_sub, file_strings.removed_sub);

  fileDirArray(&filtered, current_dir_sub, file_strings.removed_sub);

  return (autocomplete_array){.array.values = filtered.values,
                              .array.len = filtered.len,
                              .appending_index = strlen(file_strings.removed_sub)};
}

coordinates calculateCursorPos(coordinates terminal_size, coordinates cursor_pos, int prompt_len, int i) {
  int line_pos = i + prompt_len;

  if (line_pos < terminal_size.x) {
    return (coordinates){.x = line_pos, .y = cursor_pos.y};
  } else if (line_pos % terminal_size.x == 0) {
    return (coordinates){.x = terminal_size.x, .y = cursor_pos.y + ((line_pos - 1) / terminal_size.x)};
  } else {
    return (coordinates){.x = line_pos % terminal_size.x, .y = cursor_pos.y + (line_pos / terminal_size.x)};
  }
}

int calculateRowCount(coordinates terminal_size, int prompt_len, int i) {
  return calculateCursorPos(terminal_size, (coordinates){0, 0}, prompt_len, i).y;
}
