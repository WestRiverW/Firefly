ParameterTable = 
{
	webport="56008",
	-- 中心服配置信息
	centerip="127.0.0.1",
	centerport="8300",
	maxconnection="128",
	--同IPdengl数量限制
	iplimitcount="100",
	-- 数据库配置信息
	dbip="192.168.1.90",
	dbport="55001",
	dbuser="root",
	dbpwd="Abc123654",
	-- 子游戏列表
	subGame= 
	{
		"../lib/libfzffc.so",
		"../lib/libagzrty.so"
	}
}

function ServerConfig()
	return [[{
        "configs":[
            {
                "//": "--gate",
                "server":1,
                "startport":56001,
                "maxconnection":4096
            },
            {
                "//": "--hall",
                "server":2,
                "startport":8600,
                "maxconnection":128
            },{
                "//": "--game",
                "server":3, 
                "startport":8700,
                "maxconnection":128
            },{
                "//": "--mq",
                "server":4,
                "startport":8800,
                "maxconnection":128
            },{
                "//": "--db",
                "server":5,
                "startport":8900,
                "maxconnection":128
            },{
                "//": "--web",
                "server":6, 
                "startport":8200,
                "maxconnection":128
            }
        ]
    }]];
end

function GetServerConfig(key)
    return ParameterTable[key];
end

function GetSubGame(key,index)
	return ParameterTable[key][index];
end
