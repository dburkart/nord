/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "compile.h"
#include "parse.h"
#include "symbol.h"
#include "machine/memory.h"
#include "machine/value.h"
#include "util/error.h"

typedef struct
{
    const char *name;
    const char *listing;
    symbol_map_t *symbols;
    binary_t *binary;
    uint8_t rp;
    uint64_t mp;
} compile_context_t;

static inline instruction_t make_zero_instr(uint8_t opcode)
{
    return (instruction_t){ opcode, .fields={ .pair={ 0, 0 } } };
}

static inline instruction_t make_single_instr(uint8_t opcode, uint8_t arg1)
{
    return (instruction_t){ opcode, .fields={ .pair={ arg1, 0 } } };
}

static inline instruction_t make_singlew_instr(uint8_t opcode, uint16_t arg1)
{
    return (instruction_t){ opcode, .fields={ .pair={ 0, arg1 } } };
}

static inline instruction_t make_pair_instr(uint8_t opcode, uint8_t arg1, uint16_t arg2)
{
    return (instruction_t){ opcode, .fields={ .pair={ arg1, arg2 } } };
}

static inline instruction_t make_triplet_instr(uint8_t opcode, uint8_t arg1, uint8_t arg2, uint8_t arg3)
{
    return (instruction_t){ opcode, .fields={ .triplet={ arg1, arg2, arg3 } } };
}


compile_context_t *context_create(const char *name, const char *listing)
{
    compile_context_t *context = malloc(sizeof(compile_context_t));

    context->name = name;
    context->listing = listing;
    context->symbols = symbol_map_create();
    context->binary = binary_create();
    context->binary->data = memory_create(1);
    context->binary->code = code_block_create();
    context->rp = 1;

    // Set up true and false
    memory_set(context->binary->data, 0, (value_t){VAL_BOOLEAN, false});
    memory_set(context->binary->data, 1, (value_t){VAL_BOOLEAN, true});

    context->mp = 2;

    return context;
}

void context_destroy(compile_context_t *context)
{
    symbol_map_destroy(context->symbols);
    // NOTE: We don't free the code block, since that is returned (and this is
    //       an internal data structure).
    free(context);
}

void compile_comparison(compile_context_t *context, uint8_t reg, uint8_t opcode, uint8_t condition, uint8_t left, uint8_t right)
{
    instruction_t instruction;

    // First, write the false case
    instruction = make_pair_instr(OP_LOAD, reg, 0);
    code_block_write(context->binary->code, instruction);

    // Now, write out the comparison instruction
    instruction = make_triplet_instr(opcode, condition, left, right);
    code_block_write(context->binary->code, instruction);

    // Finally, write out the true case
    instruction = make_pair_instr(OP_LOAD, reg, 1);
    code_block_write(context->binary->code, instruction);
}

uint8_t compile_builtin_fn_call(compile_context_t *context, char *name, uint8_t rp_reset, uint8_t nargs, uint8_t *args)
{
    instruction_t instruction;
    symbol_t sym = symbol_map_get(context->symbols, name);

    // Undefined symbols for builtin calls simply haven't been called yet.
    // TODO: validate builtin name here
    if (sym.location.type == LOC_UNDEF)
    {
        memory_set(context->binary->data, context->mp, string_create(name));
        sym.location.type = LOC_BUILTIN;
        sym.location.address = context->mp++;
        sym.name = name;
        sym.type = SYM_FN;

        symbol_map_t *map = context->symbols;
        while (map->parent != NULL)
            map = map->parent;
        symbol_map_set(map, sym);
    }

    // We push args onto the stack in reverse order because we'll pop them
    // off in the builtin
    for (int i = nargs - 1; i >= 0; i--)
    {
        instruction = make_single_instr(OP_PUSH, args[i]);
        code_block_write(context->binary->code, instruction);
    }

    context->rp = rp_reset;

    // Number of args -> $0
    instruction = make_pair_instr(OP_LOADV, 0, nargs);
    code_block_write(context->binary->code, instruction);

    instruction = make_singlew_instr(OP_CALL_DYNAMIC, sym.location.address);
    code_block_write(context->binary->code, instruction);

    instruction = make_single_instr(OP_POP, context->rp);
    code_block_write(context->binary->code, instruction);

    return context->rp;
}

