
premake.modules.lua = {}
local m = premake.modules.lua

local p = premake

newaction {
    trigger = "gt",
    description = "Settin up greentea project",
    prjDir = "",
    prjName = "",

    onWorkspace = function(wks)
        prjDir = wks.location
    end,

    onProject = function(prj)
        prjName = prj.name
    end,


    execute = function()

        -- Creating file need for building the dll correctly
        file = io.open(prjDir .. "/" .. prjName .. "/Exports.h", "a")
        io.output(file)
        io.close()

        file = io.open(prjDir .. "/" .. prjName .. "/Exports.cpp", "a")
        io.output(file)
        io.close()
        
        os.mkdir(prjDir .. "/Assets/Scripts")
        
        os.mkdir(prjDir .. "/.gt")
        res, error = os.copyfile(GreenTeaDir .. "/resources/dummstring.h", prjDir .. "/.gt/dummstring.h")
        if not res then
            print(error)
        end
        os.execute("attrib +h " .. prjDir .. "/.gt")

        src = prjDir .. "/" .. prjName
        os.mkdir(src)
        res, error = os.copyfile(GreenTeaDir .. "/resources/cpp.hint", src .. "/cpp.hint")
        if not res then
            print(error)
        end

        file = io.open(prjDir .. "/.gt/compile_commands.json", "w")
        io.output(file)
        io.write("[\n\t{\n\t\t\"directory\": \"" .. gtrDir .. "\",\n\t\t\"arguments\":\n\t\t[\n\t\t\t\"clang++\",\n")
        io.write("\t\t\t\"-DGAME_DLL\",\n")
        io.write("\t\t\t\"-DREFLECTION\",\n")
        for key, val in pairs(IncludeDirs) do
            io.write("\t\t\t\"-I" .. val .."\",\n")
        end
        io.write("\t\t\t\"-Wno-user-defined-literals\",\n")
        io.write("\t\t\t\"-Wno-unknown-attributes\",\n")
        io.write("\t\t\t\"-Wno-nonportable-include-path\",\n")
        io.write("\t\t\t\"-std=c++17\",\n\t\t\t\"clangdump.hpp\"\n\t\t],\n\t\t\"file\": \"clangdump.hpp\"\n\t}\n]")
        io.close(file)
    end,
}

return m