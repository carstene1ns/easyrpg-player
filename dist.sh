#!/bin/bash
#
#
# by carstene1ns 2019, released in the public domain

set -e

function msg {
  if [ -z ${V} ]; then
    echo "${1}"
  else
    echo -e "\n${1}"
    local underline=$(printf "%-${#1}s" "=")
    echo "${underline// /=}"
  fi
}

function err {
  echo "${1}"
  exit 1
}

function has {
  hash ${1} 2> /dev/null || err "No '${1}' executable found"
}

function cleanup {
  echo "Something went wrong, cleaning up"
  [ -d "${TEMPDIR}" ] && rm -rf "${TEMPDIR}"
  [ -f ${TARFILE} ] && rm -f ${TARFILE}
}

[ -d .git ] || err "Not in a git repository (no .git directory found)"

# check tools
has git
has rm
has autoreconf
has a2x
has mktemp
has tar
has gzip
has xz

# options
V=""
WORKTREE_ATTRIBUTES=""
TREEISH=master
while getopts ":hvt:w" ARG; do
  case $ARG in
    v)
      V="-v"
      ;;
    w)
      WORKTREE_ATTRIBUTES="--worktree-attributes"
      ;;
    t)
      TREEISH=$OPTARG
      ;;
    h)
      echo "dist.sh - Generate EasyRPG Player distribution archives"
      echo "Options:"
      echo "  -h            - This help message"
      echo "  -v            - Run in verbose mode"
      echo "  -t <tree-ish> - Archive branch/tag/... instead of '${TREEISH}'"
      echo "  -w            - Use current working tree .gitattributes state"
      exit 0
      ;;
    \?)
      err "Invalid option: -$OPTARG."
      ;;
    :)
      err "Option -$OPTARG requires an argument."
  esac
done

RELEASE=$(git describe --abbrev=0 --tags)
TARFILE="easyrpg-player-${RELEASE}.tar"
PREFIX="easyrpg-player-${RELEASE}/"

EXTRA_FILES=(resources/easyrpg-player.6
             aclocal.m4
             config.h.in
             configure
             Makefile.in
             builds/autoconf/m4/pkg.m4
             builds/autoconf/aux/*)

msg "Deleting old archives"
rm ${V} -f ${TARFILE} ${TARFILE}.gz ${TARFILE}.xz

msg "Archiving repository (@${TREEISH})"
git archive ${V} ${WORKTREE_ATTRIBUTES} --prefix=${PREFIX} -o ${TARFILE} ${TREEISH} \
  || err "Cannot create archive"

msg "Extracting to temporary directory"
TEMPDIR=$(mktemp -d dist.XXXXX)
trap cleanup EXIT
tar -x ${V} -f ${TARFILE} --strip-components=1 -C "${TEMPDIR}"

msg "Generating manual page"
a2x ${V} -a player_version=${RELEASE} -f manpage -D "${TEMPDIR}"/resources \
  "${TEMPDIR}"/resources/easyrpg-player.6.adoc

msg "Adding autotools build system files:"
autoreconf -fi ${V} "${TEMPDIR}"

msg "Adding extra files"
tar -r ${V} -f ${TARFILE} --transform="s,^,$PREFIX," -C "$TEMPDIR" ${EXTRA_FILES[@]}

msg "Deleting temporary directory"
trap - EXIT
rm ${V} -rf "${TEMPDIR}"

msg "Compressing tar archive with gzip and xz"
GZIP= gzip -9 -k ${V} ${TARFILE}
XZ_OPT= xz -6e -T0 ${V} ${TARFILE}
