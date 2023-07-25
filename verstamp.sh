#!/bin/bash

PROJECT_STRING="$(git remote get-url origin)"
REVISION_STRING=$(git rev-parse --short HEAD)
HASH_STRING=$(git rev-parse --short HEAD)
DATE_STRING=$(date +%m/%d/%y-%H:%M:%S-%Z%z)
BRANCH_STRING=$(git rev-parse --abbrev-ref HEAD)
TAG_STRING="$(git describe --tags)"
VERSION_STRING="$(git describe --abbrev=0 --tags)"
VERSION_INFO_LONG="project:$PROJECT_STRING branch:$BRANCH_STRING hash:$REVISION_STRING tag:$VERSION_STRING"
VERSION_INFO_STRING="$PROJECT_STRING $BRANCH_STRING $REVISION_STRING $VERSION_STRING"

if [ $# -eq 0 ]; then
    echo usage "verstamp.sh <sed options> <filename>"
    echo "Replaces these strings in your code:
    \"git_info:\" with \"git_info: ${VERSION_INFO_STRING}\"
    \"git_info_long:\" with \"git_info_long: ${VERSION_INFO_LONG}\"
    \"git_version:\" with \"git_version: ${VERSION_STRING}\"
    \"git_revision:\" with \"git_revision: ${REVISION_STRING}\"
    \"git_hash:\" with \"git_hash: ${HASH_STRING}\"
    \"git_branch:\" with \"git_branch: ${BRANCH_STRING}\"
    \"git_master:\" with \"git_master: ${MASTER_FORK}\"
    \"git_tag:\" with \"git_tag: ${VERSION_STRING}\"
    \"git_long_version:\" with \"git_info: ${VERSION_INFO_STRING}\"
    \"git_project:\" with \"git_project: ${PROJECT_STRING}\"
    \"version_info:\" with \"git_info: ${VERSION_INFO_STRING}\"
    \"timestamp:\" with \"timestamp: ${DATE_STRING}\"

    Replacement replaces the name and everything up to typical end-of-string characters: \",\r\n\t\"\')\"

    Replacement will update already replaced variables so you will not need a separate template file.

    Variables (git_info: etc) do not have to be in quotes but replacements won't have quotes either.

    git_long_version contains last tag, how may commits past last tag, and short hash

    Pass in additional, leading sed arguments if you like (such as typically -i to modify file in place).

    Example: ./verstamp.sh -i myprogram.cc anotherprogram.cc ...

    YOU MUST USE OPTION -i TO PERMANENTLY MODIFY THE FILE
    "
    exit 0
fi

# regex 's' command will use whatever character follows it as the delimter for the find <=> replace values

sed -e "s|git_info:[^,\r\n\t\"\'\`)]*|git_info: ${VERSION_INFO_STRING}|"  \
    -e "s|git_info_long:[^,\r\n\t\"\'\`)]*|git_info_long: ${VERSION_INFO_LONG}|" \
    -e "s|git_version:[^,\r\n\t\"\'\`)]*|git_version: ${VERSION_STRING}|" \
    -e "s|git_revision:[^,\r\n\t\"\'\`)]*|git_revision: ${REVISION_STRING}|" \
    -e "s|git_project:[^,\r\n\t\"\'\`)]*|git_project: ${PROJECT_STRING}|" \
    -e "s|timestamp:[^,\r\n\t\"\'\`)]*|timestamp: ${DATE_STRING}|"  \
    -e "s|date_time:[^,\r\n\t\"\'\`)]*|date_time: ${DATE_STRING}|"  \
    -e "s|git_branch:[^,\r\n\t\"\'\`)]*|git_branch: ${BRANCH_STRING}|"  \
    -e "s|git_hash:[^,\r\n\t\"\'\`)]*|git_hash: ${HASH_STRING}|"  \
    -e "s|git_master:[^,\r\n\t\"\'\`)]*|git_master: ${MASTER_FORK}|" \
    -e "s|git_tag:[^,\r\n\t\"\'\`)]*|git_tag: ${VERSION_STRING}|" $*

exit $?
