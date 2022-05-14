#include "../../src/util.h"
#include <criterion/criterion.h>

Test(removeMultipleWhitespace, removes_any_extraneous_whitespace) {
  char* line = calloc(64, sizeof(char));
  strcpy(line, "    uwe   ist cool");
  char* result = removeMultipleWhitespaces(line);
  cr_expect(strcmp(result, "uwe ist cool") == 0);
  free(result);
}

Test(removeMultipleWhitespace, only_whitespace) {
  char* empty = calloc(5, sizeof(char));
  strcpy(empty, "    ");
  char* result = removeMultipleWhitespaces(empty);

  cr_expect(strcmp(result, "") == 0);
  free(result);
}

Test(removeMultipleWhitespace, when_everything_fine_nothing_changs) {
  char* line = calloc(64, sizeof(char));
  strcpy(line, "ls .");
  char* result = removeMultipleWhitespaces(line);
  cr_expect(strcmp(result, "ls .") == 0);
  free(result);
}

Test(insertStringAtPos, insert_string_at_end) {
  char* line = calloc(24, sizeof(char));
  strcpy(line, "testing the waters");
  char* insert_string = " here";

  insertStringAtPos(&line, insert_string, strlen(line));
  cr_expect(strcmp(line, "testing the waters here") == 0);

  free(line);
}

Test(insertStringAtPos, insert_string_in_middle) {
  char* line = calloc(24, sizeof(char));
  strcpy(line, "testing the waters");
  char* insert_string = "cold ";

  insertStringAtPos(&line, insert_string, 12);

  cr_expect(strcmp(line, "testing the cold waters") == 0);

  free(line);
}

Test(insertStringAtPos, insert_string_at_start) {
  char* line = calloc(24, sizeof(char));
  strcpy(line, "~/testing");
  char* insert_string = "/Users";

  removeCharAtPos(line, 1);
  insertStringAtPos(&line, insert_string, 0);

  cr_expect(strcmp(line, "/Users/testing") == 0);

  free(line);
}

Test(getAppendingIndex, returns_3_if_second_word_is_len_3) {
  char line[64] = "make mak";
  int result = getAppendingIndex(line, ' ');

  cr_expect(result == 3);
}

Test(getAppendingIndex, still_works_with_only_space) {
  char line[64] = "make ";
  int result = getAppendingIndex(line, ' ');

  cr_expect(result == 0);
}

Test(calculateCursorPos, jumps_down_if_current_line_longer_than_term) {
  coordinates cursor_pos = {.x = 20, .y = 2};
  coordinates term_size = {.x = 20, .y = 100};
  int prompt_len = 5;
  int i = 20;

  coordinates result = calculateCursorPos(term_size, cursor_pos, prompt_len, i);
  cr_expect(result.x == 5);
  cr_expect(result.y == 3);
}

Test(calculateCursorPos, shouldnt_change_coordinates_when_line_short) {
  coordinates cursor_pos = {.x = 15, .y = 2};
  coordinates term_size = {.x = 20, .y = 100};
  int prompt_len = 5;
  int i = 10;

  coordinates result = calculateCursorPos(term_size, cursor_pos, prompt_len, i);
  cr_expect(result.x == 15);
  cr_expect(result.y == 2);
}

Test(calculateCursorPos, when_on_last_row_should_still_increment) {
  coordinates cursor_pos = {.x = 15, .y = 100};
  coordinates term_size = {.x = 20, .y = 100};
  int prompt_len = 5;
  int i = 20;

  coordinates result = calculateCursorPos(term_size, cursor_pos, prompt_len, i);
  cr_expect(result.x == 5);
  cr_expect(result.y == 101);
}

Test(calculateCursorPos, when_line_expands_over_many_rows_and_cursor_shouldnt_jump_down_too_early) {
  coordinates cursor_pos = {.x = 0, .y = 1};
  coordinates term_size = {.x = 45, .y = 100};
  int prompt_len = 0;
  int i = 90;

  coordinates result = calculateCursorPos(term_size, cursor_pos, prompt_len, i);

  cr_expect(result.x == 45);
  cr_expect(result.y == 2);
}

Test(isOnlyDelimeter, true_if_just_white_space) {
  char* string = "                  ";
  bool result = isOnlyDelimeter(string, ' ');

  cr_expect(result == true);
}

Test(isOnlyDelimeter, false_if_even_single_char) {
  char* string = "                  l";
  bool result = isOnlyDelimeter(string, ' ');

  cr_expect(result == false);
}

Test(firstNonDelimeterIndex, returns_index_of_splitted_array_where_not_delimeter) {
  char* one = "";
  char* two = "";
  char* three = "com";
  char* four = "uwe";
  char* addr_one[] = {one, two, three, four};

  string_array arr1 = {.len = 4, .values = addr_one};
  int result = firstNonDelimeterIndex(arr1);

  cr_expect(result == 2);
}

Test(firstNonDelimeterIndex, if_first_elem_not_delim_returns_zero) {
  char* three = "com";
  char* four = "uwe";
  char* addr_one[] = {three, four};

  string_array arr1 = {.len = 2, .values = addr_one};
  int result = firstNonDelimeterIndex(arr1);

  cr_expect(result == 0);
}

Test(insertCharAtPos, see_if_string_reference_changes) {
  char line[24] = "uwe tested";

  insertCharAtPos(line, 3, 'i');
  cr_expect(strcmp(line, "uwei tested") == 0);
}

Test(removeSlice, when_slice_is_complete_line_empties_string) {
  char* line = calloc(4, sizeof(char));
  strcpy(line, "src");
  removeSlice(&line, 0, 3);
  cr_expect(strcmp(line, "") == 0);
  free(line);
}

Test(removeSlice, remove_nothing_cursor_end_of_current_word) {
  char* word = calloc(52, sizeof(char));
  strcpy(word, "testing if Makefile works");
  int start = 19;

  removeSlice(&word, start, start);

  logger(string, word);
  cr_expect(strcmp(word, "testing if Makefile works") == 0);
  free(word);
}
