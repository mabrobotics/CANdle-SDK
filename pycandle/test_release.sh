#!/usr/bin/env bash
rm -rf dist/ wheelhouse/
pipx run build
pipx run cibuildwheel --platform linux --archs x86_64
python3 -m twine upload --skip-existing --repository testpypi dist/* wheelhouse/*
