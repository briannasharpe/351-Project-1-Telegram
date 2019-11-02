#!/bin/bash
gnome-terminal -e 'make'
gnome-terminal -e 'sh -c \"./reciever\"'
echo -e "Running reciever"
gnome-terminal -e 'sh -c \"./sender keyfile\"'
echo -e "Running sender"
