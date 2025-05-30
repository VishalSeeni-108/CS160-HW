#include <iostream>
#include <cstdio>
#include <cstring>

#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"

// WRITEME: The default attribute propagation rule
#define default_rule(X) (X)->visit_children(this);

#include <typeinfo>

class Typecheck : public Visitor
{
  private:
    FILE* m_errorfile;
    SymTab* m_st;

    // The set of recognized errors
    enum errortype
    {
        no_main,
        nonvoid_main,
        dup_proc_name,
        dup_var_name,
        proc_undef,
        call_type_mismatch,
        narg_mismatch,
        expr_type_err,
        var_undef,
        ifpred_err,
        whilepred_err,
        incompat_assign,
        who_knows,
        ret_type_mismatch,
        array_index_error,
        no_array_var,
        arg_type_mismatch,
        expr_pointer_arithmetic_err,
        expr_abs_error,
        expr_addressof_error,
        invalid_deref
    };

    // Print the error to file and exit
    void t_error(errortype e, Attribute a)
    {
        fprintf(m_errorfile,"on line number %d, ", a.lineno);

        switch(e)
        {
            case no_main:
                fprintf(m_errorfile, "error: no main\n");
                exit(2);
            case nonvoid_main:
                fprintf(m_errorfile, "error: the Main procedure has arguments\n");
                exit(3);
            case dup_proc_name:
                fprintf(m_errorfile, "error: duplicate procedure names in same scope\n");
                exit(4);
            case dup_var_name:
                fprintf(m_errorfile, "error: duplicate variable names in same scope\n");
                exit(5);
            case proc_undef:
                fprintf(m_errorfile, "error: call to undefined procedure\n");
                exit(6);
            case var_undef:
                fprintf(m_errorfile, "error: undefined variable\n");
                exit(7);
            case narg_mismatch:
                fprintf(m_errorfile, "error: procedure call has different number of args than declartion\n");
                exit(8);
            case arg_type_mismatch:
                fprintf(m_errorfile, "error: argument type mismatch\n");
                exit(9);
            case ret_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in return statement\n");
                exit(10);
            case call_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in procedure call args\n");
                exit(11);
            case ifpred_err:
                fprintf(m_errorfile, "error: predicate of if statement is not boolean\n");
                exit(12);
            case whilepred_err:
                fprintf(m_errorfile, "error: predicate of while statement is not boolean\n");
                exit(13);
            case array_index_error:
                fprintf(m_errorfile, "error: array index not integer\n");
                exit(14);
            case no_array_var:
                fprintf(m_errorfile, "error: attempt to index non-array variable\n");
                exit(15);
            case incompat_assign:
                fprintf(m_errorfile, "error: type of expr and var do not match in assignment\n");
                exit(16);
            case expr_type_err:
                fprintf(m_errorfile, "error: incompatible types used in expression\n");
                exit(17);
            case expr_abs_error:
                fprintf(m_errorfile, "error: absolute value can only be applied to integers and strings\n");
                exit(17);
            case expr_pointer_arithmetic_err:
                fprintf(m_errorfile, "error: invalid pointer arithmetic\n");
                exit(18);
            case expr_addressof_error:
                fprintf(m_errorfile, "error: AddressOf can only be applied to integers, chars, and indexed strings\n");
                exit(19);
            case invalid_deref:
                fprintf(m_errorfile, "error: Deref can only be applied to integer pointers and char pointers\n");
                exit(20);
            default:
                fprintf(m_errorfile, "error: no good reason\n");
                exit(21);
        }
    }

    // Helpers
    // WRITEME: You might want write some hepler functions.
    const char* lhs_to_id(Lhs* lhs)
    {
        Variable *v = dynamic_cast<Variable*>(lhs);
        if(v)
        {
            return v->m_symname->spelling(); 
        }

        DerefVariable *dv = dynamic_cast<DerefVariable*>(lhs);
        if(dv)
        {
            return dv->m_symname->spelling(); 
        }

        ArrayElement *ae = dynamic_cast<ArrayElement*>(lhs);
        if(ae)
        {
            return ae->m_symname->spelling(); 
        }

        return nullptr; 
    }

