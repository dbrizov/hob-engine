#pragma once

#include <filesystem>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <sol/sol.hpp>

namespace hob {
    // ---------------------------------------------------------------------
    // LuaTypeName trait — maps C++ types to Lua annotation type names.
    // Specialize via HOB_LUA_TYPE for every usertype/enum exposed to Lua.
    // ---------------------------------------------------------------------

    template<typename T>
    struct LuaTypeName {
        static constexpr const char* value = nullptr;
    };

#define HOB_LUA_TYPE(CppType, LuaName)                                      \
    template <>                                                             \
    struct LuaTypeName<CppType> {                                           \
        static constexpr const char* value = LuaName;                       \
    };

    // clang-format off
    HOB_LUA_TYPE(void, "")
    HOB_LUA_TYPE(bool, "boolean")
    HOB_LUA_TYPE(short, "integer")
    HOB_LUA_TYPE(unsigned short, "integer")
    HOB_LUA_TYPE(int, "integer")
    HOB_LUA_TYPE(unsigned int, "integer")
    HOB_LUA_TYPE(long, "integer")
    HOB_LUA_TYPE(unsigned long, "integer")
    HOB_LUA_TYPE(long long, "integer")
    HOB_LUA_TYPE(unsigned long long, "integer")
    HOB_LUA_TYPE(float, "number")
    HOB_LUA_TYPE(double, "number")
    HOB_LUA_TYPE(std::string, "string")
    HOB_LUA_TYPE(const char*, "string")
    // clang-format on

    namespace meta_detail {
        template<typename T>
        using strip_t = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<std::remove_cv_t<T>>>>;

        template<typename T>
        const char* lua_name() {
            constexpr const char* n = LuaTypeName<strip_t<T>>::value;
            return n ? n : "any";
        }

        // Function traits — extracts return type and arg pack from member fn ptrs,
        // const member fn ptrs, and plain function pointers.
        template<typename F>
        struct fn_traits;

        template<typename R, typename... A>
        struct fn_traits<R(*)(A...)> {
            using ret = R;
            using args = std::tuple<A...>;
            static constexpr bool is_member = false;
        };

        template<typename R, typename C, typename... A>
        struct fn_traits<R(C::*)(A...)> {
            using ret = R;
            using args = std::tuple<A...>;
            static constexpr bool is_member = true;
        };

        template<typename R, typename C, typename... A>
        struct fn_traits<R(C::*)(A...) const> {
            using ret = R;
            using args = std::tuple<A...>;
            static constexpr bool is_member = true;
        };

        template<typename Tuple, std::size_t... I>
        void fill_arg_names(std::vector<std::string>& out, std::index_sequence<I...>) {
            (out.emplace_back(lua_name<std::tuple_element_t<I, Tuple>>()), ...);
        }

        template<typename Tuple>
        std::vector<std::string> arg_names() {
            std::vector<std::string> out;
            fill_arg_names<Tuple>(out, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
            return out;
        }
    }

    // ---------------------------------------------------------------------
    // Registry data model.
    // ---------------------------------------------------------------------

    struct LuaMethodInfo {
        std::string name;
        std::string ret;
        std::vector<std::string> args; // Lua type names, one per arg
        std::vector<std::string> arg_names; // optional, parallel to args; empty -> emit arg1, arg2, ...
        bool is_static = false;
    };

    struct LuaFieldInfo {
        std::string name;
        std::string type;
    };

    struct LuaOperatorInfo {
        std::string op; // Operator name: add, sub, unm, mul, div
        std::string rhs; // empty for unary
        std::string ret;
    };

    struct LuaCtorInfo {
        std::vector<std::string> args;
    };

    struct LuaUsertypeInfo {
        std::string name;
        std::string base; // optional base class for ---@class inheritance
        std::vector<LuaFieldInfo> fields;
        std::vector<LuaMethodInfo> methods;
        std::vector<LuaOperatorInfo> operators;
        std::vector<LuaCtorInfo> ctors;
    };

