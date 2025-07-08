#!/bin/bash
set -e  # exit on error
for f in ./build/ut_runners/*; do
  bash -c "$f"
done
