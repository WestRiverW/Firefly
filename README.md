
Firefly
===========================
Firefly is a game server engine and developed by c++ language,open source,stable and robust.

****
	
|Author|WestRiverW|
|---|---
|Email|WestRiverW@163.com
|System|CentOS 8.2


****

**Deploy development environment.**

# Catalog
* [wget](#wget)
* [gcc](#gcc)
* [redis](#redis)
* [hiredis](#hiredis)
* [mysql](#mysql) 
* [curl](#curl)
* [openssl](#openssl)
* [cmake](#cmake)
* [gflags](#gflags) 
* [glog](#glog)
* [protobuf](#protobuf)
* [libevent](#libevent)
* [db](#db)
* [compile](#compile)

## wget
    yum -y install wget.

## gcc
	yum -y install gcc gcc-c++ gdb.
## redis
	1.wget http://download.redis.io/releases/redis-5.0.5.tar.gz
	2.tar xzf redis-5.0.5.tar.gz
	3.cd redis-5.0.5
	4.make && make install
## hiredis
	1.https://github.com/redis/hiredis
	2.make && make install

## mysql
	1.wget http://repo.mysql.com/mysql57-community-release-el7-10.noarch.rpm
	2.rpm -Uvh mysql57-community-release-el7-10.noarch.rpm
	3.yum module disable mysql -y
	4.yum -y install mysql-community-server
	5.systemctl start mysqld
	6.grep 'temporary password' /var/log/mysqld.log
	7.mysql -uroot -p
	8.set global validate_password_policy=0;
	9.set global validate_password_length=1;
	10.ALTER USER 'root'@'localhost' IDENTIFIED BY '123456';
	11.GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY '123456' WITH GRANT OPTION;
	12.FLUSH  PRIVILEGES;
	13.firewall-cmd --list-ports
	14.firewall-cmd --zone=public --add-port=3306/tcp ––permanent
	15.firewall-cmd --reload
	16.yum install mysql-devel –y
	17.[mysqld]
	   sql_mode=STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION

## curl
	1.yum -y install libcurl
	2.yum -y install curl-devel
## openssl
	1.github https://github.com/openssl/openssl
	2.unzip
	3. ./config
	4.make
	5.make install
	6.ln -s /usr/local/lib64/libssl.so.3 /usr/lib64/libssl.so.3
	7.ln -s /usr/local/lib64/libcrypto.so.3 /usr/lib64/libcrypto.so.3
	8.openssl version
## cmake
	1.wget https://cmake.org/files/v3.12/cmake-3.12.0-rc1.tar.gz
	2.tar -zxvf cmake-3.12.0-rc1.tar.gz
	3.cd cmake-3.12.0-rc1
	4. ./bootstrap
	5.gmake
	6.gmake install
	7.cmake --version
## gflags

	1.github https://github.com/gflags/gflags
	2.unzip gflags-master.zip
	3.cd gflags-master
	4.mkdir build && cd build
	5.cmake .. -DGFLAGS_NAMESPACE=google -
## glog
	1.github https://github.com/
	2.unzip glog-master.zip
	3.yum -y install autoconf automake libtool
	4.cd glog-master
	5. ./autogen.sh
	6. ./configure
	7.make -j 8
	8.make install
## protobuf
	1.github https://github.com/  download protobuf-master C++ code
	2. ./autogen.sh
	3. ./configure
	4. make -j 6
	5.make install
	6.protoc -I=./ --cpp_out=./ ./common.proto
## libevent
	1.wget https://github.com/libevent/libevent/releases/download/release-2.1.8-stable/libevent-2.1.8-stable.tar.gz
	2.tar -zxvf libevent-2.1.8-stable.tar.gz
	3.cd libevent-2.1.8-stable
	4. ./configure
	5.make
	6.make install
## db
	Find the 'River.sql' and run it.
## compile
Compile Firefly engine code that steps is below.

	1.compile lua
	2.yum –y install libtermcap-devel ncurses-devel libevent-devel readline-devel
	3.cd external/lua
	4.make linux
	5.cp lua.h luaconf.h lauxlib.h lualib.h /usr/local/include
	6.cp liblua.a /usr/local/lib
	7.cd Firefly
	8.make

[Top](#firefly)