#! /bin/bash

echo "password:"
read PASS

echo "challenge code..."
openssl enc -aes-256-cbc -d -in ioctopus.c.enc -out ioctopus.c -k $PASS
echo "solution..."
openssl enc -aes-256-cbc -d -in solution.c.enc -out solution.c -k $PASS
