# Baseline status

`build/validate.py` now accepts an explicit file path argument.

Current local binaries:

- `build/K.COM`: length `52544`; fails validation because it includes the POC
  changes currently present in `CODE.PAS`.
- `socher1/K.com.orig`: length `52098`; passes the expected length check but
  its digest does not match `build/validate.py`.

This means there are two separate baselines to keep distinct:

- The repository validation baseline encoded in `build/validate.py`.
- The local `socher1/K.com.orig` binary shipped in this workspace.

Before doing strict behavior parity tests, decide which binary should be treated
as the reference executable.
