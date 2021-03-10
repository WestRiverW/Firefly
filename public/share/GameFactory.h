#ifndef __GAME_MODULE_FACTORY_H__
#define __GAME_MODULE_FACTORY_H__
#include <common/GameCore.h>

using namespace Firefly;
class TableHook;

typedef int ( *CREATE_FUNC )( ITableHook *&, int & );
extern "C" int CreatObj( ITableHook *&tableHook, int &nGameType );

#endif
