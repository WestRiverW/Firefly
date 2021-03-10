#!/bin/bash
bin='protoc'

${bin} -I=./ --cpp_out=./ jdddz.proto
