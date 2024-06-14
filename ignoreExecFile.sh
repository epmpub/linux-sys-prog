#!/bin/bash
# Script to add executable files to .gitignore

# List all executable files and append them to .gitignore
find . -type f -executable | grep -v ".gitignore" >> .gitignore

# Sort and remove duplicate entries
sort -u -o .gitignore .gitignore
