#!/bin/bash

VERSION=$(git describe --tags --abbrev=0)
TARGET="queue"
MAKE_BINARY="bin/main"
BUILD_DIR="build"

mkdir -p $BUILD_DIR

PLATFORM=""
OPTIONS=("linux/amd64" "darwin/arm64")
select opt in "${OPTIONS[@]}"
do
    case $opt in
        "linux/amd64")
            echo "Building for linux/amd64..."
            PLATFORM="linux/amd64"
            break
            ;;
        "darwin/arm64")
            echo "Building for darwin/arm64..."
            PLATFORM="darwin/arm64"
            break
            ;;
        *)
            echo "Invalid option $REPLY"
            exit 1
            ;;
    esac
done

platform_split=(${PLATFORM//\// })
OS=${platform_split[0]}
ARCH=${platform_split[1]}

make
mv $MAKE_BINARY $BUILD_DIR/$TARGET
tar -czf $BUILD_DIR/$TARGET-$VERSION-$OS-$ARCH.tar.gz -C $BUILD_DIR $TARGET

echo "Build completed: $BUILD_DIR/$TARGET-$VERSION-$OS-$ARCH.tar.gz"

rm $BUILD_DIR/$TARGET
