#include "sfs-api.h"

#include <errno.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <pthread.h>
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
    lua_Integer requested = luaL_optinteger(
        L, 1, (lua_Integer)sizeof name);
    size_t name_space = requested < 0 ? 0 : (size_t)requested;
    if (name_space > sizeof name)
        name_space = sizeof name;
    int idx = 1;

    lua_newtable(L);
    for (;;)
    {
        int status = sfs_list(&cookie, name, name_space);
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

static void set_errno_const(lua_State *L, const char *name, int value)
{
    lua_pushinteger(L, (lua_Integer)-value);
    lua_setfield(L, -2, name);
}

static void register_errno_table(lua_State *L)
{
    lua_newtable(L);
    set_errno_const(L, "EBADF", EBADF);
    set_errno_const(L, "EBUSY", EBUSY);
    set_errno_const(L, "EINVAL", EINVAL);
    set_errno_const(L, "ENOENT", ENOENT);
    set_errno_const(L, "ENAMETOOLONG", ENAMETOOLONG);
    lua_setglobal(L, "errno");
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
        "\n";
    if (luaL_dostring(L, prelude) != LUA_OK)
    {
        fprintf(stderr, "lua-sfs-runner: prelude failed: %s\n",
                lua_tostring(L, -1));
        exit(2);
    }
}

enum lane_value_kind
{
    LANE_NIL,
    LANE_BOOLEAN,
    LANE_INTEGER,
    LANE_NUMBER,
    LANE_STRING,
};

struct lane_value
{
    enum lane_value_kind kind;
    int boolean;
    lua_Integer integer;
    lua_Number number;
    char *string;
    size_t len;
};

struct lane_chunk
{
    char *data;
    size_t len;
};

struct lane_template
{
    char *chunk;
    size_t len;
};

struct lane_job
{
    char *chunk;
    size_t chunk_len;
    int nargs;
    struct lane_value *args;
    pthread_t thread;
    int joined;
    int ok;
    struct lane_value result;
    char *error;
};

struct lane_handle
{
    struct lane_job *job;
};

static void lane_value_clear(struct lane_value *value)
{
    if (value->kind == LANE_STRING)
        free(value->string);
    memset(value, 0, sizeof *value);
    value->kind = LANE_NIL;
}

static void lane_job_free(struct lane_job *job)
{
    if (!job)
        return;
    for (int i = 0; i < job->nargs; i++)
        lane_value_clear(&job->args[i]);
    free(job->args);
    free(job->chunk);
    lane_value_clear(&job->result);
    free(job->error);
    free(job);
}

static int lane_value_from_lua(lua_State *L, int idx, struct lane_value *out)
{
    memset(out, 0, sizeof *out);
    switch (lua_type(L, idx))
    {
    case LUA_TNIL:
        out->kind = LANE_NIL;
        return 0;
    case LUA_TBOOLEAN:
        out->kind = LANE_BOOLEAN;
        out->boolean = lua_toboolean(L, idx);
        return 0;
    case LUA_TNUMBER:
        if (lua_isinteger(L, idx))
        {
            out->kind = LANE_INTEGER;
            out->integer = lua_tointeger(L, idx);
        }
        else
        {
            out->kind = LANE_NUMBER;
            out->number = lua_tonumber(L, idx);
        }
        return 0;
    case LUA_TSTRING:
    {
        size_t len = 0;
        const char *s = lua_tolstring(L, idx, &len);
        out->string = malloc(len ? len : 1);
        if (!out->string)
            return -ENOMEM;
        memcpy(out->string, s, len);
        out->len = len;
        out->kind = LANE_STRING;
        return 0;
    }
    default:
        return -EINVAL;
    }
}

static void lane_value_push(lua_State *L, const struct lane_value *value)
{
    switch (value->kind)
    {
    case LANE_NIL:
        lua_pushnil(L);
        break;
    case LANE_BOOLEAN:
        lua_pushboolean(L, value->boolean);
        break;
    case LANE_INTEGER:
        lua_pushinteger(L, value->integer);
        break;
    case LANE_NUMBER:
        lua_pushnumber(L, value->number);
        break;
    case LANE_STRING:
        lua_pushlstring(L, value->string, value->len);
        break;
    }
}

static void lane_value_push_arg(lua_State *L, const struct lane_value *value)
{
    lane_value_push(L, value);
}

static char *lane_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy)
        memcpy(copy, s, len);
    return copy;
}

static void lane_set_error(struct lane_job *job, const char *message)
{
    free(job->error);
    job->error = lane_strdup(message ? message : "unknown lane error");
    job->ok = 0;
}

static int lane_capture_result(lua_State *L, int idx, struct lane_value *out)
{
    int rc = lane_value_from_lua(L, idx, out);
    if (rc == -EINVAL)
    {
        memset(out, 0, sizeof *out);
        out->kind = LANE_NIL;
        return 0;
    }
    return rc;
}

