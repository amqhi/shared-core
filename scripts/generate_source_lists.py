import os
import sys
import json
import zipfile
import shutil
import pathlib

# Use the directory containing this script as the root.
ROOT_DIR = pathlib.Path(__file__).parent.parent.resolve()

def get_shared_definitions():
    """Return the shared sources and include paths."""
    return {
        "include_dirs": [
            ROOT_DIR / "src",
            ROOT_DIR / "src" / "utils",
            ROOT_DIR / "src" / "models",
            ROOT_DIR / "deps",
            ROOT_DIR / "deps" / "sqlite",
            ROOT_DIR / "deps" / "json",
            ],
        "sources": [
            ROOT_DIR / "deps" / "sqlite" / "sqlite3.c",
            ROOT_DIR / "deps" / "sqlite" / "sqlite3.h",
            ROOT_DIR / "deps" / "json" / "json.hpp",
            ROOT_DIR / "src" / "core_base.cpp",
            ROOT_DIR / "src" / "core_base.h",
            ROOT_DIR / "src" / "item_manager_base.cpp",
            ROOT_DIR / "src" / "item_manager_base.h",
            ROOT_DIR / "src" / "cached_state_base.cpp",
            ROOT_DIR / "src" / "cached_state_base.h",
            ROOT_DIR / "src" / "settings_base.cpp",
            ROOT_DIR / "src" / "settings_base.h",
            ROOT_DIR / "src" / "utils" / "map_utils.cpp",
            ROOT_DIR / "src" / "utils" / "map_utils.h",
            ROOT_DIR / "src" / "utils" / "vector_utils.cpp",
            ROOT_DIR / "src" / "utils" / "vector_utils.h",
            ROOT_DIR / "src" / "utils" / "sqlite_utils.cpp",
            ROOT_DIR / "src" / "utils" / "sqlite_utils.h",
            ROOT_DIR / "src" / "utils" / "json_utils.cpp",
            ROOT_DIR / "src" / "utils" / "json_utils.h",
            ROOT_DIR / "src" / "notifier_base.h",
            ROOT_DIR / "src" / "network_provider_base.cpp",
            ROOT_DIR / "src" / "network_provider_base.h",
            ROOT_DIR / "src" / "database_provider_base.cpp",
            ROOT_DIR / "src" / "database_provider_base.h",
            ROOT_DIR / "src" / "secure_storage_provider_base.h",
            ROOT_DIR / "src" / "platform_utils_base.h",
            ROOT_DIR / "src" / "utils" / "date_time_utils.cpp",
            ROOT_DIR / "src" / "utils" / "date_time_utils.h",
            ROOT_DIR / "src" / "utils" / "mime_utils.h",
            ROOT_DIR / "src" / "utils" / "name_generator.cpp",
            ROOT_DIR / "src" / "utils" / "name_generator.h",
            ROOT_DIR / "src" / "utils" / "network_error.cpp",
            ROOT_DIR / "src" / "utils" / "network_error.h",
            ROOT_DIR / "src" / "utils" / "request_failure.cpp",
            ROOT_DIR / "src" / "utils" / "request_failure.h",
            ROOT_DIR / "src" / "utils" / "platform.h",
            ROOT_DIR / "src" / "models" / "user.cpp",
            ROOT_DIR / "src" / "models" / "user.h",
            ROOT_DIR / "src" / "models" / "item.cpp",
            ROOT_DIR / "src" / "models" / "item.h",
            ROOT_DIR / "src" / "models" / "auth_tokens.cpp",
            ROOT_DIR / "src" / "models" / "auth_tokens.h",
            ROOT_DIR / "src" / "models" / "item_attributes.h",
            ROOT_DIR / "src" / "models" / "item_attributes.cpp",
            ROOT_DIR / "src" / "common_api.cpp",
            ROOT_DIR / "src" / "common_api.h",
            ROOT_DIR / "src" / "models" / "sync_event.cpp",
            ROOT_DIR / "src" / "models" / "sync_event.h",
            ROOT_DIR / "src" / "models" / "file_metadata.cpp",
            ROOT_DIR / "src" / "models" / "file_metadata.h",
            ROOT_DIR / "src" / "models" / "folder_metadata.cpp",
            ROOT_DIR / "src" / "models" / "folder_metadata.h",
            ROOT_DIR / "src" / "models" / "uuid.cpp",
            ROOT_DIR / "src" / "models" / "uuid.h",
            ],
        "java_include_dirs": [
            ROOT_DIR / "bindings" / "java",
            ],
        "java_sources": [
            ROOT_DIR / "bindings" / "java" / "java_string.h",
            ROOT_DIR / "bindings" / "java" / "java_map.h",
            ROOT_DIR / "bindings" / "java" / "java_network_callback.cpp",
            ROOT_DIR / "bindings" / "java" / "java_network_callback.h",
            ROOT_DIR / "bindings" / "java" / "java_list.h",
            ROOT_DIR / "bindings" / "java" / "java_list.cpp",
            ROOT_DIR / "bindings" / "java" / "java_item.h",
            ROOT_DIR / "bindings" / "java" / "java_uuid.h",
            ROOT_DIR / "bindings" / "java" / "java_user.h",
            ROOT_DIR / "bindings" / "java" / "java_integer.h",
            ROOT_DIR / "bindings" / "java" / "java_long.h",
            ]
    }

