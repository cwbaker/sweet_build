
local gcc = {};

gcc.flags_by_architecture = {
    armv5 = '-march=armv5te -mtune=xscale -mthumb';
    armv7 = '-march=armv7 -mtune=xscale -mthumb';
    armv8 = '-march=armv8-a -mtune=xscale -mthumb';
    x86_64 = '';
};

function gcc.configure( settings )
    local local_settings = forge.local_settings;
    if not local_settings.gcc then
        local_settings.updated = true;
        local_settings.gcc = {
            architectures = { 'x86_64' };
            gcc = '/usr/bin/gcc';
            gxx = '/usr/bin/g++';
            ar = '/usr/bin/ar';
            environment = {
                PATH = '/usr/bin:/bin';
            };
        };
    end
end

function gcc.object_filename( forge, identifier )
    return ('%s.o'):format( identifier );
end

function gcc.static_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function gcc.dynamic_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.so'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function gcc.executable_filename( forge, identifier )
    local identifier = forge:interpolate( identifier );
    local filename = identifier;
    return identifier, filename;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function gcc.compile( forge, target ) 
    local settings = forge.settings;

    local flags = {};
    gcc.append_defines( forge, target, flags );
    gcc.append_include_directories( forge, target, flags );
    gcc.append_compile_flags( forge, target, flags );
    
    local gcc_ = settings.gcc.gcc;
    local environment = settings.gcc.environment;
    local ccflags = table.concat( flags, ' ' );
    local dependencies = ('%s.d'):format( target );
    local source = target:dependency();
    local output = target:filename();
    local input = forge:relative( source:filename() );
    print( forge:leaf(source:id()) );
    forge:system( 
        gcc_, 
        ('gcc %s -MMD -MF "%s" -o "%s" "%s"'):format(ccflags, dependencies, output, input), 
        environment,
        forge:dependencies_filter(target)
    );
end

-- Archive objects into a static library. 
function gcc.archive( forge, target )
    printf( forge:leaf(target) );
    local settings = forge.settings;
    forge:pushd( settings.obj_directory(target) );
    local objects =  {};
    for _, object in forge:walk_dependencies( target ) do
        local prototype = object:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx then
            table.insert( objects, forge:relative(object) );
        end
    end
    if #objects > 0 then
        local objects = table.concat( objects, '" "' );
        local ar = settings.gcc.ar;
        local environment = settings.gcc.environment;
        forge:system( ar, ('ar -rcs "%s" "%s"'):format(forge:native(target), objects), environment );
    end
    forge:popd();
end