    struct LuaEnumInfo {
        std::string name;
        std::vector<std::string> values;
    };

    struct LuaTableInfo {
        std::string name;
        std::vector<LuaFieldInfo> fields; // constants
    };

    class LuaMetaRegistry {
        std::vector<LuaUsertypeInfo> m_usertypes;
        std::vector<LuaEnumInfo> m_enums;
        std::vector<LuaTableInfo> m_tables;

    public:
        LuaUsertypeInfo& add_usertype(std::string name, std::string base = {}) {
            LuaUsertypeInfo info;
            info.name = std::move(name);
            info.base = std::move(base);
            m_usertypes.push_back(std::move(info));
            return m_usertypes.back();
        }

        LuaEnumInfo& add_enum(std::string name) {
            m_enums.push_back({std::move(name), {}});
            return m_enums.back();
        }

        LuaTableInfo& add_table(std::string name) {
            m_tables.push_back({std::move(name), {}});
            return m_tables.back();
        }

        bool write_to_file(const std::filesystem::path& path) const;
    };

    // ---------------------------------------------------------------------
    // UsertypeBuilder — thin wrapper around sol::usertype<T> that also
    // records meta for the annotation generator.
    // ---------------------------------------------------------------------

    template<typename T>
    class UsertypeBuilder {
        sol::usertype<T> m_usertype;
        LuaUsertypeInfo* m_info;

    public:
        UsertypeBuilder(sol::state& lua, LuaMetaRegistry& reg, const char* name, std::string base = {})
            : m_usertype(lua.new_usertype<T>(name))
            , m_info(&reg.add_usertype(name, std::move(base))) {
        }

        // ----- Constructors. Pass each ctor as sol::types<Args...>. -----
        template<typename... Sigs>
        UsertypeBuilder& ctors() {
            m_usertype[sol::call_constructor] = sol::factories(make_factory(Sigs{})...);
            (record_ctor(Sigs{}), ...);
            return *this;
        }

        UsertypeBuilder& no_ctor() {
            m_usertype[sol::meta_function::construct] = sol::no_constructor;
            return *this;
        }

        // ----- Fields (public member data) -----
        template<typename M, typename C>
        UsertypeBuilder& field(const char* name, M C::* ptr) {
            m_usertype[name] = ptr;
            m_info->fields.push_back({name, meta_detail::lua_name<M>()});
            return *this;
        }

        // ----- Methods. Member fn -> :method(), free/static fn -> .method() -----
        // Optional `arg_names` provides parameter names for the generated
        // annotations; if empty, falls back to arg1, arg2, ...
        template<typename F>
        UsertypeBuilder& method(const char* name, F fn, std::initializer_list<const char*> arg_names = {}) {
            m_usertype[name] = fn;
            record_method(name, fn, arg_names);
            return *this;
        }

        // Explicit signature override for cases the trait can't deduce
        // (lambdas, sol::variadic_args, overloads). The signature uses the
        // Annotation tail-shape: "(arg1: type, arg2: type): ret"
        template<typename F>
        UsertypeBuilder& method_sig(const char* name, F fn, const char* sig, bool is_static = false) {
            m_usertype[name] = fn;
            LuaMethodInfo info;
            info.name = name;
            info.is_static = is_static;
            info.ret = sig; // emitter recognizes leading '('
            m_info->methods.push_back(std::move(info));
            return *this;
        }

        // ----- Operators that emit ---@operator lines -----
        template<typename F> UsertypeBuilder& op_add(F fn) {
            return binary_op(sol::meta_function::addition, "add", fn);
        }

        template<typename F> UsertypeBuilder& op_sub(F fn) {
            return binary_op(sol::meta_function::subtraction, "sub", fn);
        }

        template<typename F> UsertypeBuilder& op_mul(F fn) {
            return binary_op(sol::meta_function::multiplication, "mul", fn);
        }

