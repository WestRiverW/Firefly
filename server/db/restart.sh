#!/bin/bash

bin='db'
export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/lib/:/usr/local/lib64/:/usr/local/lib/:../../lib/:./

work_path=$(pwd)
bin_pid=`cat ${work_path}/${bin}.pid 2>/dev/null | awk -F' ' '{ print $2}'`

ulimit -c unlimited

if test -n "${bin_pid}"; then
	`kill -9 ${bin_pid} 2>/dev/null`
fi

datetag=`date '+%m_%d_%k_%M'`

`cd ${work_path}`
#ls ./*.trace  2>/dev/null | xargs -r tar -czf "./SERVER_TRACE.${datetag}.tar.gz"

#ls ./*.trace 2>/dev/null | xargs -r rm -fr

#ls ./*.log.* 2>/dev/null | xargs -r rm -fr
#ls ./core.*  2>/dev/null | xargs -r tar -czf "./CORE.${datetag}.tar.gz"

#ls ./core* 2>/dev/null | xargs -r rm -fr

#ls ./nohup.out 2>/dev/null | xargs -r rm -fr

if [ $# -eq 0 ];then
	nohup ${work_path}/${bin} --logbufsecs=0 --logbuflevel=-1 --log_dir=${work_path} --alsologtostderr=false --stderrthreshold=3 --vmodule=time_tracer=1 & 
fi

echo "${bin} $!" > ${work_path}/${bin}.pid




