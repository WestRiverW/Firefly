#!/bin/bash
bin='protoc'

${bin} -I=./ --cpp_out=./ common.proto
${bin} -I=./ --cpp_out=./ hall.proto
${bin} -I=./ --cpp_out=./ game.proto
${bin} -I=./ --cpp_out=./ db.proto
${bin} -I=./ --cpp_out=./ web.proto


