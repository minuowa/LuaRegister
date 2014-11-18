#include "stdafx.h"
#include "TLuaRegister.h"
#include <stdio.h>
#include <io.h>
#include <iostream>
#include "fstream"
enum eTokenType
{
    eTokenTypeNone,
    eTokenTypeFilmClass,
    eTokenTypeFilmClassAndBase,
    eTokenTypeFilmTool,
};
TLuaRegister::TLuaRegister ( void )
    : mBuffer ( nullptr )
{
    mLexer.bind ( eTokenTypeNone, "virtual" );
    mLexer.bind ( eTokenTypeNone, "const" );
    mLexer.bind ( eTokenTypeNone, "inline" );
    mLexer.bind ( eTokenTypeFilmClass, "DeclareFilmObj" );
    mLexer.bind ( eTokenTypeFilmClassAndBase, "DeclareFilmObjBase" );
    mLexer.bind ( eTokenTypeFilmTool, "DeclareFilmTool" );
}


TLuaRegister::~TLuaRegister ( void )
{
    dSafeDeleteArray ( mBuffer );
    mLuaObjs.destroySecond();
}

bool TLuaRegister::init ( const char* path, const char* ext , const char* saveFile )
{
    GString findstr = path;
    findstr += ext;
    _finddata_t fileDir;
    long lfDir;

    if ( ( lfDir = _findfirst ( findstr.c_str(), &fileDir ) ) == -1l )
        return false;
    do
    {
        GString filename;
        filename += path;
        filename += fileDir.name;
		//if (dStrEqual(fileDir.name,"GSceneMgr.h"))
		//{
		//	OutputDebugStringA("1");
		//}
        if ( parserFile ( filename.c_str() ) )
        {
			//printf ( "%s\n", fileDir.name );
			mHeaders.push_back ( fileDir.name );
        }
    }
    while ( _findnext ( lfDir, &fileDir ) == 0 );

    _findclose ( lfDir );
    saveToFile ( saveFile );
    return true;
}

bool TLuaRegister::parseCurTokens()
{
    mCurClass.clear();

    bool luaToken = false;
    CXASSERT_RETURN_FALSE ( mLexer.parser ( ( const char* ) mBuffer ) );
    while ( !mLexer.isEnd() )
    {
        CXCharToken token;
        mLexer.next ( token );
        switch ( token.mType )
        {
        case eTokenTypeFilmClass:
        {
            mLexer.next ( token );
            luaToken = true;
            mCurClass = token.mStr;
            TLuaObj* luaobj = 0;
            CXASSERT_RETURN_FALSE ( !mLuaObjs.Get ( mCurClass, luaobj ) );
            luaobj = new TLuaObj;
            luaobj->mThisClass = mCurClass;
            mLuaObjs.Insert ( mCurClass, luaobj );
        }
        break;
        case eTokenTypeFilmClassAndBase:
        {
            luaToken = true;
            mLexer.next ( token );
            mCurClass = token.mStr;
            mLexer.next ( token );
            GString basetype = token.mStr;

            TLuaObj* luaobj = 0;
            CXASSERT_RETURN_FALSE ( !mLuaObjs.Get ( mCurClass, luaobj ) );
            luaobj = new TLuaObj;
            luaobj->mThisClass = mCurClass;
            luaobj->mClassBase = basetype;
            mLuaObjs.Insert ( mCurClass, luaobj );
        }
        break;
        case eTokenTypeFilmTool:
        {
            luaToken = true;
            mLexer.next ( token );
            if ( token.mType == eTokenTypeNone )
            {
                while ( token.mType == eTokenTypeNone )
                    mLexer.next ( token );
            }
            mLexer.next ( token );
            GString func = token.mStr;

            TLuaObj* luaobj = 0;
            if ( mLuaObjs.Get ( mCurClass, luaobj ) )
            {
                luaobj->mFunctions.push_back ( func );
            }
            else
            {
                mGlobalFunctions.push_back ( func );
            }
        }
        break;
        default:
            break;
        }
    }
    return luaToken;
}

bool TLuaRegister::parserFile ( const char* fileName )
{
    dSafeDeleteArray ( mBuffer );
    CXFileLoader fileloader;
    CXASSERT_RETURN_FALSE ( fileloader.OpenFile ( fileName ) );
    mBuffer = new Char[fileloader.GetBufferSize() + 1];
    memset ( mBuffer, 0, fileloader.GetBufferSize() + 1 );
    fileloader.ReadToBuffer ( mBuffer );
    return parseCurTokens();
}
//gLuaScript.regClass<GNode> ( "GNode" );
//gLuaScript.regClass<GTerrain, GNode> ( "GTerrain" );
//gLuaScript.regClassFunction<GTerrain> ( "recreate", &GTerrain::recreate );
//gLuaScript.regClassCreator<GTerrain>();
//gLuaScript.regGlobalFun ( "getSceneMgr", getSceneMgr );


void addSpace ( std::ofstream& stream )
{
    stream << "\t";
}
void TLuaRegister::saveToFile ( const char* file )
{
    std::ofstream ofs ( file );
    if ( ofs.is_open() )
    {
        /** @brief write header **/
        ofs << "#include \"GGameDemoHeader.h\"\n";
for ( auto h : mHeaders )
        {
            ofs << "#include \"" << h << "\"\n";
        }



        if ( !mLuaObjs.empty() )
        {
            addSpace ( ofs );
            ofs << "\n"
                << "int luaRegistAll()\n"
                << "{";

for ( auto g: mGlobalFunctions )
            {
                ofs << "\n";
                addSpace ( ofs );
                ofs << "gLuaScript.regGlobalFun ( \"" << g << "\", " << g << " );\n";
            }

for ( auto obj: mLuaObjs )
            {
                ofs << "\n";
                addSpace ( ofs );
                TLuaObj* pobj = obj.second;

                assert ( !pobj->mThisClass.empty() );
                ofs << "gLuaScript.regClass<" << pobj->mThisClass.c_str();
                if ( !pobj->mClassBase.empty() )
                    ofs << "," << pobj->mClassBase.c_str();
                ofs << ">( \"" << pobj->mThisClass.c_str() << "\" );\n";

                //constructor
                addSpace ( ofs );
                ofs << "gLuaScript.regClassCreator<" << pobj->mThisClass.c_str() << ">( );\n";

for ( auto fun: pobj->mFunctions )
                {
                    addSpace ( ofs );
                    ofs << "gLuaScript.regClassFunction<" << pobj->mThisClass.c_str() << ">";
                    ofs << "( \"" << fun << "\", &" << pobj->mThisClass << "::" << fun << " );\n";
                }
            }
            ofs << "\n";
            addSpace ( ofs );
            ofs << "return 0;\n}";
        }
        ofs.close();
    }
}