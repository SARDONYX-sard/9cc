#!/bin/bash
# @(#) This script is sh formatter.
shfmt -i 2 -w ./**/*.sh ./Dockerfile ./.gitignore ./.dockerignore ./.properties ./etc/hosts

echo formmatted.
