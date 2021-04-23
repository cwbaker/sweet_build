# Forge

![Build](https://github.com/cwbaker/forge/workflows/Build/badge.svg)

*Forge* is a Lua scriptable build tool that tracks dependencies between files, uses relative timestamps to determine which are out of date, and carries out actions to bring those files up to date.

## Example

Copy `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out`:

~~~lua
local toolset = require 'forge' {
    output = root( 'output' );
};

toolset:all {
    toolset:Copy '${output}/%1.out' {
        'bar.in';
        'baz.in';
        'foo.in';
    };    
};
~~~

The Lua script that defines the `Copy` target used in the Lua-based domain specific language above is defined as follows:

~~~lua
local Copy = PatternPrototype( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

Execute the build by running `forge` from a directory within the project:

~~~bash
$ forge
~~~

## Documentation

See [https://cwbaker.github.io/forge/](https://cwbaker.github.io/forge/) for documentation including:

- [Installation](https://cwbaker.github.io/forge/installation/) in development and build environments.

- [Getting Started](https://cwbaker.github.io/forge/getting-started/) to get up and running.

- [Copy Files](https://cwbaker.github.io/forge/tutorials/copy-files.md) and [Compile and Link C++](https://cwbaker.github.io/forge/tutorials/compile-and-link-cxx.md) for examples of Forge in practice.

- [Reference](https://cwbaker.github.io/forge/reference/) describing the Forge API in detail.

## Contributions

All contributions and feedback welcomed.  Please use the Github issue tracker and/or open a pull request.  Thanks!

## License

*Forge* is licensed under the [MIT License](http://www.opensource.org/licenses/MIT)
