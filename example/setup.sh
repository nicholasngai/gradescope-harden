#!/usr/bin/env bash

. /autograder/source/gradescope-harden/source/setup.sh

apt-get install -y python3 python3-pip python3-dev

pip3 install -r /autograder/source/requirements.txt
