//
// Graph.cpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Graph.hpp"
#include "Error.hpp"
#include "Rule.hpp"
#include "Target.hpp"
#include "BuildTool.hpp"
#include "ScriptInterface.hpp"
#include "Scheduler.hpp"
#include "OsInterface.hpp"
#include "persist.hpp"
#include <sweet/assert/assert.hpp>

using std::list;
using std::vector;
using std::string;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

/**
// Constructor.
*/
Graph::Graph()
: build_tool_( NULL ),
  root_target_(),
  cache_target_(),
  loaded_from_cache_( false ),
  traversal_in_progress_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 )
{
}

/**
// Constructor.
//
// @param build_tool
//  The BuildTool that this Graph is part of.
*/
Graph::Graph( BuildTool* build_tool )
: build_tool_( build_tool ),
  root_target_(),
  cache_target_(),
  loaded_from_cache_( false ),
  traversal_in_progress_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 )
{
    SWEET_ASSERT( build_tool_ );
    root_target_ = build_tool_->get_script_interface()->target( "", this );
}

/**
// Get the root Target.
//
// @return
//  The root Target.
*/
Target* Graph::get_root_target() const
{
    return root_target_.get();
}

/**
// Get the Target for the file that this Graph is loaded from.
//
// @return
//  The Target for this file that this Graph is loaded from or null if this
//  Graph was loaded from buildfiles and not a cache file.
*/
Target* Graph::get_cache_target() const
{
    return cache_target_.get();
}
        
/**
// Get the BuildTool that this Graph is part of.
//
// @return
//  The BuildTool.
*/
BuildTool* Graph::get_build_tool() const
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_;
}

/**
// Is this Graph loaded from a file?
//
// @return
//  True if this Graph was loaded from a file otherwise false.
*/
bool Graph::is_loaded_from_cache() const
{
    return loaded_from_cache_;
}

/**
// Mark this graph as being traversed and increment the visited and 
// successful revisions.
//
// The graph is marked as being traversed so that recursive calls to preorder
// or postorder can be detected and report errors.
//
// The visited and successful revisions are incremented so that any Targets 
// that were previously visited now have different visit and success 
// revisions and will be considered not visited and not successful.
*/
void Graph::begin_traversal()
{
    SWEET_ASSERT( !traversal_in_progress_ );
    traversal_in_progress_ = true;
    ++visited_revision_;
    ++successful_revision_;
}

/**
// Mark this graph as not being traversed.
*/
void Graph::end_traversal()
{
    SWEET_ASSERT( traversal_in_progress_ );
    traversal_in_progress_ = false;
}

/**
// Is this graph being traversed?
//
// @return
//  True if this graph is being traversed otherwise false.
*/
bool Graph::is_traversal_in_progress() const
{
    return traversal_in_progress_;
}

/**
// Get the current visited revision for this Graph.
//
// This is used to determine whether or not Targets have been visited during
// a pass.  At the begining of the pass Graph::increment_revisions() 
// is called to increment the visited and successful revisions for the pass.  
// Then when a Target is visited its visited revision is set to be the same 
// as the visited revision in the Graph.  A Target can then be checked to see 
// if it has been visited by checking if its visited revision is the same as 
// the current visited revision for its Graph.
//
// @return
//  The current visited revision for this Graph.
*/
int Graph::get_visited_revision() const
{
    return visited_revision_;
}

/**
// Get the current success revision for this Graph.
//
// This is used to determine whether or not Targets have been visited 
// successfully during a pass.  See Graph::get_visit_revision() for a
// description of how the revision values are used.
//
// @return
//  The current successful revision for this Graph.
*/
int Graph::get_successful_revision() const
{
    return successful_revision_;
}

