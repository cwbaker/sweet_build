
CxxPrototype = TargetPrototype { "Cxx" };

function CxxPrototype.depend( source )
    for _, value in ipairs(source) do
        local source_file = file( value );
        assert( source_file, "Failed to find source file '%s' for scanning" % tostring(value) );
        scan( source_file, CcScanner );
    end
end

function CxxPrototype.build( source )
    if source:is_outdated() then
        cc( source );
    end
end

function CxxPrototype.clean( source )
    for dependency in source:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

function Cxx( definition )
    assert( type(definition) == "table" );
    return function( architecture )
        local source = target( "", CxxPrototype, build.copy(definition) );
        local settings = build.current_settings();
        source.settings = settings;
        source.architecture = architecture;

        if build.built_for_platform_and_variant(source) then
            local directory = Directory( "%s/%s" % {obj_directory(source), architecture} );

            for _, value in ipairs(source) do
                local source_file = file( value );
                source_file:set_required_to_exist( true );
                source_file.unit = source;
                source_file.settings = settings;

                local object = file( "%s/%s/%s" % {obj_directory(source_file), architecture, obj_name(value)} );
                object.source = value;
                source.object = object;
                object:add_dependency( source_file );
                object:add_dependency( directory );
                source:add_dependency( object );
            end
        end
        
        return source;
    end
end
