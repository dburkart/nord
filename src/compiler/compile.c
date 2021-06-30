/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>

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

uint8_t spill(compile_context_t *context, uint8_t low_reg)
{
    uint8_t num_spilled = 0;

    for (int i = context->rp - 1; i >= low_reg; i--)
    {
        code_block_write(context->binary->code, make_single_instr(OP_SAVE, i));
        num_spilled += 1;
    }

    return num_spilled;
}

uint8_t compile_internal(ast_t *ast, compile_context_t *context)
{
    uint8_t result = 0, left, right, *regs;
    symbol_t sym;
    instruction_t instruction;
    value_t val;
    int tmp, addr;
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

                if (result < context->rp)
                {
                    instruction = make_pair_instr(OP_MOVE, context->rp, result);
                    code_block_write(context->binary->code, instruction);
                    sym.location.address = context->rp;
                }
                else
                {
                    sym.location.address = result;
                }

                sym.location.type = LOC_REGISTER;
                context->rp += 1;
            }

            sym.name = ast->op.declare.name;
            sym.type = SYM_VAR;
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

            instruction = make_pair_instr(OP_MOVE, sym.location.address, result);
            code_block_write(context->binary->code, instruction);
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

            if (args != NULL)
            {
                for (int i = args->op.list.size - 1; i >= 0; i--)
                {
                    sym.location.type = LOC_REGISTER;
                    sym.location.address = context->rp + i;
                    sym.name = args->op.list.items[i]->op.literal.value;
                    sym.type = SYM_VAR;
                    symbol_map_set(context->symbols, sym);
                    instruction = make_single_instr(OP_POP, context->rp + i);
                    code_block_write(context->binary->code, instruction);
                }
                context->rp += args->op.list.size;
            }


            // Set symbol information for the function itself
            sym.name = ast->op.fn.name;
            sym.low_reg = rp;
            sym.location.type = LOC_CODE;
            sym.location.address = addr;

            // We need to write out the function symbol to our new symbol map
            // to support recursive calls
            symbol_map_set(context->symbols, sym);

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

            context->rp = sym.low_reg;

            // Store our function in the symbol map
            symbol_map_set(context->symbols, sym);
            break;

        case AST_FUNCTION_CALL:
            sym = symbol_map_get(context->symbols, ast->op.call.name);
            args = ast->op.call.args;

            // If the symbol is undefined, we assume that it is a builtin.
            // In the future, it could also be an imported symbol. On first
            // reference of a builtin, we put the symbol in our constant
            // pool and create an entry in the root symbol map.
            if (sym.location.type == LOC_UNDEF)
            {
                val = string_create(ast->op.call.name);
                memory_set(context->binary->data, context->mp, val);
                sym.location.type = LOC_MEMORY;
                sym.location.address = context->mp++;
                sym.name = ast->op.call.name;
                sym.type = SYM_FN;

                // Set the symbol in our root context, since builtins are
                // global symbols.
                symbol_map_t *map = context->symbols;
                while (map->parent != NULL)
                {
                    map = map->parent;
                }
                symbol_map_set(map, sym);
            }

            // Currently, functions with symbols that reside in memory are
            // functions which are builtin to the language. Eventually symbols
            // of this nature might also represent symbols imported from a
            // package.
            if (sym.location.type == LOC_MEMORY)
            {
                addr = sym.location.address;

                if (args != NULL)
                {
                    regs = (uint8_t *)malloc(sizeof(uint8_t) * args->op.list.size);
                    tmp = context->rp;

                    // First, calculate the values of our arguments
                    for (int i = 0; i < args->op.list.size; i++)
                    {
                        regs[i] = compile_internal(args->op.list.items[i], context);

                        if (regs[i] == context->rp)
                            context->rp++;
                    }

                    // Now, push them on the stack in reverse order
                    for (int i = args->op.list.size - 1; i >= 0; i--)
                    {
                        instruction = make_single_instr(OP_PUSH, regs[i]);
                        code_block_write(context->binary->code, instruction);
                    }

                    context->rp = tmp;
                    free(regs);
                }

                // Builtins don't require spilling since they are currently all
                // implemented in C.
                instruction = make_pair_instr(OP_LOADV, 0, (args) ? args->op.list.size : 0);
                code_block_write(context->binary->code, instruction);

                instruction = make_singlew_instr(OP_CALL_DYNAMIC, addr);
                code_block_write(context->binary->code, instruction);

                instruction = make_single_instr(OP_POP, context->rp);
                result = context->rp;
                code_block_write(context->binary->code, instruction);

                break;
            }

            // Spill symbols
            tmp = spill(context, sym.low_reg);

            if (args != NULL)
            {
                for (int i = 0; i < args->op.list.size; i++)
                {
                    uint8_t val = compile_internal(args->op.list.items[i], context);
                    instruction = make_single_instr(OP_PUSH, val);
                    code_block_write(context->binary->code, instruction);
                }
            }

            // Call the function
            instruction = make_pair_instr(OP_LOADV, context->rp, sym.location.address);
            code_block_write(context->binary->code, instruction);

            instruction = make_single_instr(OP_CALL, context->rp);
            code_block_write(context->binary->code, instruction);

            instruction = make_single_instr(OP_POP, context->rp);
            result = context->rp;
            code_block_write(context->binary->code, instruction);

            if (tmp)
            {
                instruction.opcode = OP_RESTORE;
                instruction.fields.pair.arg2 = tmp;
                code_block_write(context->binary->code, instruction);
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
