#ifndef SYSTEM_BUILD
#include "../gcc/gcc-plugin.h"
#include "../gcc/c-family/c-common.h"
#include "../gcc/cp/cp-objcp-common.h"
#include "../gcc/cp/cp-tree.h"
#include "../gcc/langhooks.h"
#include "../gcc/langhooks-def.h"
#include "../gcc/system.h"
#include "../gcc/coretypes.h"
#include "../gcc/tree.h"
#include "../gcc/tree-pass.h"
#include "../gcc/intl.h"
#include "../gcc/toplev.h"
#include "../gcc/diagnostic.h"
#include "../gcc/context.h"
#include "../gcc/print-tree.h"
#include "../gcc/stringpool.h"
#include "../gcc/gimplify.h"
#include "../gcc/gimple-exp.h"
#include "../gcc/convert.h"
#include "../gcc/cgraph.h"
#include "../gcc/coretypes.h"
#include "../gcc/attribs.h"
#else
#include <gcc-plugin.h>
#include "config.h"
#include "system.h"
#include <cp/cp-tree.h>
#include <langhooks.h>
#include <plugin-version.h>
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "toplev.h"
#include "diagnostic.h"
#include "context.h"
#include "print-tree.h"
#include "stringpool.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "cgraph.h"
#include "coretypes.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
#include "tree-iterator.h"
#include "input.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "cgraph.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "fold-const.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "debug.h"
#include "attribs.h"
#endif

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT
int plugin_is_GPL_compatible = 1;

//

const char *fnname = "__builtin_sw_number_of_fields";
decltype(targetm.fold_builtin) old_fold_builtin;

int field_count(tree decl) {
    auto err = []() {
        error("cannot count number of fields for type");
        exit(1);
    };
	if (decl == NULL_TREE || decl == error_mark_node) {
        debug_tree(decl);
        err();
    }
    if (TREE_CODE(decl) == REFERENCE_TYPE) {
        decl = TREE_TYPE(decl);
    }
    if (TREE_CODE(decl) != RECORD_TYPE) {
        err();
    }
    int sz = 0;
    for (auto fields = TYPE_FIELDS(decl); fields; fields = TREE_CHAIN(fields)) {
        switch (auto code = TREE_CODE(fields)) {
        case FIELD_DECL:
            ++sz;
            break;
        default:
            break;
        }
    }
    return sz;
}

tree builtin_function_handler(tree fndecl, int n_args, tree *argp, bool ignore) {
    if (strcmp(IDENTIFIER_POINTER(DECL_NAME(fndecl)), fnname) != 0 || ignore)
        return old_fold_builtin(fndecl, n_args, argp, ignore);

    auto f = current_function_decl;
    if (TREE_CODE(f) == TEMPLATE_DECL || !DECL_TEMPLATE_INFO(f) || !DECL_USE_TEMPLATE(f)) {
        return NULL_TREE;
    }
    auto arg0 = TREE_VEC_ELT(DECL_TI_ARGS(f), 0);
    if (!arg0) {
        error("template arg0 does not exist");
        exit(1);
    }
    auto cnt = field_count(arg0);
    return build_int_cst(integer_type_node, cnt);
}

void start_unit(void *, void *) {
    old_fold_builtin = targetm.fold_builtin;
    targetm.fold_builtin = builtin_function_handler;

    auto args = new tree[1];
    // const void *
    args[0] = build_pointer_type(build_qualified_type(void_type_node, TYPE_QUAL_CONST));
    tree ftype = build_function_type_array(integer_type_node, 1, args);
    delete[] args;
    auto fndecl = add_builtin_function(fnname, ftype, 0, BUILT_IN_MD, 0, 0);
    DECL_DECLARED_CONSTEXPR_P(fndecl) = 1;
    SET_DECL_IMMEDIATE_FUNCTION_P(fndecl);
    set_call_expr_flags(fndecl, ECF_CONST | ECF_NOVOPS | ECF_NOTHROW | ECF_LEAF);
}

#include <thread>
#include <chrono>

EXPORT
int plugin_init(plugin_name_args *info, plugin_gcc_version *version) {
    if (!plugin_default_version_check(version, &gcc_version))
        return 1;
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    register_callback(info->base_name, PLUGIN_START_UNIT, start_unit, 0);
    return 0;
}
