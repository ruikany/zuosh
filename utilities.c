#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void quit() {
  printf("see you next time...\n");
  exit(0);
}

void file_doesnt_exist(char *filename) {
  fprintf(stderr, "hey, file %s doesn't exist\n", filename);
  fflush(stderr); // WHAT
  // so fprintf + fflush works, and printf works, but not fprintf
}

void zcat(char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file)
    file_doesnt_exist(filename);

  char *buffer = NULL;
  size_t size = 0;
  ssize_t chars_read;
  while ((chars_read = getline(&buffer, &size, file)) != -1) {
    printf("%s", buffer);
  }
  free(buffer);
  fclose(file);
}

int zgrep(char *search_term, char *files[], int num_files) {
  char *buffer = NULL;
  size_t size = 0;
  ssize_t chars_read;
  int found = 0;
  for (int i = 0; i < num_files; i++) {
    FILE *file = fopen(files[i], "r");
    if (!file) {
      file_doesnt_exist(files[i]);
      return 1;
    }

    while ((chars_read = getline(&buffer, &size, file) != -1)) {
      if (strstr(buffer, search_term) != NULL) {
        printf("Found %s in line: %s", search_term, buffer);
        found = 1;
      }
    }
    fclose(file);
  }
  if (!found)
    printf("Nothing found...\n");
  return 0;
}

int zzip(char *files[], int num_files) {
  char buffer[1024] = {-1}; // can fread up to 1024 bytes in one go
  for (int i = 0; i < num_files; i++) {
    FILE *input_file = fopen(files[i], "r");
    if (!input_file) {
      file_doesnt_exist(files[i]);
      return 1;
    }

    char output_name[256];
    snprintf(output_name, sizeof(output_name), "%s.zz", files[i]);
    FILE *compressed_file = fopen(output_name, "wb");
    if (!compressed_file) {
      fclose(input_file);
      return 1;
    }

    // to check if compression actually helped reduce file size
    int total_bytes_input = 0;
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
      total_bytes_input += bytes_read;
      size_t count;
      size_t pos = 0;
      unsigned char current = buffer[pos];

      while (pos < bytes_read) {
        current = buffer[pos];
        count = 1;

        while (pos + count < bytes_read && buffer[pos + count] == current &&
               count < 255) {
          count++;
        }
        fwrite(&count, 1, 1, compressed_file);
        fwrite(&current, 1, 1, compressed_file);

        pos += count;
      }
    }
    // can use ftell to get size
    fflush(compressed_file);
    long total_bytes_output = ftell(compressed_file);

    printf("Successfully compressed file: %s to %s\n", files[i], output_name);
    if (total_bytes_input < total_bytes_output) {
      printf("Warning: the compressed file size is actually larger...\n");
    }

    fclose(input_file);
    fclose(compressed_file);
  }
  return 0;
}

int zunzip(char *files[], int num_files) {
  for (int i = 0; i < num_files; i++) {
    FILE *compressed_file = fopen(files[i], "rb");
    if (!compressed_file) {
      file_doesnt_exist(files[i]);
      return 1;
    }

    unsigned char count, value;
    while (fread(&count, 1, 1, compressed_file) &&
           fread(&value, 1, 1, compressed_file)) {
      for (int i = 0; i < count; i++) {
        printf("%c", value);
      }
    }

    fclose(compressed_file);
  }
  return 0;
}

int compare_lines(const void *a, const void *b) {
  char *const *str1 = a;
  char *const *str2 = b;
  return strcmp(*str1, *str2);
}

int zsort(char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    file_doesnt_exist(filename);
    return 1;
  }

  char buffer[1000]; // currently 1k chars per line
  size_t capacity = 10;
  char **lines = malloc(capacity * sizeof(char *));

  int i = 0;
  while (fgets(buffer, sizeof(buffer), file)) {
    if (i >= capacity) {
      capacity *= 2;
      char **temp = realloc(lines, capacity * sizeof(char *));
      lines = temp;
    }

    lines[i] = strdup(buffer);
    i++;
  }

  qsort(lines, i, sizeof(char *), compare_lines);

  for (int j = 0; j < i; j++) {
    printf("%s", lines[j]);
    free(lines[j]);
  }
  fclose(file);
  return 0;
}

int zrev(char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    file_doesnt_exist(filename);
    return 1;
  }
  char *buffer = NULL;
  size_t size = 0;
  ssize_t chars_read;
  size_t capacity = 10;
  char **lines = malloc(capacity * sizeof(char *));
  int i = 0;
  while ((chars_read = getline(&buffer, &size, file)) != -1) {
    if (i >= capacity) {
      capacity *= 2;
      char **temp = realloc(lines, capacity * sizeof(char *));
      lines = temp;
    }

    lines[i] = strdup(buffer);
    i++;
  }
  printf("Stored %d lines:\n", i);
  for (int k = 0; k < i; k++) {
    printf("[%d]: %s", k, lines[k]);
  }

  for (int j = i - 1; j >= 0; j--) {
    printf("%s", lines[j]);
    free(lines[j]);
  }

  free(buffer);
  free(lines);
  fclose(file);
  return 0;
}