/**
// Find or create a Target.
//
// Finds or create the Target by breaking \e id into '/' delimited elements.
// If a ".." element is encountered then the relative parent is moved up a 
// level otherwise a new Target is added with that identifier as a child of 
// the current relative parent and the next element is considered.
//
// @param id
//  The identifier of the Target to find or create.
//
// @param rule
//  The Rule of the Target to create or null to create a Target that has no
//  Rule.
//
// @param working_directory
//  The Target that the identifier is relative to or null if the identifier
//  is relative to the root Target.
//
// @return
//  The Target.
*/
ptr<Target> Graph::target( const std::string& id, ptr<Rule> rule, ptr<Target> working_directory )
{
//
// Find the Target by breaking the id into '/' delimited elements and 
// searching up or down the Target hierarchy relative to the current 
// working directory.
//
    path::Path path( id );
    ptr<Target> target( working_directory && path.is_relative() ? working_directory : root_target_ );
    SWEET_ASSERT( target );

    if ( !id.empty() )
    {
        path::Path::const_iterator i = path.begin();
        while ( i != path.end() )
        {
            const string& element = *i;
            ++i;
                   
            if ( element == "." )
            {
                target = target;
            }
            else if ( element == ".." )
            {
                SWEET_ASSERT( target );
                SWEET_ASSERT( target->get_parent() );
                SWEET_ASSERT( target->get_parent()->get_graph() == this );
                target = target->get_parent();
            }
            else
            {
                ptr<Target> new_target = target->find_target_by_id( element );
                if ( !new_target )
                {
                    new_target = build_tool_->get_script_interface()->target( element, this );
                    target->add_target( new_target, target );
                    if ( i != path.end() || !rule )
                    {
                        new_target->set_working_directory( target );
                        target->add_dependency( new_target );
                    }
                }

                target = new_target;
            }
        }
    }
    else
    {
        ptr<Target> new_target = build_tool_->get_script_interface()->target( "", this );
        target->add_target( new_target, target );
        target = new_target;
    }

    if ( target->get_rule() == NULL )
    {
        target->set_rule( rule );
    }

    if ( target->get_working_directory() == NULL )
    {
        target->set_working_directory( working_directory );
    }

    if ( target->get_rule() != rule )
    {
        SWEET_ERROR( RuleConflictError("The target '%s' has been created with prototypes '%s' and '%s'", id.c_str(), target->get_rule()->get_id().c_str(), rule ? rule->get_id().c_str() : "none" ) );
    }

    return target;
}

/**
// Find a Target in this Graph.
//
// Find the Target by breaking the id into '/' delimited elements and 
// searching up or down the Target hierarchy relative to 
// \e working_directory.
//
// @param id
//  The id of the Target to find.
//
// @param working_directory
//  The Target that the identifier is relative to or null if the identifier
//  is relative to the root Target.
//
// @return
//  The Target or null if no matching Target was found.
*/
ptr<Target> Graph::find_target( const std::string& id, ptr<Target> working_directory )
{
    ptr<Target> target;

    if ( !id.empty() )
    {
        path::Path path( id );
        target = working_directory && path.is_relative() ? working_directory : root_target_;
        SWEET_ASSERT( target );

        path::Path::const_iterator i = path.begin();
        while ( i != path.end() && target )
        {       
            if ( *i == "." )
            {
                target = target;
            }
            else if ( *i == ".." )
            {
                target = target->get_parent();
            }
            else
            {
                target = target->find_target_by_id( *i );
            }

            ++i;
        }    
    }

    return target;
}

/**
// Add \e target to this Graph as a child of \e working_directory.
//
// If the identifier of \e target is not empty then \e target is also added
// as a dependency of \e working_directory.
//
// @param target
//  The Target to add to this Graph (assumed not null).
//
// @param working_directory
//  The Target to add \e target to (assumed not null).
*/
void Graph::insert_target( ptr<Target> target, ptr<Target> working_directory )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( working_directory );
    
    working_directory->add_target( target, working_directory );
    if ( !target->get_id().empty() )
    {
        working_directory->add_dependency( target );
    }
}

/**
// Load a buildfile into this Graph.
//
// @param filename
//  The name of the buildfile to load.
//
// @param target
//  The Target that top level Targets in the buildfile are made dependencies 
//  of or null to make top level Targets dependencies of the target that 
//  corresponds to the working directory.
*/
void Graph::buildfile( const std::string& filename, ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( root_target_ );
     
    path::Path path( build_tool_->get_script_interface()->absolute(filename) );
    SWEET_ASSERT( path.is_absolute() );
    ptr<Target> working_directory = Graph::target( path.branch().string(), ptr<Rule>(), ptr<Target>() );
    root_target_->add_dependency( working_directory );
    ptr<Target> buildfile_target = Graph::target( path.string(), ptr<Rule>(), ptr<Target>() );
    buildfile_target->set_bind_type( BIND_SOURCE_FILE );
    if ( cache_target_ )
    {
        cache_target_->add_dependency( buildfile_target );
    }
    build_tool_->get_scheduler()->buildfile( path );
}