    // Type Checking
    // WRITEME: You need to implement type-checking for this project

    // Check that there is one and only one main
    void check_for_one_main(ProgramImpl* p)
    {
        int numMains = 0;
        if(p->m_proc_list->size() < 1)
        {
            this->t_error(no_main, p->m_attribute);
        }

        for(auto it = p->m_proc_list->begin(); it != p->m_proc_list->end(); it++)
        {
            ProcImpl *pip = dynamic_cast<ProcImpl*>((*it));
            char *name = strdup(pip->m_symname->spelling()); 

            if(strcmp(name, "Main") == 0)
            {
                numMains++; 
            }

            if(numMains > 1)
            {
                this->t_error(dup_proc_name, p->m_attribute);
                break; 
            }
            else
            {
                if(!(pip->m_decl_list->empty()) && (strcmp(name, "Main") == 0))
                {
                    this->t_error(nonvoid_main, p->m_attribute);
                }
            }
        }

        if(numMains < 1)
        {
            this->t_error(no_main, p->m_attribute);
        }
    }

    // Create a symbol for the procedure and check there is none already
    // existing
    void add_proc_symbol(ProcImpl* p)
    {
        Symbol *s = new Symbol(); 
        char *name = strdup(p->m_symname->spelling());
        s->m_basetype = bt_procedure; //Set basetype of symbol

        if(!m_st->insert(name, s))
        {
            this->t_error(dup_proc_name, p->m_attribute);
        }


        m_st->open_scope(); //Open scope for current procedure

        //Recursively process args and add to m_arg_type array
        for(auto it = p->m_decl_list->begin(); it != p->m_decl_list->end(); it++)
        {
            DeclImpl *dip = dynamic_cast<DeclImpl*>((*it)); 
            dip->accept(this);

            for(int i = 0; i < dip->m_symname_list->size(); i++)
            {
                s->m_arg_type.push_back(dip->m_type->m_attribute.m_basetype);
            }
        }        

        p->m_type->accept(this); //Accept return type of proc
        s->m_return_type = p->m_type->m_attribute.m_basetype; //Set m_return_type in symbol
        //m_st->insert_in_parent_scope(name, s); //Insert in parent scope
        p->m_procedure_block->accept(this); //Accept body
        m_st->close_scope(); //Close scope

    }

    // Add symbol table information for all the declarations following
    void add_decl_symbol(DeclImpl* p)
    {
        for(auto it = p->m_symname_list->begin(); it != p->m_symname_list->end(); it++)
        {
            Symbol *s = new Symbol(); 
            char *name = strdup((*it)->spelling()); 
            s->m_basetype = p->m_type->m_attribute.m_basetype; 
            if(!m_st->insert(name, s))
            {
                this->t_error(dup_var_name, p->m_attribute);
            }   
        }
    }

    // Check that the return statement of a procedure has the appropriate type
    void check_proc(ProcImpl *p)
    {
        //Get proc block impl
        Procedure_blockImpl *pb = dynamic_cast<Procedure_blockImpl*>(p->m_procedure_block); 
        //Special case for return null
        if(pb->m_return_stat->m_attribute.m_basetype == bt_ptr && ((p->m_type->m_attribute.m_basetype == bt_charptr) || (p->m_type->m_attribute.m_basetype == bt_intptr)))
        {
            return; 
        }
        //Check ret_type mismatch
        if(pb->m_return_stat->m_attribute.m_basetype != p->m_type->m_attribute.m_basetype)
        {
            this->t_error(ret_type_mismatch, p->m_attribute);
        }
    }

