docker run -e CCACHE_DIR=/ccache -v `pwd`/ccache:/ccache -v `pwd`/kernel:/kernel -i -t kernelbuilder $1