        template<typename F> UsertypeBuilder& op_div(F fn) {
            return binary_op(sol::meta_function::division, "div", fn);
        }

        template<typename F> UsertypeBuilder& op_unm(F fn) {
            return unary_op(sol::meta_function::unary_minus, "unm", fn);
        }

        // Metamethods that have no ---@operator counterpart.
        template<typename F> UsertypeBuilder& op_eq(F fn) {
            m_usertype[sol::meta_function::equal_to] = fn;
            return *this;
        }

        template<typename F> UsertypeBuilder& op_tostring(F fn) {
            m_usertype[sol::meta_function::to_string] = fn;
            return *this;
        }

        // Direct sol2 access for things outside this wrapper's API.
        sol::usertype<T>& sol() { return m_usertype; }

    private:
        template<typename F>
        UsertypeBuilder& binary_op(sol::meta_function mf, const char* op_name, F fn) {
            m_usertype[mf] = fn;
            using traits = meta_detail::fn_traits<F>;
            using args_t = typename traits::args;
            static_assert(std::tuple_size_v<args_t> >= 1, "binary_op expects at least one rhs arg");
            // Member operator+ has args=(rhs); free op+(lhs, rhs) has args=(lhs, rhs).
            constexpr std::size_t rhs_idx = std::tuple_size_v<args_t> - 1;
            LuaOperatorInfo info;
            info.op = op_name;
            info.rhs = meta_detail::lua_name<std::tuple_element_t<rhs_idx, args_t>>();
            info.ret = meta_detail::lua_name<typename traits::ret>();
            m_info->operators.push_back(std::move(info));
            return *this;
        }

        template<typename F>
        UsertypeBuilder& unary_op(sol::meta_function mf, const char* op_name, F fn) {
            m_usertype[mf] = fn;
            using traits = meta_detail::fn_traits<F>;
            LuaOperatorInfo info;
            info.op = op_name;
            info.ret = meta_detail::lua_name<typename traits::ret>();
            m_info->operators.push_back(std::move(info));
            return *this;
        }

        template<typename... A>
        static auto make_factory(sol::types<A...>) {
            return [](A... a) { return T(a...); };
        }

        template<typename... A>
        void record_ctor(sol::types<A...>) {
            LuaCtorInfo c;
            (c.args.emplace_back(meta_detail::lua_name<A>()), ...);
            m_info->ctors.push_back(std::move(c));
        }

        template<typename F>
        void record_method(const char* name, F /*fn*/, std::initializer_list<const char*> arg_names) {
            using traits = meta_detail::fn_traits<F>;
            LuaMethodInfo info;
            info.name = name;
            info.is_static = !traits::is_member;
            info.ret = meta_detail::lua_name<typename traits::ret>();
            info.args = meta_detail::arg_names<typename traits::args>();
            for (const char* n : arg_names) {
                info.arg_names.emplace_back(n);
            }
            m_info->methods.push_back(std::move(info));
        }
    };

    template<typename T>
    UsertypeBuilder<T> bind_usertype(sol::state& lua, LuaMetaRegistry& reg, const char* name, std::string base = {}) {
        return UsertypeBuilder<T>(lua, reg, name, std::move(base));
    }

    // ---------------------------------------------------------------------
    // TableBuilder — wraps a global table of constants.
    // ---------------------------------------------------------------------

    class TableBuilder {
        sol::table m_table;
        LuaTableInfo* m_info;

    public:
        TableBuilder(sol::state& lua, LuaMetaRegistry& reg, const char* name)
            : m_table(lua.create_named_table(name))
            , m_info(&reg.add_table(name)) {
        }

        template<typename V>
        TableBuilder& constant(const char* name, V value) {
            m_table[name] = value;
            m_info->fields.push_back({name, meta_detail::lua_name<V>()});
            return *this;
        }
    };

    inline TableBuilder bind_table(sol::state& lua, LuaMetaRegistry& reg, const char* name) {
        return TableBuilder(lua, reg, name);
    }
}
