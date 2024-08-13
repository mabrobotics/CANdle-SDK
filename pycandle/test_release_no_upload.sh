#!/usr/bin/env bash
rm -rf dist/ wheelhouse/
python3 -m build
pipx run cibuildwheel --platform linux --archs x86_64
