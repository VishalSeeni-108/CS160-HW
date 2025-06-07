#include <cassert>
#include <typeinfo>

#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"


class Codegen : public Visitor
{
  private:
    FILE* m_outputfile;
    SymTab *m_st;

    // Basic size of a word (integers and booleans) in bytes
    static const int wordsize = 4;

    int label_count; // Access with new_label

    // Helpers
    // This is used to get new unique labels (cleverly names label1, label2, ...)
    int new_label()
    {
        return label_count++;
    }

    void set_text_mode()
    {
        fprintf(m_outputfile, ".text\n\n");
    }

    void set_data_mode()
    {
        fprintf(m_outputfile, ".data\n\n");
    }

    // PART 1:
    // 1) get arithmetic expressions on integers working:
    //  you wont really be able to run your code,
    //  but you can visually inspect it to see that the correct
    //  chains of opcodes are being generated.
    // 2) get procedure calls working:
    //  if you want to see at least a very simple program compile
    //  and link successfully against gcc-produced code, you
    //  need to get at least this far
    // 3) get boolean operation working
    //  before we can implement any of the conditional control flow
    //  stuff, we need to have booleans worked out.
    // 4) control flow:
    //  we need a way to have if-elses and while loops in our language.
    // 5) arrays: just like variables, but with an index

    // Hint: the symbol table has been augmented to track an offset
    //  with all of the symbols.  That offset can be used to figure
    //  out where in the activation record you should look for a particuar
    //  variable


    ///////////////////////////////////////////////////////////////////////////////
    //
    //  function_prologue
    //  function_epilogue
    //
    //  Together these two functions implement the callee-side of the calling
    //  convention.  A stack frame has the following layout:
    //
    //                         <- SP (before pre-call / after epilogue)
    //  high -----------------
    //       | actual arg 1  |
    //       |    ...        |
    //       | actual arg n  |
    //       -----------------
    //       |  Return Addr  |
    //       =================
    //       | temporary 1   | <- SP (when starting prologue)
    //       |    ...        |
    //       | temporary n   |
    //   low ----------------- <- SP (when done prologue)
    //
    //
    //              ||
    //              ||
    //             \  /
    //              \/
    //
    //
    //  The caller is responsible for placing the actual arguments
    //  and the return address on the stack. Actually, the return address
    //  is put automatically on the stack as part of the x86 call instruction.
    //
    //  On function entry, the callee
    //
    //  (1) allocates space for the callee's temporaries on the stack
    //
    //  (2) saves callee-saved registers (see below) - including the previous activation record pointer (%ebp)
    //
    //  (3) makes the activation record pointer (frmae pointer - %ebp) point to the start of the temporary region
    //
    //  (4) possibly copies the actual arguments into the temporary variables to allow easier access
    //
    //  On function exit, the callee:
    //
    //  (1) pops the callee's activation record (temporay area) off the stack
    //
    //  (2) restores the callee-saved registers, including the activation record of the caller (%ebp)
    //
    //  (3) jumps to the return address (using the x86 "ret" instruction, this automatically pops the
    //      return address off the stack
    //
    //////////////////////////////////////////////////////////////////////////////
    //
    // Since we are interfacing with code produced by GCC, we have to respect the
    // calling convention that GCC demands:
    //
    // Contract between caller and callee on x86:
    //    * after call instruction:
    //           o %eip points at first instruction of function
    //           o %esp+4 points at first argument
    //           o %esp points at return address
    //    * after ret instruction:
    //           o %eip contains return address
    //           o %esp points at arguments pushed by caller
    //           o called function may have trashed arguments
    //           o %eax contains return value (or trash if function is void)
    //           o %ecx, %edx may be trashed
    //           o %ebp, %ebx, %esi, %edi must contain contents from time of call
    //    * Terminology:
    //           o %eax, %ecx, %edx are "caller save" registers
    //           o %ebp, %ebx, %esi, %edi are "callee save" registers
    ////////////////////////////////////////////////////////////////////////////////


    void emit_prologue(SymName *name, unsigned int size_locals, unsigned int num_args)
    {
        // Declare label
        fprintf(m_outputfile, ".globl %s\n", name->spelling()); 
        fprintf(m_outputfile, "%s:\n", name->spelling()); 
        //Callee saved registers
        fprintf(m_outputfile, "\tpushl\t%%ebp\n"); 
        fprintf(m_outputfile, "\tpushl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpushl\t%%esi\n"); 
        fprintf(m_outputfile, "\tpushl\t%%edi\n"); 
        //Other setup 
        fprintf(m_outputfile, "\tmovl\t%%esp, %%ebp\n"); 
        fprintf(m_outputfile, "\tsubl\t$%d,%%esp\n", size_locals); 
    }

