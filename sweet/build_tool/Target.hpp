//
// Target.hpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_TARGET_HPP_INCLUDED
#define SWEET_BUILD_TOOL_TARGET_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/pointer/ptr.hpp>
#include <sweet/pointer/weak_ptr.hpp>
#include <ctime>
#include <string>
#include <vector>

struct lua_State;

namespace sweet
{

namespace build_tool
{

class Rule;
class Graph;
class BuildTool;

/**
// A Target.
*/
class SWEET_BUILD_TOOL_DECLSPEC Target : public enable_ptr_from_this<Target>
{
    std::string id_; ///< The identifier of this Target.
    mutable std::string path_; ///< The full path to this Target in the Target namespace.
    mutable std::string directory_; ///< The branch path to this Target in the Target namespace.
    Graph* graph_; ///< The Graph that this Target is part of.
    ptr<Rule> rule_; ///< The Rule for this Target or null if this Target has no Rule.
    int bind_type_; ///< How this Target should bind to files.
    std::time_t timestamp_; ///< The timestamp for this Target.
    std::time_t last_write_time_; ///< The last write time of the file that this Target is bound to.
    std::time_t last_scan_time_; ///< The last write time of the file that this Target was bound to when it was scanned.
    bool outdated_; ///< Whether or not this Target is out of date.
    bool changed_; ///< Whether or not this Target's timestamp has changed since the last time it was bound to a file.
    bool bound_to_file_; ///< Whether or not this Target is bound to a file.
    bool bound_to_dependencies_; ///< Whether or not this Target is bound to its dependencies.
    bool required_to_exist_; ///< Whether or not this Target is required to be bound to an existing file.
    std::string filename_; ///< The filename of this Target.
    weak_ptr<Target> working_directory_; ///< The Target that relative paths expressed when this Target is visited are relative to.
    weak_ptr<Target> parent_; ///< The parent of this Target in the Target namespace or null if this Target has no parent.
    std::vector<ptr<Target>> targets_; ///< The children of this Target in the Target namespace.
    std::vector<Target*> dependencies_; ///< The Targets that this Target depends on.
    bool visiting_; ///< Whether or not this Target is in the process of being visited.
    int visited_revision_; ///< The visited revision the last time this Target was visited.
    int successful_revision_; ///< The successful revision the last time this Target was successfully visited.
    int height_; ///< The height of this Target in the current or most recent dependency graph traversal.

    public:
        Target();
        Target( const std::string& id, Graph* graph );
        void recover( Graph* graph );

        const std::string& get_id() const;
        const std::string& get_path() const;
        const std::string& get_directory() const;
        Graph* get_graph() const;

        void set_rule( ptr<Rule> rule );
        ptr<Rule> get_rule() const;

        void set_bind_type( int bind_type );
        int get_bind_type() const;
        
        void bind();
        void bind_to_file();
        void bind_as_phony();
        void bind_as_source_file();
        void bind_as_intermediate_file();
        void bind_as_generated_file();

        void set_required_to_exist( bool required_to_exist );
        bool is_required_to_exist() const;

        void set_timestamp( std::time_t timestamp );
        std::time_t get_timestamp() const;
        std::time_t get_last_write_time() const;

        void set_last_scan_time( std::time_t last_scan_time );
        std::time_t get_last_scan_time() const;

        void set_outdated( bool outdated );
        bool is_outdated() const;
        bool is_changed() const;
        bool is_bound_to_file() const;

        void set_filename( const std::string& filename );
        const std::string& get_filename() const;

        void set_working_directory( ptr<Target> target );
        ptr<Target> get_working_directory() const;

        void set_parent( ptr<Target> target );
        ptr<Target> get_parent() const;

        void add_target( ptr<Target> target, ptr<Target> this_target );
        ptr<Target> find_target_by_id( const std::string& id ) const;
        const std::vector<ptr<Target>>& get_targets() const;

        void add_dependency( ptr<Target> target );
        void clear_dependencies();
        bool is_dependency( ptr<Target> target ) const;
        bool is_buildable() const;
        std::string generate_failed_dependencies_message() const;
        const std::vector<Target*>& get_dependencies() const;

        void set_visiting( bool visiting );
        bool is_visiting() const;

        void set_visited( bool visited );
        bool is_visited() const;

        void set_successful( bool successful );
        bool is_successful() const;

        void set_height( int height );
        int get_height() const;

        template <class Archive> void persist( Archive& archive );
};

/**
// Persist this Target with an Archive.
//
// @param archive
//  The Archive to persist this Target with.
//
// @return
//  Nothing.
*/
template <class Archive> 
void Target::persist( Archive& archive )
{
    archive.value( "id", id_ );
    archive.value( "rule", rule_ );
    archive.value( "bind_type", bind_type_ );
    archive.value( "last_write_time", last_write_time_ );
    archive.value( "last_scan_time", last_scan_time_ );
    archive.value( "filename", filename_ );
    archive.refer( "working_directory", working_directory_ );
    archive.value( "targets", "target", targets_ );
    archive.refer( "dependencies", "dependency", dependencies_ );
    sweet::lua::persist<Target, lua::LuaByReference>( archive, "object", *this );
}

}

namespace lua
{

void lua_push( lua_State* lua, std::time_t timestamp );
std::time_t lua_to( lua_State* lua, int position, const std::time_t* null_pointer_for_overloading );

}

}

#endif