    // Check that the declared return type is not an array
    void check_return(Return *p)
    {
        //TODO - below pulls the child expression type as the return type, need to check this against the parent procedure return type
        p->m_attribute.m_basetype = p->m_expr->m_attribute.m_basetype; 

    }

    // Create a symbol for the procedure and check there is none already
    // existing
    void check_call(Call *p)
    {
        Symbol *sf, *sid; 
        const char *id = lhs_to_id(p->m_lhs); 
        const char *f = p->m_symname->spelling(); 

        //Check if LHS is defined - i.e. check for undefined variable
        if((sid = m_st->lookup(id)) == 0)
        {
            this->t_error(var_undef, p->m_attribute); 
        }

        //Check if proc is defined / undefined process
        if((sf = m_st->lookup(f)) == 0)
        {
            this->t_error(proc_undef, p->m_attribute); 
        }

        //Check if proc's basetype is actually bt_procedure
        if(sf->m_basetype != bt_procedure)
        {
            this->t_error(proc_undef, p->m_attribute);
        } 

        //Check if lhs type = proc's return type
        if(sid->m_basetype != sf->m_return_type)
        {
            this->t_error(call_type_mismatch, p->m_attribute);
        }

        //Check if number of arguments match
        if(sf->m_arg_type.size() != p->m_expr_list->size())
        {
            this->t_error(narg_mismatch, p->m_attribute);
        }

        //Check argument type
        std::vector<Basetype>::iterator formal_args_iter = sf->m_arg_type.begin(); 
        for(std::list<Expr_ptr>::iterator act_args_iter = p->m_expr_list->begin(); act_args_iter != p->m_expr_list->end(); act_args_iter++, formal_args_iter++)
        {
            Basetype act_type = (*act_args_iter)->m_attribute.m_basetype; 
            Basetype form_type = (*formal_args_iter); 

            if(act_type != form_type)
            {
                if(!((act_type == bt_ptr) && ((form_type == bt_charptr) || (form_type == bt_intptr))))
                {
                    this->t_error(arg_type_mismatch, p->m_attribute); 
                }
            }
        }
    }

    // For checking that this expressions type is boolean used in if/else
    void check_pred_if(Expr* p)
    {
        if(p->m_attribute.m_basetype != bt_boolean)
        {
            this->t_error(ifpred_err, p->m_attribute);
        }
    }

    // For checking that this expressions type is boolean used in while
    void check_pred_while(Expr* p)
    {
        if(p->m_attribute.m_basetype != bt_boolean)
        {
            this->t_error(whilepred_err, p->m_attribute);
        }
    }

    void check_assignment(Assignment* p)
    {
        Symbol *sid; 
        const char *id = lhs_to_id(p->m_lhs); 

         //Check if LHS is defined - i.e. check for undefined variable
        if((sid = m_st->lookup(id)) == 0)
        {
            this->t_error(var_undef, p->m_attribute); 
        }

        //Check if LHS type is same as expression type/null pointer being assigned a char/int pointer
        if(sid->m_basetype != p->m_expr->m_attribute.m_basetype)
        {
            if(!(  ((sid->m_basetype == bt_charptr) && (p->m_expr->m_attribute.m_basetype == bt_ptr)) 
                || ((sid->m_basetype == bt_intptr) && (p->m_expr->m_attribute.m_basetype == bt_ptr))
                || ((dynamic_cast<ArrayElement*>(p->m_lhs)) && (p->m_expr->m_attribute.m_basetype == bt_char)) ))
            {
                if(((dynamic_cast<DerefVariable*>(p->m_lhs))))
                {
                    DerefVariable *dv = ((dynamic_cast<DerefVariable*>(p->m_lhs))); 
                    char* name = strdup(dv->m_symname->spelling());
                    Symbol *s = m_st->lookup(name);
                    if(!((s->m_basetype == bt_intptr && p->m_expr->m_attribute.m_basetype == bt_integer) || (s->m_basetype == bt_charptr && p->m_expr->m_attribute.m_basetype == bt_char)))
                    {
                        this->t_error(incompat_assign, p->m_attribute);
                    }
                }
                else
                {
                    this->t_error(incompat_assign, p->m_attribute);
                }
            }
        }
    }

