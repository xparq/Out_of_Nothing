#!/bin/sh

commit_hash=$(git rev-parse --short=8 HEAD)
echo "const char* LAST_COMMIT_HASH = \"${commit_hash}\";" > ${COMMIT_HASH_INCLUDE_FILE}