    void emit_epilogue()
    {
        fprintf(m_outputfile, "\tmovl\t%%ebp, %%esp\n"); 
        fprintf(m_outputfile, "\tpopl\t%%edi\n");
        fprintf(m_outputfile, "\tpopl\t%%esi\n");
        fprintf(m_outputfile, "\tpopl\t%%ebx\n");
        fprintf(m_outputfile, "\tpopl\t%%ebp\n");
        fprintf(m_outputfile, "\tret\n"); 
    }

  // WRITEME: more functions to emit code

  public:

    Codegen(FILE* outputfile, SymTab* st)
    {
        m_outputfile = outputfile;
        m_st = st;
        label_count = 0;
    }

    void visitProgramImpl(ProgramImpl* p)
    {
        set_text_mode(); 
        p->visit_children(this);
    }

    void visitProcImpl(ProcImpl* p)
    {
        emit_prologue(p->m_symname, m_st->scopesize(p->m_attribute.m_scope), 0); //num args will need to be changed
        p->visit_children(this);
        emit_epilogue(); 
    }

    void visitProcedure_blockImpl(Procedure_blockImpl* p)
    {
        p->visit_children(this);
    }

    void visitNested_blockImpl(Nested_blockImpl* p)
    {
        p->visit_children(this);
    }