    void check_string_assignment(StringAssignment* p)
    {
        Symbol *sid; 
        const char *id = lhs_to_id(p->m_lhs); 

         //Check if LHS is defined - i.e. check for undefined variable
        if((sid = m_st->lookup(id)) == 0)
        {
            this->t_error(var_undef, p->m_attribute); 
        }

        //Check if LHS type is a string
        if(sid->m_basetype != bt_string)
        {
            this->t_error(incompat_assign, p->m_attribute);
        }
    }

    void check_array_access(ArrayAccess* p)
    {
        //Check is symname is defined
        char *name = strdup(p->m_symname->spelling());
        if(!(m_st->exist(name)))
        {
            this->t_error(var_undef, p->m_attribute);
        }

        if(p->m_expr->m_attribute.m_basetype != bt_integer)
        {
            this->t_error(array_index_error, p->m_attribute); 
        }
        
        Symbol *s = m_st->lookup(name); 
        if(s->m_basetype != bt_string)
        {
            this->t_error(no_array_var, p->m_attribute);
        }

        p->m_attribute.m_basetype = bt_char; 
    }

    void check_array_element(ArrayElement* p)
    {
        //Check is symname is defined
        char *name = strdup(p->m_symname->spelling());
        if(!(m_st->exist(name)))
        {
            this->t_error(var_undef, p->m_attribute);
        }

        if(p->m_expr->m_attribute.m_basetype != bt_integer)
        {
            this->t_error(array_index_error, p->m_attribute); 
        }
        
        Symbol *s = m_st->lookup(name); 
        if(s->m_basetype != bt_string)
        {
            this->t_error(no_array_var, p->m_attribute);
        }

       p->m_attribute.m_basetype = bt_char; 
    }

