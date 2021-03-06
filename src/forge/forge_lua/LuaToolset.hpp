#ifndef FORGE_LUATOOLSET_HPP_INCLUDED
#define FORGE_LUATOOLSET_HPP_INCLUDED

#include <ctime>
#include <lua.hpp>

struct lua_State;

namespace sweet
{

namespace forge
{

class Toolset;

class LuaToolset
{
    lua_State* lua_state_; ///< The main Lua virtual machine to create the toolset API in.

public:
    static const char* TOOLSET_METATABLE;

    LuaToolset();
    ~LuaToolset();

    void create( lua_State* lua_state );
    void destroy();
    void create_toolset( Toolset* toolset );
    void update_toolset( Toolset* toolset );
    void destroy_toolset( Toolset* toolset );

    static int id( lua_State* lua_state );
    static int prototype( lua_State* lua_state );
    static int create_call_metamethod( lua_State* lua_state );
    static int continue_create_call_metamethod( lua_State* lua_state, int /*status*/, lua_KContext /*context*/ );
};

}

}

#endif
