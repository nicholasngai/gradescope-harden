#!/bin/sh

set -eux

GRADESCOPE_HARDEN_SRC=/autograder/source/gradescope-harden/source

# Ensure the existence of the config file.
if [ ! -f /autograder/source/gradescope-harden.yml ]; then
    echo 'gradescope-harden.yml config file not found! Please make sure the file is included in your autograder.' >&2
    exit 1
fi

# Install dependencies.
apt-get update
apt-get install --no-install-recommends -y \
    gcc \
    libc6-dev \
    libyaml-dev \
    make

# Build autograder.
make -C "$GRADESCOPE_HARDEN_SRC" run_autograder

# Move /autograder/run_autograder to /autograder/run_autograder.orig and replace
# with our own run_autograder.
mv /autograder/run_autograder /autograder/run_autograder.orig
cp "$GRADESCOPE_HARDEN_SRC/run_autograder" /autograder/run_autograder
