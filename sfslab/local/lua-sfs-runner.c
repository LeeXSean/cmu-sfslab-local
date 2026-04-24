#include "sfs-api.h"

#include <errno.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int push_status(lua_State *L, int status)
{
    lua_pushinteger(L, (lua_Integer)status);
    return 1;
}

static int l_disk_format(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    size_t size = (size_t)luaL_checkinteger(L, 2);
    return push_status(L, sfs_format(name, size));
}

static int l_disk_mount(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    return push_status(L, sfs_mount(name));
}

static int l_disk_unmount(lua_State *L)
{
    return push_status(L, sfs_unmount());
}

static int l_disk_open(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    return push_status(L, sfs_open(name));
}

static int l_disk_close(lua_State *L)
{
    int fd = (int)luaL_checkinteger(L, 1);
    sfs_close(fd);
    return push_status(L, 0);
}

static int l_disk_write(lua_State *L)
{
    int fd = (int)luaL_checkinteger(L, 1);
    size_t len = 0;
    const char *buf = luaL_checklstring(L, 2, &len);
    lua_pushinteger(L, (lua_Integer)sfs_write(fd, buf, len));
    return 1;
}

static int l_disk_read(lua_State *L)
{
    int fd = (int)luaL_checkinteger(L, 1);
    size_t len = (size_t)luaL_checkinteger(L, 2);
    char *buf = malloc(len ? len : 1);
    if (!buf)
        return push_status(L, -ENOMEM);

    ssize_t nread = sfs_read(fd, buf, len);
    if (nread < 0)
    {
        free(buf);
        lua_pushinteger(L, (lua_Integer)nread);
        return 1;
    }

    lua_pushlstring(L, buf, (size_t)nread);
    free(buf);
    return 1;
}

static int l_disk_getpos(lua_State *L)
{
    int fd = (int)luaL_checkinteger(L, 1);
    lua_pushinteger(L, (lua_Integer)sfs_getpos(fd));
    return 1;
}

static int l_disk_seek(lua_State *L)
{
    int fd = (int)luaL_checkinteger(L, 1);
    ssize_t delta = (ssize_t)luaL_checkinteger(L, 2);
    lua_pushinteger(L, (lua_Integer)sfs_seek(fd, delta));
    return 1;
}

static int l_disk_rename(lua_State *L)
{
    const char *old_name = luaL_checkstring(L, 1);
    const char *new_name = luaL_checkstring(L, 2);
    return push_status(L, sfs_rename(old_name, new_name));
}

static int l_disk_remove(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    return push_status(L, sfs_remove(name));
}

static int l_list_iter(lua_State *L)
{
    int idx = (int)lua_tointeger(L, lua_upvalueindex(2)) + 1;
    lua_pushinteger(L, (lua_Integer)idx);
    lua_replace(L, lua_upvalueindex(2));

    lua_rawgeti(L, lua_upvalueindex(1), idx);
    if (lua_isnil(L, -1))
        return 0;
    return 1;
}

static int l_disk_list(lua_State *L)
{
    sfs_list_cookie cookie = NULL;
    char name[SFS_FILE_NAME_SIZE_LIMIT];
    int idx = 1;

    lua_newtable(L);
    for (;;)
    {
        int status = sfs_list(&cookie, name, sizeof name);
        if (status == 1)
            break;
        if (status < 0)
        {
            lua_pop(L, 1);
            lua_pushinteger(L, status);
            return 1;
        }
        lua_pushstring(L, name);
        lua_rawseti(L, -2, (lua_Integer)idx++);
    }

    lua_pushinteger(L, 0);
    lua_pushcclosure(L, l_list_iter, 2);
    return 1;
}

static void register_disk(lua_State *L)
{
    static const luaL_Reg funcs[] = {
        {"format", l_disk_format},
        {"mount", l_disk_mount},
        {"unmount", l_disk_unmount},
        {"open", l_disk_open},
        {"close", l_disk_close},
        {"write", l_disk_write},
        {"read", l_disk_read},
        {"getPos", l_disk_getpos},
        {"seek", l_disk_seek},
        {"rename", l_disk_rename},
        {"remove", l_disk_remove},
        {"list", l_disk_list},
        {NULL, NULL},
    };
    luaL_newlib(L, funcs);
    lua_setglobal(L, "disk");
}

static void install_prelude(lua_State *L)
{
    const char *prelude =
        "TRACE_DISK = TRACE_DISK or 'trace.img'\n"
        "TRACE_DISK_SIZE = TRACE_DISK_SIZE or 1048576\n"
        "function check(v, ...)\n"
        "  if type(v) == 'number' and v < 0 then\n"
        "    error('SFS error '..tostring(v), 2)\n"
        "  end\n"
        "  return v, ...\n"
        "end\n"
        "lanes = lanes or {}\n"
        "function lanes.gen(_, fn)\n"
        "  return function(...)\n"
        "    local args = {...}\n"
        "    local ok, result = pcall(function() return fn(table.unpack(args)) end)\n"
        "    return { join = function()\n"
        "      if not ok then error(result, 2) end\n"
        "      return result\n"
        "    end }\n"
        "  end\n"
        "end\n";
    if (luaL_dostring(L, prelude) != LUA_OK)
    {
        fprintf(stderr, "lua-sfs-runner: prelude failed: %s\n",
                lua_tostring(L, -1));
        exit(2);
    }
}

static int run_trace(const char *path)
{
    lua_State *L = luaL_newstate();
    if (!L)
    {
        fprintf(stderr, "lua-sfs-runner: out of memory\n");
        return 2;
    }

    luaL_openlibs(L);
    register_disk(L);
    install_prelude(L);

    int rc = luaL_dofile(L, path);
    if (rc != LUA_OK)
    {
        fprintf(stderr, "%s: FAIL: %s\n", path, lua_tostring(L, -1));
        lua_close(L);
        sfs_unmount();
        unlink("trace.img");
        return 1;
    }

    lua_close(L);
    sfs_unmount();
    unlink("trace.img");
    printf("%s: PASS\n", path);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s TRACE.lua [...]\n", argv[0]);
        return 2;
    }

    int failed = 0;
    int passed = 0;
    int total = 0;
    for (int i = 1; i < argc; i++)
    {
        total++;
        if (run_trace(argv[i]) == 0)
            passed++;
        else
            failed = 1;
    }

    if (argc > 2)
        printf("lua-sfs-runner: %d/%d traces passed\n", passed, total);
    return failed;
}