    // For checking boolean operations(and, or ...)
    void checkset_boolexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(child1->m_attribute.m_basetype != bt_boolean || child2->m_attribute.m_basetype != bt_boolean)
        {
            this->t_error(expr_type_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_boolean; 
    }

    // For checking arithmetic expressions(plus, times, ...)
    void checkset_arithexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(child1->m_attribute.m_basetype == bt_intptr || child1->m_attribute.m_basetype == bt_charptr || child1->m_attribute.m_basetype == bt_ptr ||
           child2->m_attribute.m_basetype == bt_intptr || child2->m_attribute.m_basetype == bt_charptr || child2->m_attribute.m_basetype == bt_ptr)
        {
            this->t_error(expr_pointer_arithmetic_err, parent->m_attribute);
        }

        if(child1->m_attribute.m_basetype != bt_integer || child2->m_attribute.m_basetype != bt_integer)
        {
            this->t_error(expr_type_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_integer; 
    }

    // Called by plus and minus: in these cases we allow pointer arithmetics
    void checkset_arithexpr_or_pointer(Expr* parent, Expr* child1, Expr* child2)
    {
        if((child1->m_attribute.m_basetype != bt_charptr) || (child2->m_attribute.m_basetype != bt_integer))
        {
            this->t_error(expr_pointer_arithmetic_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_charptr; 
    }

    // For checking relational(less than , greater than, ...)
    void checkset_relationalexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(child1->m_attribute.m_basetype != bt_integer || child2->m_attribute.m_basetype != bt_integer)
        {
            this->t_error(expr_type_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_boolean;
    }

    // For checking equality ops(equal, not equal)
    void checkset_equalityexpr(Expr* parent, Expr* child1, Expr* child2)
    {
        if(!((child1->m_attribute.m_basetype == bt_integer && child2->m_attribute.m_basetype == bt_integer) || (child1->m_attribute.m_basetype == bt_boolean && child2->m_attribute.m_basetype == bt_boolean) 
          || (child1->m_attribute.m_basetype == bt_char && child2->m_attribute.m_basetype == bt_char)))
        {
            if(child1->m_attribute.m_basetype == bt_charptr)
            {
                if(!(child2->m_attribute.m_basetype == bt_charptr || child2->m_attribute.m_basetype == bt_ptr))
                {
                    this->t_error(expr_type_err, parent->m_attribute);
                }
            }
            else if(child1->m_attribute.m_basetype == bt_intptr)
            {
                if(!(child2->m_attribute.m_basetype == bt_intptr || child2->m_attribute.m_basetype == bt_ptr))
                {
                    this->t_error(expr_type_err, parent->m_attribute);
                }
            }
            else if(child1->m_attribute.m_basetype == bt_ptr)
            {
                if(!(child2->m_attribute.m_basetype == bt_charptr || child2->m_attribute.m_basetype == bt_intptr || child2->m_attribute.m_basetype == bt_ptr))
                {
                    this->t_error(expr_type_err, parent->m_attribute);
                }
            }
            else
            {
                this->t_error(expr_type_err, parent->m_attribute);
            }
        }

        parent->m_attribute.m_basetype = bt_boolean; 
    }

    // For checking not
    void checkset_not(Expr* parent, Expr* child)
    {
        if(child->m_attribute.m_basetype != bt_boolean)
        {
            this->t_error(expr_type_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_boolean;
    }

    // For checking unary minus
    void checkset_uminus(Expr* parent, Expr* child)
    {
        if(child->m_attribute.m_basetype != bt_integer)
        {
            this->t_error(expr_type_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_integer;
    }

    void checkset_absolute_value(Expr* parent, Expr* child)
    {
        if(!(child->m_attribute.m_basetype == bt_integer || child->m_attribute.m_basetype == bt_string))
        {
            this->t_error(expr_type_err, parent->m_attribute);
        }

        parent->m_attribute.m_basetype = bt_integer;
    }

    void checkset_addressof(Expr* parent, Lhs* child)
    {
        Symbol *sid; 
        const char *id = lhs_to_id(child); 

         //Check if LHS is defined - i.e. check for undefined variable
        if((sid = m_st->lookup(id)) == 0)
        {
            this->t_error(var_undef, parent->m_attribute); 
        }

        //Check if LHS type is an integer, char, or indexed string and set appropriate value
        if(sid->m_basetype == bt_integer)
        {
            parent->m_attribute.m_basetype = bt_intptr; 
        }
        else if(sid->m_basetype == bt_char || (dynamic_cast<ArrayElement*>(child)) )
        {
            parent->m_attribute.m_basetype = bt_charptr; 
        }
        else if((dynamic_cast<DerefVariable*>(child)))
        {
            DerefVariable *dv = (dynamic_cast<DerefVariable*>(child)); 
            char* name = strdup(dv->m_symname->spelling()); 
            Symbol *s = m_st->lookup(name); 
            if(s->m_basetype == bt_intptr)
            {
                parent->m_attribute.m_basetype = bt_intptr; 
            }
            else if(s->m_basetype == bt_charptr)
            {
                parent->m_attribute.m_basetype = bt_charptr; 
            }
            else
            {
                this->t_error(expr_addressof_error, parent->m_attribute);
            }

        }
        else
        {
            this->t_error(expr_addressof_error, parent->m_attribute);
        }
    }

    void checkset_deref_expr(Deref* parent,Expr* child)
    {
        if(child->m_attribute.m_basetype == bt_intptr)
        {
            parent->m_attribute.m_basetype = bt_integer; 
        }
        else if(child->m_attribute.m_basetype == bt_charptr)
        {
            parent->m_attribute.m_basetype = bt_char; 
        }
        else
        {
            this->t_error(invalid_deref, parent->m_attribute);
        }
    }

    // Check that if the right-hand side is an lhs, such as in case of
    // addressof
    void checkset_deref_lhs(DerefVariable* p)
    {
        //Check is symname is defined
        char *name = strdup(p->m_symname->spelling());
        if(!(m_st->exist(name)))
        {
            this->t_error(var_undef, p->m_attribute);
        }

        Symbol *s = m_st->lookup(name); 

        if(s->m_basetype == bt_intptr)
        {
            p->m_attribute.m_basetype = bt_integer; 
        }
        else if(s->m_basetype == bt_charptr)
        {
            p->m_attribute.m_basetype = bt_char; 
        }
        else
        {
            this->t_error(invalid_deref, p->m_attribute);
        }
    }

    void checkset_variable(Variable* p)
    {
        //Duplicate variables checked by add_decl_symbol
        //Variables added to symbol table in add_decl_symbol/DeclImpl as well
        //Check if variable is in symbol table and throw error if it isn't
        char *name = strdup(p->m_symname->spelling());
        if(m_st->exist(name))
        {
            Symbol* s = m_st->lookup(name);
            p->m_attribute.m_basetype = s->m_basetype; //Pass type from symbol table back into variable
        }
        else //Variable not in symbol table i.e. not declared yet (TODO - figure out how scoping fits into this)
        {
            this->t_error(var_undef, p->m_attribute);
        }
    }


  public:

    Typecheck(FILE* errorfile, SymTab* st) {
        m_errorfile = errorfile;
        m_st = st;
    }

    void visitProgramImpl(ProgramImpl* p)
    {
        check_for_one_main(p); 
        for(auto it = p->m_proc_list->begin(); it != p->m_proc_list->end(); it++)
        {
            ProcImpl *pip = dynamic_cast<ProcImpl*>((*it));
            pip->accept(this);
        }
    }

    void visitProcImpl(ProcImpl* p)
    {
        add_proc_symbol(p); 
        check_proc(p); 
    }

    void visitCall(Call* p)
    {
        default_rule(p)
        check_call(p); 
    }

    void visitNested_blockImpl(Nested_blockImpl* p)
    {
        m_st->open_scope(); 
        default_rule(p)
        m_st->close_scope(); 
    }

    void visitProcedure_blockImpl(Procedure_blockImpl* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = p->m_parent_attribute->m_basetype; 
    }

    void visitDeclImpl(DeclImpl* p)
    {
        default_rule(p)
        add_decl_symbol(p);
    }

    void visitAssignment(Assignment* p)
    {
        default_rule(p)
        check_assignment(p);
    }

    void visitStringAssignment(StringAssignment *p)
    {
        default_rule(p)
        check_string_assignment(p);
    }

    void visitIdent(Ident* p)
    {
        default_rule(p)
        char *name = strdup(p->m_symname->spelling());
        if(m_st->exist(name))
        {
            Symbol* s = m_st->lookup(name);
            p->m_attribute.m_basetype = s->m_basetype; //Pass type from symbol table back into identifier
        }
        else //Variable not in symbol table i.e. not declared yet (TODO - figure out how scoping fits into this)
        {
            this->t_error(var_undef, p->m_attribute);
        }

    }

    void visitReturn(Return* p)
    {
        default_rule(p)
        check_return(p);
    }

    void visitIfNoElse(IfNoElse* p)
    {
        default_rule(p)
        check_pred_if(p->m_expr); 
    }

    void visitIfWithElse(IfWithElse* p)
    {
        default_rule(p)
        check_pred_if(p->m_expr);
    }

    void visitWhileLoop(WhileLoop* p)
    {
        default_rule(p)
        check_pred_while(p->m_expr);

    }

    void visitCodeBlock(CodeBlock *p) 
    {
        default_rule(p)
    }

    void visitTInteger(TInteger* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_integer;
    }

    void visitTBoolean(TBoolean* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_boolean;
    }

    void visitTCharacter(TCharacter* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_char; 
    }

    void visitTString(TString* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_string; 
    }

    void visitTCharPtr(TCharPtr* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_charptr; 
    }

    void visitTIntPtr(TIntPtr* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_intptr; 
    }

    void visitAnd(And* p)
    {
        default_rule(p)
        checkset_boolexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitDiv(Div* p)
    {
        default_rule(p)
        checkset_arithexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitCompare(Compare* p)
    {
        default_rule(p)
        checkset_equalityexpr(p, p->m_expr_1, p->m_expr_2); 
    }

    void visitGt(Gt* p)
    {
        default_rule(p)
        checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitGteq(Gteq* p)
    {
        default_rule(p)
        checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitLt(Lt* p)
    {
        default_rule(p)
        checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitLteq(Lteq* p)
    {
        default_rule(p)
        checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitMinus(Minus* p)
    {
        default_rule(p)
        if(p->m_expr_1->m_attribute.m_basetype == bt_charptr)
        {
            checkset_arithexpr_or_pointer(p, p->m_expr_1, p->m_expr_2);
        }
        else
        {
            checkset_arithexpr(p, p->m_expr_1, p->m_expr_2);
        }
    }

    void visitNoteq(Noteq* p)
    {
        default_rule(p)
        checkset_equalityexpr(p, p->m_expr_1, p->m_expr_2); 
    }

    void visitOr(Or* p)
    {
        default_rule(p)
        checkset_boolexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitPlus(Plus* p)
    {
        default_rule(p)
        if(p->m_expr_1->m_attribute.m_basetype == bt_charptr)
        {
            checkset_arithexpr_or_pointer(p, p->m_expr_1, p->m_expr_2);
        }
        else
        {
            checkset_arithexpr(p, p->m_expr_1, p->m_expr_2);
        }
    }

    void visitTimes(Times* p)
    {
        default_rule(p)
        checkset_arithexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitNot(Not* p)
    {
        default_rule(p)
        checkset_not(p, p->m_expr);
    }

    void visitUminus(Uminus* p)
    {
        default_rule(p)
        checkset_uminus(p, p->m_expr); 
    }

    void visitArrayAccess(ArrayAccess* p)
    {
        default_rule(p)
        check_array_access(p); 
    }

    void visitIntLit(IntLit* p)
    {
        default_rule(p); 
        p->m_attribute.m_basetype = bt_integer; 

    }

    void visitCharLit(CharLit* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_char; 
    }

    void visitBoolLit(BoolLit* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_boolean;
    }

    void visitNullLit(NullLit* p)
    {
        default_rule(p)
        p->m_attribute.m_basetype = bt_ptr; 
    }

    void visitAbsoluteValue(AbsoluteValue* p)
    {
        default_rule(p)
        checkset_absolute_value(p, p->m_expr);
    }

    void visitAddressOf(AddressOf* p)
    {
        default_rule(p)
        checkset_addressof(p, p->m_lhs); 
    }

    void visitVariable(Variable* p)
    {
        default_rule(p)
        checkset_variable(p);
    }

    void visitDeref(Deref* p)
    {
        default_rule(p)
        checkset_deref_expr(p, p->m_expr); 
    }

    void visitDerefVariable(DerefVariable* p)
    {
        default_rule(p)
        checkset_deref_lhs(p); 
    }

    void visitArrayElement(ArrayElement* p)
    {
        default_rule(p)
        check_array_element(p);
    }

    // Special cases
    void visitPrimitive(Primitive* p) {}
    void visitSymName(SymName* p) {}
    void visitStringPrimitive(StringPrimitive* p) {}
};


void dopass_typecheck(Program_ptr ast, SymTab* st)
{
    Typecheck* typecheck = new Typecheck(stderr, st);
    ast->accept(typecheck); // Walk the tree with the visitor above
    delete typecheck;
}
