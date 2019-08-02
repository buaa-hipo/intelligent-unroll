#ifndef UNROLL_FUNCTION_HPP
#define UNROLL_FUNCTION_HPP
#include "llvm_lib/llvm_module.h"
#include "llvm_lib/llvm_codegen.hpp"
template<typename FType1, typename FType2, typename FType3> 
class UnrollFunction;



template<typename R1, typename ...AnalyzeArgs,typename R2,  typename ...Args, typename ...AddArgs>
class UnrollFunction<R1(AnalyzeArgs...),R2( Args...), void(AddArgs...)> {
    public:
    FuncStatement * func_ptr_;
    using FunctionType = R2(*)(Args...,AddArgs...);
    FunctionType func_;
    LLVMModule<FunctionType> * llvm_module_ptr_ ;
    virtual R1 analyze(AnalyzeArgs ...args) = 0; 
    virtual void generate_func( ) = 0;
    virtual R2 operator()(Args ...args ) = 0;
    R1 compiler(AnalyzeArgs ...args) {
        analyze(std::forward<AnalyzeArgs>(args)...);
        generate_func();
        LLVMCodeGen codegen;
        codegen.AddFunction( func_ptr_ );
        //codegen.PrintModule();
        //LLVMLOG(INFO) << *codegen.get_mod();
        llvm_module_ptr_ = new LLVMModule< FunctionType >( codegen.get_mod(),codegen.get_ctx() );
        llvm_module_ptr_->Init("llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx");

        llvm_module_ptr_->Init("llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx");
         
        func_ = llvm_module_ptr_->GetFunction("function");
        //LOG(INFO) << llvm_module_ptr_->GetSource( "asm" );
    }   
};

#endif
