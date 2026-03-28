// Reads a line of input into buffer, handles UTF-8 and cursor movement.
// Returns: length (excluding null terminator), 0 for empty, -1 on error
int mini_readline(const char *prompt, char *line_buffer, int bufsize);
