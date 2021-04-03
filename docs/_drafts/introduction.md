---
layout: page
title: Introduction
parent: Getting Started
nav_order: 1
---

- TOC
{:toc}

- [Introduction](introduction.md) describes key concepts that will help you understand Forge and how to use it.

---

Forge is based on a dependency graph made of targets and dependencies between them.  Builds are performed by traversing the graph and carrying out different actions as each target is visited.  The type of each target, specified by its target prototype, determines exactly what those actions are.

The entry point of a build is a plain Lua script named *forge.lua* at the root of the project being built.  This root build script creates toolsets to represent the tools used in the build before loading one or more buildfiles to define the dependency graph.

The type of each toolset, specified by its toolset prototype, determines which tools are supported and their default settings.  The default settings are usually overridden from the root build script.  Several toolsets of the same and/or different types are typical to support, for example, building C++ for several architectures (several C++ type toolsets) along with associated data (built with a different type of toolset).

Buildfiles are plain Lua scripts with the *.build* extension that appear throughout the project directory hierarchy.  They use the toolsets from the root build script to create the dependency graph of targets that describes the build.

Target and toolset prototypes are to targets and toolsets in Forge as classes are to objects in a language like C++ or Java.  If you're familiar with prototypes in Lua or JavaScript then the prototypes in Forge are exactly that.
