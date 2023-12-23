#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_serial_input(char *serial_input, char *message) {
  int volume;

  if (strncmp(serial_input, "MVMAX", 5) == 0) {
    return 1;
  } else if (strncmp(serial_input, "MUON", 4) == 0) {
    snprintf(message, 13, "Airplay   --");
    return 1;
  } else if (strncmp(serial_input, "MUOFF", 5) == 0) {
    snprintf(message, 13, "Airplay   %02d", volume);
    return 1;
  } else if (strncmp(serial_input, "PWSTANDBY", 9) == 0) {
    snprintf(message, 13, "            ");
    return 1;
  } else if (strncmp(serial_input, "MV", 2) == 0) {
    char volume_code[4];
    strncpy(volume_code, serial_input + 2, 3);
    volume_code[3] = '\0';

    if (strlen(volume_code) == 2) {
      sscanf(volume_code, "%d", &volume);
      snprintf(message, 13, "Airplay   %02d", volume);
      return 1;
    }
  }

  return 0;
}

int main() {
  char *dp1_serial_port = "/dev/ser-dp1";
  char *dp2_serial_port = "/dev/ser-dp2";
  char *avr_serial_port = "/dev/ser-avr";
  int baud_rate = 9600;

  FILE *dp1_ser = fopen(dp1_serial_port, "w");
  FILE *dp2_ser = fopen(dp2_serial_port, "w");
  FILE *avr_ser = fopen(avr_serial_port, "r+");

  if (dp1_ser == NULL || dp2_ser == NULL || avr_ser == NULL) {
    perror("Error opening serial ports");
    exit(EXIT_FAILURE);
  }

  char serial_input[256];
  char partial_input[256] = "";
  size_t partial_input_len = 0;
  int volume;
  int ch; // Change 'char' to 'int' to handle EOF
  while ((ch = fgetc(avr_ser)) != EOF) {
    if (ch == '\n') {
      // Process the complete message when a newline character is encountered
      serial_input[partial_input_len] = '\0';
      strcpy(serial_input, partial_input);
      char message[13];

      if (parse_serial_input(serial_input, message)) {
        printf("Received from avr_ser: %s\n", serial_input);
        printf("Printing: %s\n", message);

        fprintf(dp1_ser, "%s\n", message);
        fprintf(dp2_ser, "%s\n", message);
      } else {
        printf("Invalid input: %s\n", serial_input);
      }

      // Reset the partial input buffer
      partial_input_len = 0;
      memset(partial_input, 0, sizeof(partial_input));
    } else {
      // Append the character to the partial input buffer
      partial_input[partial_input_len++] = ch;
    }
  }

  fclose(dp1_ser);
  fclose(dp2_ser);
  fclose(avr_ser);

  return 0;
}
