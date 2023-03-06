# Gradescope-Harden: Hardening Gradescope autograders

Gradescope-Harden is a drop-in solution for Gradescope autograders to provide hardening against malicious student code, a well-known problem for autograders.

## Features and Configuration

The most basic `gradescope-harden.yml` file looks like this:

```yaml
disable_networking: true
```

### Network Isolation

If `disable_networking` is set to `true`, Gradescope-Harden allows the operator to disable outgoing network connections to prevent issues like test case exfiltration and reverse shells. This is implemented as a seccomp filter for `socket` syscalls of domain AF\_INET or AF\_INET6.

**Note:** This will also disable connections to `localhost`, which may interfere with some autograders. A fix is currently being implemented for this.

## Quick Start

1. Clone this repository as a Git submodule in your own autograder source tree:

   ```shell
   git submodule add https://github.com/nicholasngai/gradescope-harden.git
   ```

2. Add the following line to the top of your `setup.sh`:

   ```shell
   . /autograder/source/gradescope-harden/source/setup.sh
   ```

3. Create a `gradescope-harden.yml` config file and add it to the **root** of your `autograder.zip` file.

4. Add all files in the `gradescope-harden/source` directory under **a subdirectory of the same name** in your `autograder.zip` file.

An example of this setup (using a symlink instead of a Git submodule) is available in the `example` directory, taken from [gradescope/autograder_samples](https://github.com/gradescope/autograder_samples).

## Acknowledgements

This project is inspired by some of the work in [saligrama/securescope](https://github.com/saligrama/securescope)!
