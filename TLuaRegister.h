#ifndef TLuaRegister_h__
#define TLuaRegister_h__

#include "XBase.h"
struct TLuaObj
{
    GString mClassBase;
    GString mThisClass;
    GStringArr mFunctions;
};
//-------------------------------------------------------------------------
/**
	@brief 
	@author nnboy,18/11/2014  15:31
*/
class TLuaRegister
{
public:
    TLuaRegister ( void );
    ~TLuaRegister ( void );
	bool init ( const char* path, const char* ext ,const char* saveFile);
private:
    bool parserFile ( const char* fileName );
	bool parseCurTokens();
	void saveToFile(const char* file);
    GStringArr mHeaders;
	GStringArr mGlobalFunctions;
    CXMap<GString, TLuaObj*> mLuaObjs;
    CXCharLexer mLexer;
    GString mCurClass;
    char* mBuffer;
};

#endif // TLuaRegister_h__
