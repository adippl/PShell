#include "util.h"

char* getLastTwoDirs(char*);
void printPrompt(const char* dir, color color);
string_array removeDots(string_array* array);
void stringToLower(char* string);
char* joinHistoryFilePath(char* home_dir, char* destination_file);
void replaceAliases(string_array* splitted_line);
bool update(line_data* line_info, autocomplete_data* autocomplete_info, history_data* history_info,
            coordinates terminal_size, string_array PATH_BINS, coordinates* cursor_pos);
line_data* lineDataConstructor(int directory_len);
autocomplete_data* autocompleteDataConstructor();
history_data* historyDataConstructor(string_array* command_history, string_array global_command_history);
string_array concatenateArrays(const string_array one, const string_array two);
token_index_arr tokenizeLine(char* line);
