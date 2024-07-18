This project demonstrates techniques used in **binary manipulation**, specifically targeting ELF (Executable
and Linkable Format) executables. This tool encrypts the .text section of an **ELF executable**, injects a
custom payload written in Assembly into a code cave, and modifies the entry point to execute the injected
payload first. Upon execution, the payload will display a message, decrypt the `.text` section, and
then execute the original program.

## Introduction
The Malicious-Packer is a program written in C that showcases how to encrypt, inject, and manipulate ELF executables. The injected payload performs two main actions:

- Prints a custom message.
- Decrypts the `.text` section and transfers control to the original entry point of the executable.

## Usage
1. Compile the project
   ```bash
   make
   ```
2. Run the packer on a target program (ELF executable binary)
   ```bash
   ./woody-woodpacker target_program
   ```
3. Run the infected program
