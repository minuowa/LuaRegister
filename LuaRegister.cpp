// LuaRester.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TLuaRegister.h"
#include "XBase.h"

/** @brief 
	vs外部工具中命令：$(ProjectDir) *.h luaRegister.cpp
 **/
int _tmain ( int argc, _TCHAR* argv[] )
{
	//TLuaRegister luaregister;
	//luaregister.init ( "..\\FiGameDemo\\", "*.h", "..\\FiGameDemo\\luaRegister.cpp" );

    if ( argc < 4 )
    {
        std::cout << "\nArgs is invalid!" ;
        std::cout << "Right args:path ext outfile";
        return 1;
    }
    GString out;
    out.Format ( _T ( "Args:\nPath:%s,Ext:%s,OutFile:%s" ), argv[1], argv[2], argv[3] );
    std::cout << ( out  );
    GString outfile = argv[1];
    outfile += argv[3];

	TLuaRegister luaregister;
	luaregister.init ( argv[1], argv[2], outfile );
    return 0;
}

