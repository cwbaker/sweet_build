//
// LuaTarget.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTarget.hpp"
#include "LuaForge.hpp"
#include "types.hpp"
#include <forge/Target.hpp>
#include <forge/TargetPrototype.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

static const char* TARGET_METATABLE = "build.Target";
static const char* STRING_VECTOR_CONST_ITERATOR_METATABLE = "build.vector<string>::const_iterator";

LuaTarget::LuaTarget()
: lua_state_( nullptr )
{
}

LuaTarget::~LuaTarget()
{
    destroy();
}

void LuaTarget::create( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    destroy();

    lua_state_ = lua_state;
    luaxx_create( lua_state_, this, TARGET_TYPE );

    static const luaL_Reg functions[] = 
    {
        { "id", &LuaTarget::id },
        { "path", &LuaTarget::path },
        { "branch", &LuaTarget::branch },
        { "prototype", &LuaTarget::prototype },
        { "set_cleanable", &LuaTarget::set_cleanable },
        { "cleanable", &LuaTarget::cleanable },
        { "set_built", &LuaTarget::set_built },
        { "built", &LuaTarget::built },
        { "timestamp", &LuaTarget::timestamp },
        { "last_write_time", &LuaTarget::last_write_time },
        { "outdated", &LuaTarget::outdated },
        { "set_filename", &LuaTarget::set_filename },
        { "filename", &LuaTarget::filename },
        { "filenames", &LuaTarget::filenames },
        { "directory", &LuaTarget::directory },
        { "set_working_directory", &LuaTarget::set_working_directory },
        { "add_dependency", &LuaTarget::add_explicit_dependency },
        { "remove_dependency", &LuaTarget::remove_dependency },
        { "add_implicit_dependency", &LuaTarget::add_implicit_dependency },
        { "clear_implicit_dependencies", &LuaTarget::clear_implicit_dependencies },
        { "add_ordering_dependency", &LuaTarget::add_ordering_dependency },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_pop( lua_state_, 1 );

    static const luaL_Reg implicit_creation_functions [] = 
    {
        { "parent", &LuaTarget::parent },
        { "working_directory", &LuaTarget::working_directory },
        { "dependency", &LuaTarget::explicit_dependency },
        { "dependencies", &LuaTarget::explicit_dependencies },
        { "implicit_dependency", &LuaTarget::implicit_dependency },
        { "implicit_dependencies", &LuaTarget::implicit_dependencies },
        { "ordering_dependency", &LuaTarget::ordering_dependency },
        { "ordering_dependencies", &LuaTarget::ordering_dependencies },
        { "any_dependency", &LuaTarget::any_dependency },
        { "any_dependencies", &LuaTarget::any_dependencies },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    lua_pushlightuserdata( lua_state_, this );
    luaL_setfuncs( lua_state_, implicit_creation_functions, 1 );    
    lua_pop( lua_state_, 1 );

    luaL_newmetatable( lua_state_, TARGET_METATABLE );
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaTarget::filename );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pop( lua_state_, 1 );

    luaL_newmetatable( lua_state_, STRING_VECTOR_CONST_ITERATOR_METATABLE );
    lua_pushcfunction( lua_state_, &vector_string_const_iterator_gc );
    lua_setfield( lua_state_, -2, "__gc" );
    lua_pop( lua_state_, 1 );

    const int BUILD = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, BUILD, "Target" );
}

void LuaTarget::destroy()
{
    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, this );
        lua_state_ = nullptr;
    }
}

void LuaTarget::create_target( Target* target )
{
    SWEET_ASSERT( target );

    if ( !target->referenced_by_script() )
    {
        luaxx_create( lua_state_, target, TARGET_TYPE );
        target->set_referenced_by_script( true );
        recover_target( target );
        update_target( target );
    }
}

void LuaTarget::recover_target( Target* target )
{
    SWEET_ASSERT( target );
    luaxx_push( lua_state_, target );
    luaL_setmetatable( lua_state_, TARGET_METATABLE );
    lua_pop( lua_state_, 1 );
}

void LuaTarget::update_target( Target* target )
{
    SWEET_ASSERT( target );

    TargetPrototype* target_prototype = target->prototype();
    if ( target_prototype )
    {
        luaxx_push( lua_state_, target );
        luaxx_push( lua_state_, target_prototype );
        lua_setmetatable( lua_state_, -2 );
        lua_pop( lua_state_, 1 );
    }
    else
    {
        luaxx_push( lua_state_, target );
        luaL_setmetatable( lua_state_, TARGET_METATABLE );
        lua_pop( lua_state_, 1 );
    }
}

void LuaTarget::destroy_target( Target* target )
{
    SWEET_ASSERT( target );
    luaxx_destroy( lua_state_, target );
    target->set_referenced_by_script( false );
}

int LuaTarget::id( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const string& id = target->id();
        lua_pushlstring( lua_state, id.c_str(), id.size() );
        return 1; 
    }
    return 0;
}

int LuaTarget::path( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const string& path = target->path();
        lua_pushlstring( lua_state, path.c_str(), path.size() );
        return 1; 
    }
    return 0;
}

