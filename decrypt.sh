#! /bin/bash

echo "challenge code..."
openssl enc -aes-256-cbc -d -in ioctopus.c.enc -out ioctopus.c
echo "solution..."
openssl enc -aes-256-cbc -d -in solution.c.enc -out solution.c
echo "makefile..."
openssl enc -aes-256-cbc -d -in Makefile.enc -out Makefile
