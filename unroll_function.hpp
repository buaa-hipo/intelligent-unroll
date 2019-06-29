#ifndef UNROLL_FUNCTION_HPP
#define UNROLL_FUNCTION_HPP
template<typename FType1, typename FType2, typename Mask> 
class UnrollFunction;

template<typename R1, typename ...AnalyzeArgs,typename R2,  typename ...Args,typename Mask>
class UnrollFunction<R1(AnalyzeArgs...),R2( Args...), Mask> {
    public:
    Mask * mask_ptr;
    FuncStatement * func_ptr_;

    LLVMModule<R2(*)(int**,Args...)> * llvm_module_ptr_ ;
    virtual R1 analyze(AnalyzeArgs ...args) = 0; 
    virtual void generate_func( ) = 0;
    virtual R2 operator()(Args ...args ) = 0;
    R1 compiler(AnalyzeArgs ...args) {
        analyze(std::forward<AnalyzeArgs>(args)...);
        generate_func();
        LLVMCodeGen codegen;
        codegen.AddFunction( func_ptr_ );
        llvm_module_ptr_ = new LLVMModule<R2(*)(int**,Args...)>( codegen.get_mod(),codegen.get_ctx() );
        llvm_module_ptr_->Init("llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx");

        llvm_module_ptr_->Init("llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx");

    }   
};

#endif
