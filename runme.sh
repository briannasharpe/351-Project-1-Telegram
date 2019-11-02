#!/bin/bash

#To run the file,navigate to the folder with
#this bash file and type "./runme.sh" without quotes

echo "Batch file executing..."
chmod +x runme.sh
gnome-terminal -e "./receiver"
gnome-terminal -e "./sender keyfile.txt"

