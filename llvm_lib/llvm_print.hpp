#ifndef LLVM_PRINT_HPP
#define LLVM_PRINT_HPP
#include "llvm_lib/llvm_common.h"
class LLVMPrint {
    llvm::Function * func_print_;
    llvm::IRBuilder<> * builder_ptr_;
    llvm::Type * t_int_;
    llvm::Type * t_double_;
    llvm::Type * t_double_p_;
    llvm::Type * t_doublev4_p_;
    public:
    LLVMPrint( llvm::Module * TheModule, llvm::LLVMContext * TheContext, llvm::IRBuilder<> * builder_ptr ) {
        builder_ptr_ = builder_ptr;
        std::vector<llvm::Type*> arg_type_print;

        t_int_ =  llvm::Type::getInt32Ty(*TheContext);
        arg_type_print.push_back(llvm::Type::getInt8PtrTy(*TheContext));

        llvm::FunctionType * func_type =  llvm::FunctionType::get( t_int_, arg_type_print, true);

        llvm::Constant * func_const = TheModule->getOrInsertFunction("printf", func_type );// AttributeSet().addAttribute( *TheContext, 1U, Attribute::NoAlias ));
        llvm::Function * func_print = reinterpret_cast<llvm::Function*>( func_const );
        func_print_ = func_print;
    }
    void PrintInt( llvm::Value * value ) {
    
        std::vector<llvm::Value *> values;
        llvm::Value *formatStr = builder_ptr_->CreateGlobalStringPtr("Print %d\n");
        values.push_back( formatStr );
        values.push_back(value);
        builder_ptr_->CreateCall( func_print_,values );

    }
    void PrintPtr( llvm::Value * value_ptr, const int lanes ) {
        std::vector<llvm::Value*> values;
        std::string str_format = std::string(" data ");
        for( int i = 0 ; i < lanes ; i++ ) { 
            str_format += " %lf ";
        }

        str_format += "\n";
        llvm::Value *formatStr = builder_ptr_->CreateGlobalStringPtr(str_format);
        values.push_back(formatStr);
        for( int i = 0 ; i < lanes ; i++ ) {
            llvm::Constant * index = llvm::ConstantInt::get( t_int_, i);  
            llvm::Value * ptr = builder_ptr_->CreateInBoundsGEP( value_ptr, index);
            llvm::Value * data = builder_ptr_->CreateLoad( ptr );
            values.push_back( data ); 
        }
        builder_ptr_->CreateCall( func_print_, values );
    }
    void PrintPoint() {
        
        llvm::Value *formatStr = builder_ptr_->CreateGlobalStringPtr("Hello world\n");

        std::vector<llvm::Value*> values;

        values.push_back( formatStr ); 

        builder_ptr_->CreateCall( func_print_, values );
    }
};
#define LLVMPrintPoint(mod,ctx,build)  LLVMPrint( mod,ctx,build).PrintPoint()

#define LLVMPrintInt(mod,ctx,build,data)  LLVMPrint( mod,ctx,build).PrintInt(data)

#define LLVMPrintPtr(mod,ctx,build,ptr,lanes)  LLVMPrint( mod,ctx,build).PrintPtr(ptr,lanes)


#endif