static void *lane_thread_main(void *arg)
{
    struct lane_job *job = arg;
    lua_State *L = luaL_newstate();
    if (!L)
    {
        lane_set_error(job, "out of memory");
        return NULL;
    }

    luaL_openlibs(L);
    register_disk(L);
    register_errno_table(L);
    install_prelude(L);

    if (luaL_loadbuffer(L, job->chunk, job->chunk_len, "lane") != LUA_OK)
    {
        lane_set_error(job, lua_tostring(L, -1));
        lua_close(L);
        return NULL;
    }

    for (int i = 0; i < job->nargs; i++)
        lane_value_push_arg(L, &job->args[i]);

    if (lua_pcall(L, job->nargs, LUA_MULTRET, 0) != LUA_OK)
    {
        lane_set_error(job, lua_tostring(L, -1));
        lua_close(L);
        return NULL;
    }

    int nresults = lua_gettop(L);
    if (nresults > 0 && lane_capture_result(L, -nresults, &job->result) != 0)
        lane_set_error(job, "unsupported lane result");
    else
        job->ok = 1;

    lua_close(L);
    return NULL;
}

static int lane_dump_writer(lua_State *L, const void *p, size_t size, void *ud)
{
    (void)L;
    struct lane_chunk *chunk = ud;
    char *next = realloc(chunk->data, chunk->len + size);
    if (!next)
        return 1;
    memcpy(next + chunk->len, p, size);
    chunk->data = next;
    chunk->len += size;
    return 0;
}

static int lane_template_gc(lua_State *L)
{
    struct lane_template *tmpl = lua_touserdata(L, 1);
    free(tmpl->chunk);
    tmpl->chunk = NULL;
    tmpl->len = 0;
    return 0;
}

static int lane_handle_join(lua_State *L)
{
    struct lane_handle *handle = luaL_checkudata(L, 1, "sfs_lane_handle");
    struct lane_job *job = handle->job;
    if (!job)
        return luaL_error(L, "lane already joined");

    if (!job->joined)
    {
        pthread_join(job->thread, NULL);
        job->joined = 1;
    }

    if (!job->ok)
    {
        char *error = lane_strdup(job->error ? job->error : "lane failed");
        lane_job_free(job);
        handle->job = NULL;
        lua_pushstring(L, error ? error : "lane failed");
        free(error);
        return lua_error(L);
    }

    lane_value_push(L, &job->result);
    lane_job_free(job);
    handle->job = NULL;
    return 1;
}

static int lane_handle_gc(lua_State *L)
{
    struct lane_handle *handle = luaL_checkudata(L, 1, "sfs_lane_handle");
    if (handle->job)
    {
        if (!handle->job->joined)
            pthread_join(handle->job->thread, NULL);
        lane_job_free(handle->job);
        handle->job = NULL;
    }
    return 0;
}

static int lane_generator(lua_State *L)
{
    struct lane_template *tmpl =
        lua_touserdata(L, lua_upvalueindex(1));

    struct lane_job *job = calloc(1, sizeof *job);
    if (!job)
        return luaL_error(L, "out of memory");

    job->chunk = malloc(tmpl->len ? tmpl->len : 1);
    if (!job->chunk)
    {
        lane_job_free(job);
        return luaL_error(L, "out of memory");
    }
    memcpy(job->chunk, tmpl->chunk, tmpl->len);
    job->chunk_len = tmpl->len;

    job->nargs = lua_gettop(L);
    if (job->nargs > 0)
    {
        job->args = calloc((size_t)job->nargs, sizeof job->args[0]);
        if (!job->args)
        {
            lane_job_free(job);
            return luaL_error(L, "out of memory");
        }
    }

    for (int i = 0; i < job->nargs; i++)
    {
        int rc = lane_value_from_lua(L, i + 1, &job->args[i]);
        if (rc == -ENOMEM)
        {
            lane_job_free(job);
            return luaL_error(L, "out of memory");
        }
        if (rc != 0)
        {
            lane_job_free(job);
            return luaL_error(L, "unsupported lane argument type");
        }
    }

    int rc = pthread_create(&job->thread, NULL, lane_thread_main, job);
    if (rc != 0)
    {
        lane_job_free(job);
        return luaL_error(L, "pthread_create failed");
    }

    struct lane_handle *handle = lua_newuserdata(L, sizeof *handle);
    handle->job = job;
    luaL_getmetatable(L, "sfs_lane_handle");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_lanes_gen(lua_State *L)
{
    int fn = lua_gettop(L);
    luaL_checktype(L, fn, LUA_TFUNCTION);

    struct lane_chunk chunk = {0};
    if (lua_dump(L, lane_dump_writer, &chunk, 0) != 0)
    {
        free(chunk.data);
        return luaL_error(L, "could not dump lane function");
    }

    struct lane_template *tmpl = lua_newuserdata(L, sizeof *tmpl);
    tmpl->chunk = chunk.data;
    tmpl->len = chunk.len;
    luaL_getmetatable(L, "sfs_lane_template");
    lua_setmetatable(L, -2);

    lua_pushcclosure(L, lane_generator, 1);
    return 1;
}

static void register_lanes(lua_State *L)
{
    if (luaL_newmetatable(L, "sfs_lane_template"))
    {
        lua_pushcfunction(L, lane_template_gc);
        lua_setfield(L, -2, "__gc");
    }
    lua_pop(L, 1);

    if (luaL_newmetatable(L, "sfs_lane_handle"))
    {
        lua_pushcfunction(L, lane_handle_gc);
        lua_setfield(L, -2, "__gc");
        lua_newtable(L);
        lua_pushcfunction(L, lane_handle_join);
        lua_setfield(L, -2, "join");
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushcfunction(L, l_lanes_gen);
    lua_setfield(L, -2, "gen");
    lua_setglobal(L, "lanes");
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
    register_errno_table(L);
    install_prelude(L);
    register_lanes(L);

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
