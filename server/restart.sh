#!/bin/bash

work_path=$(pwd)

if [ $# -eq 0 ]; then
	cd ${work_path}/center && ./restart.sh && sleep 1
	cd ${work_path}/gate && ./restart.sh && sleep 1
	cd ${work_path}/db && ./restart.sh && sleep 1
	cd ${work_path}/hall && ./restart.sh && sleep 1
	cd ${work_path}/game && ./restart.sh && sleep 1
	cd ${work_path}/web && ./restart.sh && sleep 1
elif [ $1 = "kill" ]; then
	cd ${work_path}/center && ./restart.sh 1
	cd ${work_path}/gate && ./restart.sh 1
	cd ${work_path}/db && ./restart.sh 1
	cd ${work_path}/hall && ./restart.sh 1
	cd ${work_path}/game && ./restart.sh 1
	cd ${work_path}/web && ./restart.sh 1
elif [ $1 = "clean" ]; then
	cd ${work_path}/center && rm -rf center.* core.* nohup.out
	cd ${work_path}/gate && rm -rf gate.* core.* nohup.out
	cd ${work_path}/db && rm -rf db.* core.* nohup.out
	cd ${work_path}/hall && rm -rf hall.* core.* nohup.out
	cd ${work_path}/game && rm -rf game.* core.* nohup.out
	cd ${work_path}/web && rm -rf web.* core.* nohup.out
fi

ps -ef | grep ${work_path}