/**
// Make a postorder pass over this Graph to bind its Targets.
//
// @param target
//  The Target to begin the visit at or null to begin the visitation from 
//  the root of the Graph.
//
// @return
//  The number of Targets that failed to bind because they were files that
//  were expected to exist (see Target::set_required_to_exist()).
*/
int Graph::bind( ptr<Target> target )
{
    SWEET_ASSERT( !target || target->get_graph() == this );

    struct ScopedVisit
    {
        Target* target_;

        ScopedVisit( Target* target )
        : target_( target )
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( target_->is_visiting() == false );
            target_->set_visited( true );
            target_->set_visiting( true );
        }

        ~ScopedVisit()
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( target_->is_visiting() == true );
            target_->set_visiting( false );
        }
    };

    struct Bind
    {
        BuildTool* build_tool_;
        int failures_;
        
        Bind( BuildTool* build_tool )
        : build_tool_( build_tool ),
          failures_( 0 )
        {
            SWEET_ASSERT( build_tool_ );
            build_tool_->get_graph()->begin_traversal();
        }
        
        ~Bind()
        {
            build_tool_->get_graph()->end_traversal();
        }
    
        void visit( Target* target )
        {
            SWEET_ASSERT( target );

            if ( !target->is_visited() )
            {
                ScopedVisit visit( target );

                const vector<Target*>& dependencies = target->get_dependencies();
                for ( vector<Target*>::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i )
                {
                    Target* dependency = *i;
                    SWEET_ASSERT( dependency );

                    if ( !dependency->is_visiting() )
                    {
                        Bind::visit( dependency );
                    }
                    else
                    {
                        build_tool_->warning( "Ignoring cyclic dependency from '%s' to '%s' during binding", target->get_id().c_str(), dependency->get_id().c_str() );
                        dependency->set_successful( true );
                    }
                }

                target->bind();
                target->set_successful( true );

                if ( target->is_required_to_exist() && target->is_bound_to_file() && target->get_last_write_time() == 0 )
                {
                    build_tool_->error( "The source file '%s' does not exist", target->get_filename().c_str() );
                    ++failures_;
                }
            }
        }
    };

    Bind bind( build_tool_ );
    bind.visit( target ? target.get() : root_target_.get() );
    return bind.failures_;
}

/**
// Clear all of the targets in this graph.
*/
void Graph::clear( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    root_target_ = build_tool_->get_script_interface()->target( "", this );
    cache_target_ = target( filename );
    cache_target_->set_bind_type( BIND_GENERATED_FILE );
    loaded_from_cache_ = false;
}

/**
// Recover implicit relationships after this Graph has been loaded from an
// Archive.
//
// @param filename
//  The name of the file that this Graph was loaded from that is used to 
//  identify the cache target for this Graph (assumed not empty).
*/
void Graph::recover( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( root_target_ );
    root_target_->recover( this );
    cache_target_ = find_target( filename, ptr<Target>() );
    SWEET_ASSERT( cache_target_ );
    bind( cache_target_ );
    loaded_from_cache_ = true;
}

/**
// Load this Graph from a binary file.
//
// @param filename
//  The name of the file to load this Graph from.
//
// @return
//  The target that corresponds to the file that this Graph was loaded from or
//  null if there was no cache target.
*/
Target* Graph::load_xml( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( build_tool_ );

    root_target_.reset();
    cache_target_.reset();

    path::Path path( build_tool_->get_script_interface()->absolute(filename) );
    sweet::persist::XmlReader reader;
    enter( reader );
    reader.read( path.string().c_str(), "graph", *this );   
    exit( reader );
    recover( filename );
    return cache_target_.get();
}

/**
// Save this Graph to an XML file.
//
// @param filename
//  The name of the file to save this Graph to.
*/
void Graph::save_xml( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( build_tool_ );
    path::Path path( build_tool_->get_script_interface()->absolute(filename) );
    sweet::persist::XmlWriter writer;
    enter( writer );
    writer.write( path.string().c_str(), "graph", *this );
    exit( writer );
}

/**
// Load this Graph from a binary file.
//
// @param filename
//  The name of the file to load this Graph from.
//
// @return
//  The target that corresponds to the file that this Graph was loaded from or
//  null if there was no cache target.
*/
Target* Graph::load_binary( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( build_tool_ );

    root_target_.reset();
    cache_target_.reset();

    path::Path path( build_tool_->get_script_interface()->absolute(filename) );
    sweet::persist::BinaryReader reader;
    enter( reader );
    reader.read( path.string().c_str(), "graph", *this );   
    exit( reader );
    recover( filename );
    return cache_target_.get();
}

/**
// Save this Graph to a binary file.
//
// @param filename
//  The name of the file to save this Graph to.
*/
void Graph::save_binary( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( build_tool_ );
    path::Path path( build_tool_->get_script_interface()->absolute(filename) );
    sweet::persist::BinaryWriter writer;
    enter( writer );
    writer.write( path.string().c_str(), "graph", *this );
    exit( writer );
}