    void visitAssignment(Assignment* p)
    {
        //p->visit_children(this);
        p->m_lhs->accept(this);
        if(Variable* lhs_var = dynamic_cast<Variable*>(p->m_lhs))
        {
            fprintf(m_outputfile, "\tmovl\t$%d,%%eax\n", m_st->lookup(lhs_var->m_attribute.m_scope, lhs_var->m_symname->spelling())->get_offset()); //Get offset
            fprintf(m_outputfile, "\tpushl\t%%eax\n"); //Push onto stack for later
        }

        p->m_expr->accept(this); 
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); //Pull value of expression
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); //Pull offset back
        fprintf(m_outputfile, "\tmovl %%ebx,\t(%%ebp, %%eax, 1)\n");
    }

    void visitCall(Call* p)
    {
        //p->visit_children(this);


        for (auto rit = p->m_expr_list->rbegin(); rit != p->m_expr_list->rend(); ++rit) {
            (*rit)->accept(this);
            fprintf(m_outputfile, "\tpopl\t%%eax\n");
            fprintf(m_outputfile, "\tpushl\t%%eax\n");
        }

        fprintf(m_outputfile, "\tcall\t%s\n", p->m_symname->spelling()); //Call method
        fprintf(m_outputfile, "\taddl $%d*4,%%esp\n",(int)p->m_expr_list->size());
        //Value of call in eax - push onto stack
        fprintf(m_outputfile, "\tpushl\t%%eax\n"); 

        p->m_lhs->accept(this);
        if(Variable* lhs_var = dynamic_cast<Variable*>(p->m_lhs))
        {
            fprintf(m_outputfile, "\tmovl\t$%d,%%eax\n", m_st->lookup(lhs_var->m_attribute.m_scope, lhs_var->m_symname->spelling())->get_offset()); //Get offset
            fprintf(m_outputfile, "\tpushl\t%%eax\n"); //Push onto stack for later
        }

        fprintf(m_outputfile, "\tpopl\t%%eax\n"); //Pull offset back
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); //Pull value back
        fprintf(m_outputfile, "\tmovl %%ebx,\t(%%ebp, %%eax, 1)\n");
    }

    void visitReturn(Return* p)
    {
        p->visit_children(this);

        // Load expression into %eax
        fprintf(m_outputfile, "\tpopl\t%%eax\n");
    }

    // Control flow
    void visitIfNoElse(IfNoElse* p)
    {
        int end_label = new_label(); 

        //p->visit_children(this);
        p->m_expr->accept(this);
        fprintf(m_outputfile, "\tpopl\t%%eax\n");
        fprintf(m_outputfile, "\tcmpl\t$1,%%eax\n");
        fprintf(m_outputfile, "\tjne\t end_%d\n", end_label);
        p->m_nested_block->accept(this); 
        fprintf(m_outputfile, "end_%d: \n", end_label);
    }

    void visitIfWithElse(IfWithElse* p)
    {
        int false_label = new_label(); 
        int end_label = new_label(); 

        //p->visit_children(this);
        p->m_expr->accept(this); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t$1,%%eax\n"); 
        fprintf(m_outputfile, "\tjne\t false_%d\n", false_label);
        p->m_nested_block_1->accept(this); 
        fprintf(m_outputfile, "\tjmp\t end_%d\n", end_label); 
        fprintf(m_outputfile, "false_%d: \n", false_label);
        p->m_nested_block_2->accept(this); 
        fprintf(m_outputfile, "end_%d: \n", end_label);  
    }

    void visitWhileLoop(WhileLoop* p)
    {
        int head_label = new_label(); 
        int tail_label = new_label(); 
        //p->visit_children(this);
        fprintf(m_outputfile, "head_%d: \n", head_label); 
        p->m_expr->accept(this); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t$1,%%eax\n"); 
        fprintf(m_outputfile, "\tjne\ttail_%d\n", tail_label); 
        p->m_nested_block->accept(this); 
        fprintf(m_outputfile, "\tjmp\thead_%d\n", head_label); 
        fprintf(m_outputfile, "tail_%d: \n", tail_label); 
    }

    void visitCodeBlock(CodeBlock *p) 
    {
        p->visit_children(this);
    }

    // Variable declarations (no code generation needed)
    void visitDeclImpl(DeclImpl* p)
    {
        p->visit_children(this);
    }

    void visitTInteger(TInteger* p)
    {
        p->visit_children(this);
    }

    void visitTIntPtr(TIntPtr* p)
    {
        p->visit_children(this);
    }

    void visitTBoolean(TBoolean* p)
    {
        p->visit_children(this);
    }

    void visitTCharacter(TCharacter* p)
    {
        p->visit_children(this);
    }

    void visitTCharPtr(TCharPtr* p)
    {
        p->visit_children(this);
    }

    void visitTString(TString* p)
    {
        p->visit_children(this);
    }

    // Comparison operations
    void visitCompare(Compare* p)
    {
        int true_label = new_label(); 
        int end_label = new_label(); 

        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t%%ebx, %%eax\n");
        fprintf(m_outputfile, "\tje\ttrue_%d\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$0\n"); 
        fprintf(m_outputfile, "\tjmp\tend_%d\n", end_label);
        fprintf(m_outputfile, "true_%d:\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$1\n"); 
        fprintf(m_outputfile, "end_%d:\n", end_label);
    }

    void visitNoteq(Noteq* p)
    {
        int true_label = new_label(); 
        int end_label = new_label(); 

        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t%%ebx, %%eax\n");
        fprintf(m_outputfile, "\tjne\ttrue_%d\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$0\n"); 
        fprintf(m_outputfile, "\tjmp\tend_%d\n", end_label);
        fprintf(m_outputfile, "true_%d:\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$1\n"); 
        fprintf(m_outputfile, "end_%d:\n", end_label);
    }

    void visitGt(Gt* p)
    {
        int true_label = new_label(); 
        int end_label = new_label(); 

        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t%%ebx, %%eax\n");
        fprintf(m_outputfile, "\tjg\ttrue_%d\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$0\n"); 
        fprintf(m_outputfile, "\tjmp\tend_%d\n", end_label);
        fprintf(m_outputfile, "true_%d:\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$1\n"); 
        fprintf(m_outputfile, "end_%d:\n", end_label);
    }

    void visitGteq(Gteq* p)
    {
        int true_label = new_label(); 
        int end_label = new_label(); 

        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t%%ebx, %%eax\n");
        fprintf(m_outputfile, "\tjge\ttrue_%d\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$0\n"); 
        fprintf(m_outputfile, "\tjmp\tend_%d\n", end_label);
        fprintf(m_outputfile, "true_%d:\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$1\n"); 
        fprintf(m_outputfile, "end_%d:\n", end_label);
    }

    void visitLt(Lt* p)
    {
        int true_label = new_label();
        int end_label = new_label();

        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t%%ebx, %%eax\n");
        fprintf(m_outputfile, "\tjl\ttrue_%d\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$0\n"); 
        fprintf(m_outputfile, "\tjmp\tend_%d\n", end_label);
        fprintf(m_outputfile, "true_%d:\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$1\n"); 
        fprintf(m_outputfile, "end_%d:\n", end_label);
    }

    void visitLteq(Lteq* p)
    {
        int true_label = new_label(); 
        int end_label = new_label(); 

        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcmpl\t%%ebx, %%eax\n");
        fprintf(m_outputfile, "\tjle\ttrue_%d\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$0\n"); 
        fprintf(m_outputfile, "\tjmp\tend_%d\n", end_label);
        fprintf(m_outputfile, "true_%d:\n", true_label);
        fprintf(m_outputfile, "\tpushl\t$1\n"); 
        fprintf(m_outputfile, "end_%d:\n", end_label);
    }

    // Arithmetic and logic operations
    void visitAnd(And* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tandl\t%%ebx, %%eax\n"); 
        fprintf(m_outputfile, "\tpushl\t%%eax\n");
    }

    void visitOr(Or* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\torl\t%%ebx, %%eax\n"); 
        fprintf(m_outputfile, "\tpushl\t%%eax\n");
    }

    void visitMinus(Minus* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tsubl\t%%ebx, %%eax\n"); 
        fprintf(m_outputfile, "\tpushl\t%%eax\n"); 
    }

    void visitPlus(Plus* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\taddl\t%%ebx, %%eax\n"); 
        fprintf(m_outputfile, "\tpushl\t%%eax\n"); 
    }

    void visitTimes(Times* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\timull\t%%ebx, %%eax\n"); 
        fprintf(m_outputfile, "\tpushl\t%%eax\n"); 
    }

    void visitDiv(Div* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpopl\t%%eax\n"); 
        fprintf(m_outputfile, "\tcdq\n");
        fprintf(m_outputfile, "\tidivl\t%%ebx\n"); 
        fprintf(m_outputfile, "\tpushl\t%%eax\n"); 
    }

    void visitNot(Not* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%eax\n");
        fprintf(m_outputfile, "\tnotl\t%%eax\n");
        fprintf(m_outputfile, "\tpushl\t%%eax\n");
    }

    void visitUminus(Uminus* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpopl\t%%eax\n");
        fprintf(m_outputfile, "\tnegl\t%%eax\n");
        fprintf(m_outputfile, "\tpushl\t%%eax\n");
    }

    // Variable and constant access
    void visitIdent(Ident* p)
    {
        //p->visit_children(this);
        fprintf(m_outputfile, "\tmovl\t%d(%%ebp),%%eax\n", m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling())->get_offset());
        fprintf(m_outputfile, "\tpushl\t%%eax\n");
        
    }

    void visitBoolLit(BoolLit* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpushl\t$0x%x\n", p->m_primitive->m_data);
    }

    void visitCharLit(CharLit* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpushl\t$0x%x\n", p->m_primitive->m_data);
    }

    void visitIntLit(IntLit* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpushl\t$0x%x\n", p->m_primitive->m_data);
    }

    void visitNullLit(NullLit* p)
    {
        p->visit_children(this);
        fprintf(m_outputfile, "\tpushl\t$0\n");
    }

    void visitArrayAccess(ArrayAccess* p)
    {
        p->visit_children(this);
    }

    // LHS
    void visitVariable(Variable* p)
    {
        p->visit_children(this);
    }

    void visitDerefVariable(DerefVariable* p)
    {
        p->visit_children(this);
    }

    void visitArrayElement(ArrayElement* p)
    {
        p->visit_children(this);
    }

    // Special cases
    void visitSymName(SymName* p)
    {
        //p->visit_children(this);
    }

    void visitPrimitive(Primitive* p)
    {
        //p->visit_children(this);
    }

    // Strings
    void visitStringAssignment(StringAssignment* p)
    {
        p->visit_children(this);
    }

    void visitStringPrimitive(StringPrimitive* p)
    {
        //p->visit_children(this);
    }

    void visitAbsoluteValue(AbsoluteValue* p)
    {
        p->visit_children(this);
    }

    // Pointer
    void visitAddressOf(AddressOf* p)
    {
        p->visit_children(this);
    }

    void visitDeref(Deref* p)
    {
        p->visit_children(this);
    }
};


void dopass_codegen(Program_ptr ast, SymTab* st)
{
    Codegen* codegen = new Codegen(stdout, st);
    ast->accept(codegen);
    delete codegen;
}
