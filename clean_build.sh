#!/usr/bin/env bash

set -euo pipefail

project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"

find "$project_root" -type d -name build -prune -print0 |
    while IFS= read -r -d '' build_dir; do
        printf 'Removing %s\n' "$build_dir"
        rm -rf -- "$build_dir"
    done

printf 'All build directories have been removed.\n'
