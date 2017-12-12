
local function call( cc, definition )
    build.merge( cc, definition );
    local settings = cc.settings;
    local architecture = cc.architecture;
    for _, value in ipairs(definition) do
        local source = file( value );
        source:set_required_to_exist( true );
        source.unit = cc;
        source.settings = settings;

        local object = file( ("%s/%s/%s/%s"):format(obj_directory(cc), architecture, relative(source:branch()), obj_name(value)) );
        object.source = value;
        object:add_dependency( source );
        object:add_dependency( Directory(object:branch()) );
        cc:add_dependency( object );
    end
end

local function depend( cc )
    for _, value in ipairs(cc) do
        local source_file = file( value );
        assert( source_file, ("Failed to find source file '%s' for scanning"):format(tostring(value)) );
        scan( source_file, CcScanner );
    end
end

local function build_( cc_ )
    if cc_:is_outdated() then
        cc( cc_ );
    end
end

local function clean( cc )
    for dependency in cc:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

local function create_target_prototype( id, language )
    local function create( target_prototype, architecture, settings )
        local cc = build.Target( anonymous(), target_prototype );
        cc.settings = settings or build.current_settings();
        cc.architecture = architecture;
        cc.language = language;
        return cc;
    end
    
    local target_prototype = build.TargetPrototype( id );
    target_prototype.create = create;
    target_prototype.call = call;
    target_prototype.depend = depend;
    target_prototype.build = build_;
    target_prototype.clean = clean;
    return target_prototype;
end

local Cc = create_target_prototype( "Cc", "c" );
local Cxx = create_target_prototype( "Cxx", "c++" );
local ObjC = create_target_prototype( "ObjC", "objective-c" );
local ObjCxx = create_target_prototype( "ObjCxx", "objective-c++" );

_G.Cc = Cc;
_G.Cxx = Cxx;
_G.ObjC = ObjC;
_G.ObjCxx = ObjCxx;
