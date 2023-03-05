#!/bin/sh

# setup.sh re-execs the original /autograder/setup.sh with GRADESCOPE_HARDEN
# set, so just return when this is already set.
if [ "${GRADESCOPE_HARDEN+x}" = 'x' ]; then
    return
fi

set -eux

GRADESCOPE_HARDEN_SRC=/autograder/source/gradescope-harden/source

# Ensure the existence of the config file.
if [ ! -f /autograder/source/gradescope-harden.yml ]; then
    echo 'gradescope-harden.yml config file not found! Please make sure the file is included in your autograder.' >&2
    exit 1
fi

# Install dependencies.
apt-get update
apt-get install -y \
    python3 \
    python3-pip \
    python3-seccomp
pip3 install -r "$GRADESCOPE_HARDEN_SRC/requirements.txt"

# Move /autograder/run_autograder to /autograder/run_autograder.orig and replace
# with our own run_autograder.
mv /autograder/run_autograder /autograder/run_autograder.orig
cp "$GRADESCOPE_HARDEN_SRC/run_autograder" /autograder/run_autograder

# Re-exec calling script with GRADESCOPE_HARDEN set.
GRADESCOPE_HARDEN=true exec "$SHELL" "$0" "$@"
