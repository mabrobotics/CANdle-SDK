#!/usr/bin/env bash
rm -rf dist/ wheelhouse/
python3 -m build
pipx run cibuildwheel --platform linux --archs x86_64
python3 -m twine upload --skip-existing --repository testpypi dist/* wheelhouse/*