def generate_cmake_file(defs):
    """Generate sources.cmake."""
    cmake_path = ROOT_DIR / "sources.cmake"

    def to_cmake_path(p):
        rel = pathlib.Path(p).relative_to(ROOT_DIR)
        return f"${{CMAKE_CURRENT_LIST_DIR}}/{str(rel).replace(os.sep, '/')}"

    inc_str = "\n".join([f"    {to_cmake_path(p)}" for p in defs["include_dirs"]])
    src_str = "\n".join([f"    {to_cmake_path(p)}" for p in defs["sources"]])
    java_inc_str = "\n".join([f"    {to_cmake_path(p)}" for p in defs["java_include_dirs"]])
    java_src_str = "\n".join([f"    {to_cmake_path(p)}" for p in defs["java_sources"]])

    cmake_content = f"""# AUTO-GENERATED FILE. DO NOT EDIT DIRECTLY.
# Generated by update_source_paths_for_bindings.py

set(SHARED_INCLUDE_DIRS
{inc_str}
)

set(SHARED_SOURCES
{src_str}
)

set(SHARED_JAVA_BINDING_INCLUDE_DIRS
{java_inc_str}
)

set(SHARED_JAVA_BINDING_SOURCES
{java_src_str}
)
"""
    with open(cmake_path, "w", encoding="utf-8") as f:
        f.write(cmake_content)
    print(f"[+] Generated CMake file: {cmake_path}")

def generate_msbuild_props(defs):
    """Generate sources.props for Visual Studio / MSBuild."""
    props_path = ROOT_DIR / "sources.props"

    def to_msbuild_path(p):
        rel = pathlib.Path(p).relative_to(ROOT_DIR)
        return f"$(MSBuildThisFileDirectory){str(rel).replace('/', '\\')}"

    def compile_entry(ms_path, *, java=False, compile_as_c=False):
        attrs = []
        if java:
            attrs.append('Condition="\'$(IncludeJavaBindings)\' == \'true\'"')
        lines = [f'    <ClCompile Include="{ms_path}"' + (f' {attrs[0]}' if attrs else '') + '>']
        lines.append('      <PrecompiledHeader>NotUsing</PrecompiledHeader>')
        if compile_as_c:
            lines.append('      <CompileAs>CompileAsC</CompileAs>')
        lines.append('    </ClCompile>')
        return '\n'.join(lines)

    def include_entry(ms_path, *, java=False):
        if java:
            return (
                f'    <ClInclude Include="{ms_path}" '
                f'Condition="\'$(IncludeJavaBindings)\' == \'true\'" />'
            )
        return f'    <ClInclude Include="{ms_path}" />'

    include_dirs_str = ";".join([to_msbuild_path(p) for p in defs["include_dirs"]]) + ";"
    java_include_dirs_str = ";".join([to_msbuild_path(p) for p in defs["java_include_dirs"]]) + ";"

    cl_compile, cl_include = [], []
    for src in defs["sources"]:
        p = pathlib.Path(src)
        ms_path = to_msbuild_path(p)
        ext = p.suffix.lower()
        if ext in [".cpp", ".c", ".cc", ".cxx"]:
            cl_compile.append(compile_entry(ms_path, compile_as_c=(ext == ".c")))
        elif ext in [".h", ".hpp", ".hxx"]:
            cl_include.append(include_entry(ms_path))

    for src in defs["java_sources"]:
        p = pathlib.Path(src)
        ms_path = to_msbuild_path(p)
        ext = p.suffix.lower()
        if ext in [".cpp", ".c", ".cc", ".cxx"]:
            cl_compile.append(compile_entry(ms_path, java=True))
        elif ext in [".h", ".hpp", ".hxx"]:
            cl_include.append(include_entry(ms_path, java=True))

    xml_content = f"""<?xml version="1.0" encoding="utf-8"?>
<!-- AUTO-GENERATED FILE. DO NOT EDIT DIRECTLY. -->
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <SharedIncludeDirs>$(SharedIncludeDirs){include_dirs_str}</SharedIncludeDirs>
    <SharedJavaBindingIncludeDirs>$(SharedJavaBindingIncludeDirs){java_include_dirs_str}</SharedJavaBindingIncludeDirs>
  </PropertyGroup>
  <ItemGroup>
{'\n'.join(cl_compile)}
  </ItemGroup>
  <ItemGroup>
{'\n'.join(cl_include)}
  </ItemGroup>
</Project>
"""
    with open(props_path, "w", encoding="utf-8") as f:
        f.write(xml_content)
    print(f"[+] Generated MSBuild props: {props_path}")

def generate_meson_json(defs):
    """Generate sources.json for the Meson build system."""
    json_path = ROOT_DIR / "sources.json"

    def to_relative_path(p):
        return str(pathlib.Path(p).relative_to(ROOT_DIR)).replace('\\', '/')

    meson_data = {
        "include_dirs": [to_relative_path(p) for p in defs["include_dirs"]],
        "sources": [to_relative_path(p) for p in defs["sources"]],
        "java_include_dirs": [to_relative_path(p) for p in defs["java_include_dirs"]],
        "java_sources": [to_relative_path(p) for p in defs["java_sources"]],
    }

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(meson_data, f, indent=4)
    print(f"[+] Generated Meson JSON: {json_path}")

if __name__ == "__main__":

    # 1. Load the shared source definitions.
    defs = get_shared_definitions()

    # 2. Write a config file for each build system.
    generate_cmake_file(defs)
    generate_msbuild_props(defs)
    generate_meson_json(defs)

    print("[+] All build config files generated successfully!")