/**
// Print the dependency graph of Targets in this Graph.
//
// @param target
//  The Target to begin printing from or null to print from the root Target of
//  this Graph.
*/
void Graph::print_dependencies( ptr<Target> target )
{
    struct ScopedVisit
    {
        Target* target_;

        ScopedVisit( Target* target )
        : target_( target )
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( target_->is_visiting() == false );
            target_->set_visiting( true );
        }

        ~ScopedVisit()
        {
            SWEET_ASSERT( target_->is_visiting() );
            target_->set_visiting( false );
        }
    };

    struct RecursivePrinter
    {
        Graph* graph_;
        
        RecursivePrinter( Graph* graph )
        : graph_( graph )
        {
            SWEET_ASSERT( graph_ );
            graph_->begin_traversal();
        }
        
        ~RecursivePrinter()
        {
            graph_->end_traversal();
        }
    
        static const char* id( Target* target )
        {
            SWEET_ASSERT( target );
            if ( !target->get_id().empty() )
            {
                return target->get_id().c_str();
            }
            else if ( target->get_bind_type() == BIND_PHONY )
            {
                return target->get_path().c_str();
            }
            else
            {
                return target->get_filename().c_str();
            }
        }
        
        void print( Target* target, int level )
        {
            SWEET_ASSERT( target );
            SWEET_ASSERT( level >= 0 );

            ScopedVisit visit( target );

            std::time_t timestamp = target->get_timestamp();
            struct tm* time = ::localtime( &timestamp );

            printf( "\n" );
            for ( int i = 0; i < level; ++i )
            {
                printf( "    " );
            }

            if ( target->get_rule() )
            {
                printf( "%s ", target->get_rule()->get_id().c_str() );
            }

            printf( "'%s' %s %04d-%02d-%02d %02d:%02d:%02d", 
                id(target),
                target->is_outdated() ? "true" : "false", 
                time->tm_year + 1900, 
                time->tm_mon + 1, 
                time->tm_mday, 
                time->tm_hour, 
                time->tm_min, 
                time->tm_sec 
            );

            const vector<Target*>& dependencies = target->get_dependencies();
            for ( vector<Target*>::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i )
            {
                Target* dependency = *i;
                SWEET_ASSERT( dependency );
                if ( !dependency->is_visiting() )
                {
                    print( dependency, level + 1 );
                }
                else
                {
                    BuildTool* build_tool = target->get_graph()->get_build_tool();
                    SWEET_ASSERT( build_tool );
                    build_tool->warning( "Ignoring cyclic dependency from '%s' to '%s' while printing dependencies", target->get_id().c_str(), dependency->get_id().c_str() );
                }
            }
        }
    };

    RecursivePrinter recursive_printer( this );
    recursive_printer.print( target ? target.get() : root_target_.get(), 0 );
    printf( "\n\n" );
}

/**
// Print the Target namespace of this Graph.
//
// @param target
//  The Target to begin printing from or null to print from the root Target of
//  this Graph.
*/
void Graph::print_namespace( ptr<Target> target )
{
    struct RecursivePrinter
    {
        Graph* graph_;
        
        RecursivePrinter( Graph* graph )
        : graph_( graph )
        {
            SWEET_ASSERT( graph_ );
            graph_->begin_traversal();
        }
        
        ~RecursivePrinter()
        {
            graph_->end_traversal();
        }

        static void print( ptr<Target> target, int level )
        {
            SWEET_ASSERT( target != 0 );
            SWEET_ASSERT( level >= 0 );

            if ( !target->get_id().empty() )
            {
                printf( "\n" );
                for ( int i = 0; i < level; ++i )
                {
                    printf( "    " );
                }

                if ( target->get_rule() != 0 )
                {
                    printf( "%s ", target->get_rule()->get_id().c_str() );
                }

                printf( "'%s'", target->get_id().c_str() );
            }

            if ( !target->is_visited() )
            {
                target->set_visited( true );            
            
                const vector<ptr<Target> >& targets = target->get_targets();
                for ( vector<ptr<Target> >::const_iterator i = targets.begin(); i != targets.end(); ++i )
                {
                    ptr<Target> target = *i;
                    SWEET_ASSERT( target );
                    print( target, level + 1 );
                }
            }
        }
    };

    RecursivePrinter recursive_printer( this );
    recursive_printer.print( target ? target : root_target_, 0 );
    printf( "\n\n" );
}