-- Link dynamic libraries and executables.
function gcc.link( forge, target ) 
    printf( forge:leaf(target) );

    local objects = {};
    local libraries = {};
    local settings = forge.settings;
    forge:pushd( settings.obj_directory(target) );
    for _, dependency in forge:walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx then
            table.insert( objects, forge:relative(dependency) );
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            if dependency.whole_archive then 
                table.insert( libraries, '-Wl,--whole-archive' );
            end
            table.insert( libraries, ('-l%s'):format(dependency:id()) );
            if dependency.whole_archive then 
                table.insert( libraries, '-Wl,--no-whole-archive' );
            end
        end
    end

    local flags = {};
    gcc.append_link_flags( forge, target, flags );
    gcc.append_library_directories( forge, target, flags );
    gcc.append_link_libraries( forge, target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        local gxx = settings.gcc.gxx;
        local environment = settings.gcc.environment;
        forge:system( gxx, ('g++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
    end
    forge:popd();
end

local Cc = forge:PatternPrototype( 'Cc', gcc.object_filename );
Cc.language = 'c';
Cc.build = gcc.compile;
gcc.Cc = Cc;

local Cxx = forge:PatternPrototype( 'Cxx', gcc.object_filename );
Cxx.language = 'c++';
Cxx.build = gcc.compile;
gcc.Cxx = Cxx;

local StaticLibrary = forge:FilePrototype( 'StaticLibrary', gcc.static_library_filename );
StaticLibrary.build = gcc.archive;
gcc.StaticLibrary = StaticLibrary;

local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', gcc.dynamic_library_filename );
DynamicLibrary.build = gcc.link;
gcc.DynamicLibrary = DynamicLibrary;

local Executable = forge:FilePrototype( 'Executable', gcc.executable_filename );
Executable.build = gcc.link;
gcc.Executable = Executable;

-- Register the gcc C/C++ toolset in *forge*.
function gcc.register( forge )
    forge.Cc = gcc.Cc;
    forge.Cxx = gcc.Cxx;
    forge.StaticLibrary = gcc.StaticLibrary;
    forge.DynamicLibrary = gcc.DynamicLibrary;
    forge.Executable = gcc.Executable;
end

function gcc.append_defines( forge, target, flags )
    local settings = forge.settings;

    if not settings.assertions then 
        table.insert( flags, '-DNDEBUG' );
    end

    local defines = settings.defines;
    if defines then
        for _, define in ipairs(defines) do
            table.insert( flags, ('-D%s'):format(define) );
        end
    end
    
    local defines = target.defines;
    if defines then
        for _, define in ipairs(defines) do
            table.insert( flags, ('-D%s'):format(define) );
        end
    end
end

function gcc.append_include_directories( forge, target, flags )
    local settings = forge.settings;

    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( flags, ('-I "%s"'):format(forge:relative(directory)) );
        end
    end

    if settings.include_directories then
        for _, directory in ipairs(settings.include_directories) do
            table.insert( flags, ('-I "%s"'):format(directory) );
        end
    end
end

function gcc.append_compile_flags( forge, target, flags )
    local settings = forge.settings;

    table.insert( flags, "-c" );
    table.insert( flags, gcc.flags_by_architecture[settings.architecture] );
    table.insert( flags, "-fpic" );
    
    local language = target.language or 'c++';
    if language then
        table.insert( flags, ("-x %s"):format(language) );
        if string.find(language, "c++", 1, true) then
            table.insert( flags, "-std=c++11" );
            if settings.exceptions then
                table.insert( flags, "-fexceptions" );
            end
            if settings.run_time_type_info then
                table.insert( flags, "-frtti" );
            end
        end
    end
        
    if settings.debug then
        table.insert( flags, "-g3" );
    end

    if settings.optimization then
        table.insert( flags, "-O3" );
        table.insert( flags, "-Ofast" );
    end
    
    if settings.preprocess then
        table.insert( flags, "-E" );
    end

    if settings.runtime_checks then
        table.insert( flags, "-fstack-protector" );
    else
        table.insert( flags, "-fno-stack-protector" );
    end

    if settings.warnings_as_errors then 
        table.insert( flags, "-Werror" );
    end

    local warning_level = settings.warning_level
    if warning_level == 0 then 
        table.insert( flags, "-w" );
    elseif warning_level == 1 then
        table.insert( flags, "-Wall" );
    elseif warning_level >= 2 then
        table.insert( flags, "-Wall -Wextra" );
    end
end

function gcc.append_library_directories( forge, target, library_directories )
    local settings = forge.settings;

    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
    
    if settings.library_directories then
        for _, directory in ipairs(settings.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
end

function gcc.append_link_flags( forge, target, flags )
    local settings = forge.settings;

    table.insert( flags, gcc.flags_by_architecture[settings.architecture] );
    table.insert( flags, "-std=c++11" );

    if target:prototype() == forge.DynamicLibrary then
        table.insert( flags, "-shared" );
    end
    
    if settings.verbose_linking then
        table.insert( flags, "-verbose" );
    end
    
    if settings.debug then
        table.insert( flags, "-g" );
    end

    -- The latest GCC with Android (or clang with iOS) doesn't recognize 
    -- '-Wl,map' to specify the path to output a mapfile.
    -- if settings.generate_map_file then
    --     table.insert( flags, ('-Wl,-Map,"%s"'):format(forge:native(("%s.map"):format(target:filename()))) );
    -- end

    if settings.strip and not settings.generate_dsym_bundle then
        table.insert( flags, "-Wl,--strip-all" );
    end

    if settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(forge:absolute(settings.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(forge:native(target:filename())) );
end

function gcc.append_link_libraries( forge, target, libraries )
    local settings = forge.settings;

    if settings.libraries then 
        for _, library in ipairs(settings.libraries) do 
            table.insert( libraries, ('-l%s'):format(library) );
        end
    end

    if target.libraries then 
        for _, library in ipairs(target.libraries) do 
            table.insert( libraries, ('-l%s'):format(library) );
        end
    end
end

forge:register_module( gcc );
return gcc;