uint8_t compile_internal(ast_t *ast, compile_context_t *context)
{
    uint8_t result = 0, left, right, var, nil, *regs;
    symbol_t sym;
    instruction_t instruction;
    value_t val;
    int tmp, addr, begin, end;
    ast_t *args;

    switch (ast->type)
    {
        case AST_UNARY:
            right = compile_internal(ast->op.unary.operand, context);

            switch (ast->op.unary.operator.type)
            {
                case TOK_MINUS:
                    instruction = make_pair_instr(OP_NEGATE, context->rp, right);
                    break;
                case TOK_BANG:
                    instruction = make_pair_instr(OP_NOT, context->rp, right);
                    break;
                case TOK_RETURN:
                    instruction = make_single_instr(OP_RETURN, right);
                    break;
                default:
                    ;
            }

            code_block_write(context->binary->code, instruction);
            result = context->rp;
            break;
        case AST_BINARY:
            left = compile_internal(ast->op.binary.left, context);
            context->rp += 1;
            right = compile_internal(ast->op.binary.right, context);
            context->rp -= 1;

            switch (ast->op.binary.operator.type)
            {
                // -- Arithmetic
                case TOK_PLUS:
                    instruction = make_triplet_instr(OP_ADD, context->rp, left, right);
                    break;
                case TOK_MINUS:
                    instruction = make_triplet_instr(OP_SUBTRACT, context->rp, left, right);
                    break;
                case TOK_ASTERISK:
                    instruction = make_triplet_instr(OP_MULTIPLY, context->rp, left, right);
                    break;
                case TOK_SLASH:
                    instruction = make_triplet_instr(OP_DIVIDE, context->rp, left, right);
                    break;

                // -- Logic
                case TOK_AND:
                    instruction = make_triplet_instr(OP_AND, context->rp, left, right);
                    break;
                case TOK_OR:
                    instruction = make_triplet_instr(OP_OR, context->rp, left, right);
                    break;

                case TOK_EQUAL_EQUAL:
                    compile_comparison(context, context->rp + 2, OP_EQUAL, 1, left, right);
                    // Because we don't have proper register allocation, we need an extra instruction
                    // to move the result to a compact register
                    instruction = make_pair_instr(OP_MOVE, context->rp, context->rp + 2);
                    break;
                case TOK_BANG_EQUAL:
                    compile_comparison(context, context->rp + 2, OP_EQUAL, 0, left, right);
                    // Because we don't have proper register allocation, we need an extra instruction
                    // to move the result to a compact register
                    instruction = make_pair_instr(OP_MOVE, context->rp, context->rp + 2);
                    break;
                case TOK_LESS:
                    compile_comparison(context, context->rp + 2, OP_LESSTHAN, 1, left, right);
                    // Because we don't have proper register allocation, we need an extra instruction
                    // to move the result to a compact register
                    instruction = make_pair_instr(OP_MOVE, context->rp, context->rp + 2);
                    break;
                case TOK_LESS_EQUAL:
                    compile_comparison(context, context->rp + 2, OP_LESSTHAN, 1, left, right);
                    compile_comparison(context, context->rp + 3, OP_EQUAL, 1, left, right);
                    // Because we don't have proper register allocation, we need an extra instruction
                    // to move the result to a compact register
                    instruction = make_triplet_instr(OP_OR, context->rp, context->rp + 2, context->rp + 3);
                    break;
                case TOK_GREATER:
                    compile_comparison(context, context->rp + 2, OP_LESSTHAN, 0, left, right);
                    // Because we don't have proper register allocation, we need an extra instruction
                    // to move the result to a compact register
                    instruction = make_pair_instr(OP_MOVE, context->rp, context->rp + 2);
                    break;
                case TOK_GREATER_EQUAL:
                    compile_comparison(context, context->rp + 2, OP_LESSTHAN, 0, left, right);
                    compile_comparison(context, context->rp + 3, OP_EQUAL, 1, left, right);
                    // Because we don't have proper register allocation, we need an extra instruction
                    // to move the result to a compact register
                    instruction = make_triplet_instr(OP_OR, context->rp, context->rp + 2, context->rp + 3);
                    break;
                default:
                    ;
            }

            code_block_write(context->binary->code, instruction);

            result = context->rp;
            break;

        case AST_DECLARE:
            // Handle declaration with no assignment
            if (ast->op.declare.initial_value == NULL)
            {
                sym.location.type = LOC_NONE;
            }
            else
            {
                result = compile_internal(ast->op.declare.initial_value, context);

                if (ast->op.declare.initial_value->type == AST_FUNCTION_DECL)
                {
                    sym = symbol_map_get(context->symbols, "__anonymous");
                    value_t fn = memory_get(context->binary->data, sym.location.address);
                    sym.location.type = LOC_MEMORY;
                    sym.location.address = context->mp++;
                    memory_set(context->binary->data, sym.location.address, fn);
                }
                else if (result < context->rp)
                {
                    instruction = make_pair_instr(OP_MOVE, context->rp, result);
                    code_block_write(context->binary->code, instruction);
                    sym.location.address = context->rp;
                    sym.location.type = LOC_REGISTER;
                    context->rp += 1;
                }
                else
                {
                    sym.location.address = result;
                    sym.location.type = LOC_REGISTER;
                    context->rp += 1;
                }
            }

            sym.name = ast->op.declare.name;
            sym.type = (ast->op.declare.var_type.type == TOK_VAR) ? SYM_VAR : SYM_CONSTANT;
            symbol_map_set(context->symbols, sym);
            break;

        case AST_ASSIGN:
            result = compile_internal(ast->op.assign.value, context);
            // Right now, since we assign registers in a stack-like manner, we
            // use a move instruction for assignment so that register usage
            // stays compact. If we used a different method for register
            // assignment, a more efficient (at runtime) approach would be to
            // simply change the location of the variable in the symbol map.
            //
            // TODO: Handle variables in memory
            sym = symbol_map_get(context->symbols, ast->op.assign.name);

            if (sym.location.type == LOC_UNDEF)
            {
                char *error;
                location_t loc = {ast->location.start, ast->location.end};
                asprintf(&error, "Use of undeclared identifier \"%s\"", ast->op.assign.name);
                printf("%s", format_error(context->name, context->listing, error, loc));
                exit(1);
            }

            if (sym.type == SYM_CONSTANT)
            {
                char *error;
                location_t loc = {ast->location.start, ast->location.end};
                asprintf(&error, "Cannot assign to constant \"%s\", value is immutable", ast->op.assign.name);
                printf("%s", format_error(context->name, context->listing, error, loc));
                exit(1);
            }

            if (ast->op.assign.value->type == AST_FUNCTION_DECL)
            {
                sym = symbol_map_get(context->symbols, "__anonymous");
                value_t fn = memory_get(context->binary->data, sym.location.address);
                sym.location.address = context->mp++;
                sym.name = ast->op.assign.name;
                sym.type = SYM_VAR;
                symbol_map_set(context->symbols, sym);
                memory_set(context->binary->data, sym.location.address, fn);
            }
            else
            {
                instruction = make_pair_instr(OP_MOVE, sym.location.address, result);
                code_block_write(context->binary->code, instruction);
            }

            break;

        case AST_LITERAL:
            // TODO: Support ints which are larger than 16-bit
            if (ast->op.literal.token.type == TOK_NUMBER)
            {
                result = context->rp;
                instruction = make_pair_instr(OP_LOADV, result, atoi(ast->op.literal.value));
                code_block_write(context->binary->code, instruction);
            }

            if (ast->op.literal.token.type == TOK_FLOAT)
            {
                result = context->rp;

                // First, set the constant in the text section of our binary
                val.type = VAL_FLOAT;
                val.contents.real = atof(ast->op.literal.value);
                memory_set(context->binary->data, context->mp, val);

                instruction = make_pair_instr(OP_LOAD, result, context->mp);
                code_block_write(context->binary->code, instruction);
                context->mp += 1;
            }

            if (ast->op.literal.token.type == TOK_STRING)
            {
                result = context->rp;

                // First, set the constant in the text section of our binary
                val = string_create(ast->op.literal.value);
                memory_set(context->binary->data, context->mp, val);

                // Now, write out an instruction to load it into a register
                instruction = make_pair_instr(OP_LOAD, result, context->mp);
                code_block_write(context->binary->code, instruction);
                context->mp += 1;
            }

            if (ast->op.literal.token.type == TOK_IDENTIFIER)
            {
                sym = symbol_map_get(context->symbols, ast->op.literal.value);
                if (sym.location.type == LOC_UNDEF)
                {
                    char *error;
                    location_t loc = {ast->location.start, ast->location.end};
                    asprintf(&error, "Use of undeclared identifier \"%s\"", ast->op.literal.value);
                    printf("%s", format_error(context->name, context->listing, error, loc));
                    exit(1);
                }

                if (sym.location.type == LOC_MEMORY)
                {
                    instruction = make_pair_instr(OP_LOAD, context->rp, sym.location.address);
                    code_block_write(context->binary->code, instruction);
                    sym.location.type = LOC_REGISTER;
                    sym.location.address = context->rp++;
                    symbol_map_set(context->symbols, sym);
                }

                result = sym.location.address;
            }

            if (ast->op.literal.token.type == TOK_TRUE || ast->op.literal.token.type == TOK_FALSE)
            {
                result = context->rp;
                instruction = make_pair_instr(OP_LOAD, result, (ast->op.literal.token.type == TOK_TRUE) ? 1 : 0);
                code_block_write(context->binary->code, instruction);
            }

            if (ast->op.literal.token.type == TOK_NIL)
            {
                result = context->rp;
                instruction = make_single_instr(OP_NIL, result);
                code_block_write(context->binary->code, instruction);
            }

            break;

        case AST_GROUP:
            result = compile_internal(ast->op.group, context);
            break;

        case AST_RANGE:
            tmp = context->rp;

            regs = malloc(sizeof(uint8_t) * 2);
            regs[0] = compile_internal(ast->op.range.begin, context);
            context->rp++;
            regs[1] = compile_internal(ast->op.range.end, context);

            result = compile_builtin_fn_call(
                context,
                "range",
                tmp,
                2,
                regs
            );

            break;

        case AST_TUPLE:
            regs = (uint8_t *)malloc(sizeof(uint8_t) * ast->op.list.size);
            tmp = context->rp;

            // First, calculate the values of the tuple
            for (int i = 0; i < ast->op.list.size; i++)
            {
                regs[i] = compile_internal(ast->op.list.items[i], context);

                if (regs[i] == context->rp)
                    context->rp++;
            }

            // Now, push them on the stack in reverse order
            for (int i = ast->op.list.size - 1; i >= 0; i--)
            {
                instruction = make_single_instr(OP_PUSH, regs[i]);
                code_block_write(context->binary->code, instruction);
            }

            context->rp = tmp;
            free(regs);

            // Now, set register 0 to the number of args
            instruction = make_pair_instr(OP_LOADV, 0, ast->op.list.size);
            code_block_write(context->binary->code, instruction);

            // Call tuple
            val = string_create("tuple");
            memory_set(context->binary->data, context->mp, val);
            tmp = context->mp;
            context->mp += 1;

            instruction = make_singlew_instr(OP_CALL_DYNAMIC, tmp);
            code_block_write(context->binary->code, instruction);

            instruction = make_single_instr(OP_POP, context->rp);
            code_block_write(context->binary->code, instruction);

            result = context->rp;

            break;

        case AST_STMT_LIST:
            for (int i = 0; i < ast->op.list.size; i++)
            {
                result = compile_internal(ast->op.list.items[i], context);
            }
            break;

        case AST_IF_STMT:
            // First, we compile our condition
            result = compile_internal(ast->op.if_stmt.condition, context);

            // Increment rp if necessary
            if (result == context->rp)
                context->rp += 1;

            // Set our address to jump to if we evaluate to false. This is a dummy
            // value for now, we'll come back later and modify it
            tmp = context->binary->code->size;
            instruction = make_pair_instr(OP_LOADV, context->rp++, 0);
            code_block_write(context->binary->code, instruction);

            // Load "true" into a register to compare against
            instruction = make_pair_instr(OP_LOAD, context->rp, 1);
            code_block_write(context->binary->code, instruction);

            instruction = make_triplet_instr(OP_EQUAL, 0, context->rp, result);
            code_block_write(context->binary->code, instruction);

            // Condition evaluates to false
            instruction = make_single_instr(OP_JMP, context->rp - 1);
            code_block_write(context->binary->code, instruction);

            // Decrement rp since we no longer need our comparison or
            // jump variables
            context->rp -= 2;

            // Now we write out our block
            result = compile_internal(ast->op.if_stmt.body, context);

            addr = context->binary->code->size;
            context->binary->code->code[tmp].fields.pair.arg2 = addr;
            break;

        case AST_FOR_STMT:
            // First, we compile our collection
            result = compile_internal(ast->op.for_stmt.iterable, context);

            // Next, make an iterator over the collection
            left = compile_builtin_fn_call(
                context,
                "iter",
                context->rp,
                1,
                &result
            );

            // Need to keep track of our iterator
            context->rp += 1;

            // Now, define a new symbol map for the for loop
            symbol_map_t *for_map = symbol_map_create();
            for_map->parent = context->symbols;
            context->symbols = for_map;

            // This will point to our local variable. We'll need this even if it's not
            // defined by the user.
            var = context->rp++;
            nil = context->rp++;

            instruction = make_single_instr(OP_NIL, nil);
            code_block_write(context->binary->code, instruction);

            // If a local variable was defined, set it in the synbol map
            if (ast->op.for_stmt.var != NULL)
            {
                sym.type = SYM_VAR;
                sym.name = ast->op.for_stmt.var;
                sym.location.type = LOC_REGISTER;
                sym.location.address = var;
                symbol_map_set(context->symbols, sym);
            }

            // Begin for-loop
            begin = context->binary->code->size;
            instruction = make_triplet_instr(OP_DEREF, var, left, 1);
            code_block_write(context->binary->code, instruction);

            // Dummy exit jump
            end = context->binary->code->size;
            instruction = make_pair_instr(OP_LOADV, context->rp, 0);
            code_block_write(context->binary->code, instruction);

            instruction = make_triplet_instr(OP_EQUAL, 1, var, nil);
            code_block_write(context->binary->code, instruction);

            // Save our instruction for modification later
            instruction = make_single_instr(OP_JMP, context->rp);
            code_block_write(context->binary->code, instruction);

            // Now, compile our for block
            compile_internal(ast->op.for_stmt.body, context);

            // Jump to the start of the loop
            instruction = make_pair_instr(OP_LOADV, context->rp, begin);
            code_block_write(context->binary->code, instruction);
            instruction = make_single_instr(OP_JMP, context->rp);
            code_block_write(context->binary->code, instruction);

            // Now, fix up our exit jump
            context->binary->code->code[end].fields.pair.arg2 = context->binary->code->size;

            context->rp -= 2;

            break;

        case AST_FUNCTION_DECL:
            args = ast->op.fn.args;

            // First, write out a dummy instruction which will be used to jump
            // over the function.
            instruction = make_pair_instr(OP_LOADV, context->rp, 0);
            tmp = context->binary->code->size;
            code_block_write(context->binary->code, instruction);
            instruction = make_single_instr(OP_JMP, context->rp);
            code_block_write(context->binary->code, instruction);

            addr = context->binary->code->size;

            // Next, create a new symbol map for our new function, and capture
            // our arguments
            symbol_map_t *fn_map = symbol_map_create();
            fn_map->parent = context->symbols;
            context->symbols = fn_map;
            uint8_t rp = context->rp;

            // Create the function definition
            uint64_t mp = context->mp++;
            value_t func = function_def_create(
                ast->op.fn.name,
                addr,
                (args == NULL) ? 0 : args->op.list.size,
                NULL,
                rp
            );
            memory_set(context->binary->data, mp, func);

            // Set symbol information for the function itself
            sym.name = ast->op.fn.name;
            sym.type = SYM_FN;
            sym.location.type = LOC_MEMORY;
            sym.location.address = mp;

            // We need to write out the function symbol to our new symbol map
            // to support recursive calls
            symbol_map_set(context->symbols, sym);

            if (args != NULL)
            {
                for (int i = args->op.list.size - 1; i >= 0; i--)
                {
                    sym.location.type = LOC_REGISTER;
                    sym.location.address = context->rp + i;
                    sym.name = args->op.list.items[i]->op.literal.value;
                    sym.type = SYM_VAR;
                    symbol_map_set(context->symbols, sym);
                }
                context->rp += args->op.list.size;
            }

            // Now, write out the block
            result = compile_internal(ast->op.fn.body, context);

            // Add an implicit return if needed
            size_t address = context->binary->code->size - 1;
            if (context->binary->code->code[address].opcode != OP_RETURN)
            {
                instruction = make_single_instr(OP_RETURN, result);
                code_block_write(context->binary->code, instruction);
            }

            // Finally, modify our first jump instruction to jump to the correct address
            address = context->binary->code->size;
            context->binary->code->code[tmp].fields.pair.arg2 = address;

            // Now, reset our symbol map
            symbol_map_t *map = context->symbols;
            context->symbols = context->symbols->parent;
            symbol_map_destroy(map);

            // Construct locals to keep track of
            uint8_t *locals = (uint8_t *)malloc(context->rp - rp + 1);
            for (int i = rp; i < context->rp + 1; i++)
            {
                locals[i - rp] = i;
            }
            locals[context->rp - rp + 1] = 0;

            // Update our function definition with locals
            function_t *f = (function_t *)func.contents.object;
            f->locals = locals;
            memory_set(context->binary->data, mp, func);

            // Set symbol information for the function itself
            sym.name = ast->op.fn.name;
            sym.type = SYM_FN;
            sym.location.type = LOC_MEMORY;
            sym.location.address = mp;

            context->rp = rp;

            // If we're an anonymous function, load it into a register
            if (strcmp(ast->op.fn.name, "__anonymous") == 0)
            {
                instruction = make_pair_instr(OP_LOAD, context->rp, mp);
                result = context->rp;
                code_block_write(context->binary->code, instruction);
            }

            // Store our function in the symbol map
            symbol_map_set(context->symbols, sym);
            break;

        case AST_FUNCTION_CALL:
            sym = symbol_map_get(context->symbols, ast->op.call.name);
            args = ast->op.call.args;
            function_t *function;

            // Assume undefined symbols are builtins
            if (sym.location.type == LOC_UNDEF || sym.location.type == LOC_BUILTIN)
            {
                regs = NULL;
                tmp = context->rp;

                if (args != NULL)
                {
                    regs = (uint8_t *)malloc(sizeof(uint8_t) * args->op.list.size);

                    // First, calculate the values of our arguments
                    for (int i = 0; i < args->op.list.size; i++)
                    {
                        regs[i] = compile_internal(args->op.list.items[i], context);

                        if (regs[i] == context->rp)
                            context->rp++;
                    }
                }

                result = compile_builtin_fn_call(
                    context,
                    ast->op.call.name,
                    tmp,
                    (args == NULL) ? 0 : args->op.list.size,
                    regs
                );

                free(regs);

                break;
            }
            else if (sym.location.type == LOC_MEMORY)
            {
                // We'll use the function definition for analysis and register spilling
                function = (function_t *)memory_get(context->binary->data, sym.location.address).contents.object;
            }
            else if (sym.location.type == LOC_REGISTER)
            {

            }
            else
            {
                assert(sym.location.type);
            }

            // Now, iterate through the function args and save any locals that conflict
            for (uint8_t i = 0; i < function->nargs; i++)
            {

                if (function->locals == NULL)
                {
                    if (function->low_reg + i >= context->rp)
                        continue;

                    instruction = make_single_instr(OP_PUSH, function->low_reg + i);
                    code_block_write(context->binary->code, instruction);
                }
                else if (function->locals[i] < context->rp)
                {
                    instruction = make_single_instr(OP_PUSH, function->locals[i]);
                    code_block_write(context->binary->code, instruction);
                }
            }

            if (args != NULL)
            {
                if (args->op.list.size != function->nargs)
                {
                    char *error;
                    location_t loc = {args->location.start, args->location.end};
                    asprintf(&error, "Function \"%s\" expected %d arguments, but was passed %ld.",
                        function->name,
                        function->nargs,
                        args->op.list.size
                    );
                    printf("%s", format_error(context->name, context->listing, error, loc));
                    exit(1);
                }

                rp = context->rp;

                // First, set the register pointer to our first local of the function we're calling
                context->rp = function->low_reg;

                for (int i = 0; i < args->op.list.size; i++)
                {
                    uint8_t val = compile_internal(args->op.list.items[i], context);

                    // If we weren't serendipitous, move things around
                    if (val != function->low_reg + i)
                    {
                        instruction = make_pair_instr(OP_MOVE, function->low_reg + i, val);
                        code_block_write(context->binary->code, instruction);
                    }

                    context->rp += 1;
                }
            }

            // Call the function
            instruction = make_singlew_instr(OP_CALL, sym.location.address);
            code_block_write(context->binary->code, instruction);

            if (args != NULL)
                context->rp = rp;

            // Pop the return value
            instruction = make_single_instr(OP_POP, context->rp);
            result = context->rp;
            code_block_write(context->binary->code, instruction);

            // Pop our locals back into registers
            for (int i = function->nargs - 1; i >= 0; --i)
            {
                if (function->low_reg + i < context->rp)
                {
                    instruction = make_single_instr(OP_POP, function->low_reg + i);
                    code_block_write(context->binary->code, instruction);
                }
            }

            break;

        default:
            ;
    }

    return result;
}

binary_t *compile(const char *name, const char *listing, ast_t *ast)
{
    compile_context_t *context = context_create(name, listing);
    compile_internal(ast, context);
    binary_t *binary = context->binary;
    context_destroy(context);
    return binary;
}
