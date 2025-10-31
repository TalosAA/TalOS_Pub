. ./prepare-build-env.sh

SYSTEM_HEADER_PROJECTS="kernel"
PROJECTS="kernel"
BUILD_MODE=$1

if [ -n "$1" ]
then
  if [ "$1" = "RELEASE" ]
  then
    export BUILD_MODE="RELEASE"
    export CFLAGS=
  else
    export BUILD_MODE="DEBUG"
    export CFLAGS='-O0 -g'
    BUILD_MODE=DEBUG
  fi
else
  export BUILD_MODE="DEBUG"
  export CFLAGS='-O0 -g'
fi

echo Build in $BUILD_MODE mode.

DEFAULT_HOST=i686-elf

export MAKE=${MAKE:-make}
export HOST=$DEFAULT_HOST
export HOSTARCH=IA32

export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc
export MKDIR=mkdir

export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include

export CPPFLAGS=''

# Work around that the -elf gcc targets doesn't have a system include directory
# because it was configured with --without-headers rather than --with-sysroot.
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
  export CC="$CC -isystem=$INCLUDEDIR"
fi