int LuaTarget::branch( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const string& branch = target->branch();
        lua_pushlstring( lua_state, branch.c_str(), branch.size() );
        return 1; 
    }
    return 0;
}

int LuaTarget::parent( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* parent = target->parent();
        if ( parent )
        {
            if ( !parent->referenced_by_script() )
            {
                LuaTarget* lua_target = (LuaTarget*) lua_touserdata( lua_state, lua_upvalueindex(1) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( parent );
            }
            luaxx_push( lua_state, parent );
            return 1;
        }
    }
    return 0;
}

int LuaTarget::prototype( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        TargetPrototype* target_prototype = target->prototype();
        luaxx_push( lua_state, target_prototype );
        return 1;
    }
    return 0;
}

int LuaTarget::set_cleanable( lua_State* lua_state )
{
    const int TARGET = 1;
    const int CLEANABLE = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        bool cleanable = lua_toboolean( lua_state, CLEANABLE ) != 0;
        target->set_cleanable( cleanable );
    }
    return 0;
}

int LuaTarget::cleanable( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->cleanable() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::set_built( lua_State* lua_state )
{
    const int TARGET = 1;
    const int BUILT = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        bool built = lua_toboolean( lua_state, BUILT ) != 0;
        target->set_built( built );
    }
    return 0;
}

int LuaTarget::built( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->built() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::timestamp( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushinteger( lua_state, target->timestamp() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::last_write_time( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushinteger( lua_state, target->last_write_time() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::outdated( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->outdated() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::set_filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int FILENAME = 2;
    const int INDEX = 3;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        size_t length = 0;
        const char* filename = luaL_checklstring( lua_state, FILENAME, &length );
        int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
        luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
        target->set_filename( string(filename, length), index - 1 );
    }
    return 0;
}

int LuaTarget::filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
        luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
        --index;
        if ( index < int(target->filenames().size()) )
        {
            const string& filename = target->filename( index );
            lua_pushlstring( lua_state, filename.c_str(), filename.length() );
        }
        else
        {
            lua_pushlstring( lua_state, "", 0 );
        }
        return 1;
    }
    return 0;
}

int LuaTarget::filenames( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const vector<string>& filenames = target->filenames();
        luaxx_pushiterator( lua_state, filenames.begin(), filenames.end(), STRING_VECTOR_CONST_ITERATOR_METATABLE );
        return 1;
    }
    return 0;
}

int LuaTarget::directory( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
        luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
        --index;
        if ( index < int(target->filenames().size()) )
        {
            const string& directory = target->directory( index );
            lua_pushlstring( lua_state, directory.c_str(), directory.size() );
        }
        else
        {
            lua_pushlstring( lua_state, "", 0 );
        }
        return 1;
    }
    return 0;
}

int LuaTarget::set_working_directory( lua_State* lua_state )
{
    const int TARGET = 1;
    const int WORKING_DIRECTORY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* working_directory = (Target*) luaxx_to( lua_state, WORKING_DIRECTORY, TARGET_TYPE );
        target->set_working_directory( working_directory );
    }
    return 0;
}

int LuaTarget::working_directory( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {    
        Target* working_directory = target->working_directory();
        if ( !working_directory->referenced_by_script() )
        {
            LuaTarget* lua_target = (LuaTarget*) lua_touserdata( lua_state, lua_upvalueindex(1) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( working_directory );
        }
        luaxx_push( lua_state, working_directory );
        return 1;
    }
    return 0;
}

int LuaTarget::add_explicit_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_explicit_dependency( dependency );
    }
    return 0;
}

int LuaTarget::remove_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->remove_dependency( dependency );
    }
    return 0;
}

int LuaTarget::add_implicit_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_implicit_dependency( dependency );
    }
    return 0;
}

int LuaTarget::clear_implicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        target->clear_implicit_dependencies();
    }
    return 0;
}

int LuaTarget::add_ordering_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_ordering_dependency( dependency );
    }
    return 0;
}

int LuaTarget::any_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->any_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        luaxx_push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::any_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        Target* dependency = target->any_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            luaxx_push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::any_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target, TARGET, "expected target table" );

    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::any_dependencies_iterator, 2 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::explicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->explicit_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        luaxx_push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::explicit_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        Target* dependency = target->explicit_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            luaxx_push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::explicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );

    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::explicit_dependencies_iterator, 2 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::implicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->implicit_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        luaxx_push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::implicit_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        Target* dependency = target->implicit_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            luaxx_push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::implicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::implicit_dependencies_iterator, 2 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::ordering_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = int( luaL_optinteger(lua_state, INDEX, 1) );
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->ordering_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        luaxx_push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::ordering_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;
    if ( target && index <= finish )
    {
        Target* dependency = target->ordering_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            luaxx_push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::ordering_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );
    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );
    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::ordering_dependencies_iterator, 2 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::vector_string_const_iterator_gc( lua_State* lua_state )
{
    return luaxx_gc<vector<string>::const_iterator>( lua_state );
}