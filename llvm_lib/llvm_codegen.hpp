#include "statement.hpp"
#include <map>
#include "llvm_lib/llvm_common.h"
#include "llvm_lib/llvm_log.h"
#include "llvm_lib/llvm_print.hpp"
#include "log.h"
#include <string>
#include <memory>
#include "ir_func.hpp"
#define SET_DISPATCH(CLASS_NAME)  \
    ftype_ptr->set_dispatch<CLASS_NAME>([this](StateMent * __basic_class_ptr)->Value*{\
                CLASS_NAME * __class_name = dynamic_cast<CLASS_NAME*>( __basic_class_ptr );\
                if(__class_name==NULL){printf("type StateMent " "-> "#CLASS_NAME"failt");exit(1);}\
                return this->CodeGen_(__class_name);})\
 
class LLVMCodeGen {
    const int lanes_ = VECTOR;

    const int alinements_ = 256;
    using Value = llvm::Value;
    using Function = llvm::Function;
    using string = std::string;
    using AllocaInst = llvm::AllocaInst; 
    using BasicBlock = llvm::BasicBlock;


    using FType = ir_func<Value*(StateMent*)>; 
    std::map<Varience * , Value *> var_val_map;

    std::map<Varience * , Value *> arg_val_map_;
    std::set<Varience *> var_mutable_set_;
    std::unique_ptr<llvm::Module> mod_ptr_;
    std::unique_ptr<llvm::IRBuilder<>>  build_ptr_;
    std::unique_ptr<llvm::LLVMContext> ctx_ptr_;

    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    llvm::Type*  t_int_;

    llvm::Type*  t_int8_;
    llvm::Type*  t_int64_ ;
    llvm::Type*  t_bool_ ;
    llvm::Type*  t_double_;

    llvm::Type*  t_float_;

    llvm::Type*  t_int_p_ ;
    llvm::Type*  t_int64_p_;

    llvm::Type*  t_int8_p_ ;
    llvm::Type*  t_bool_p_;
    llvm::Type*  t_double_p_;

    llvm::Type*  t_float_ptr_;

    llvm::Type* t_bool_vec_;
    llvm::Type* t_int_vec_;
    llvm::Type* t_int64_vec_;
    llvm::Type*  t_double_vec_;

    llvm::Type*  t_float_vec_;

    llvm::Type*  t_float_ptr_vec_;

    llvm::Type*  t_float_vec_ptr_;

    llvm::Type* t_int_vec4_;
    llvm::Type*  t_double_vec4_;
    llvm::Type* t_void_;
    llvm::Type* t_int_vec_p_;
    llvm::Type*  t_double_vec_p_;
    llvm::Type* t_int_vec4_p_;
    llvm::Type*  t_double_vec4_p_;

    llvm::Type* t_double_ptr_vec4_;
    llvm::Type* t_int_ptr_vec_ ;
    llvm::Type*  t_double_ptr_vec_;

    llvm::Constant *Zero_ ;
    llvm::Constant* FZeroVec_;

    llvm::Constant *FZero_ ;

    llvm::Constant* DZeroVec_;

    llvm::Constant *DZero_ ;
    llvm::Constant *One_ ;
    llvm::Constant *True_;
    llvm::Constant * Null_;
    llvm::Constant * ZeroVec_;
    llvm::Type * Type2LLVMType(const Type & type) {
        if( type == __int_v ) { 
            return llvm::VectorType::get( t_int_,VECTOR ); 
        } else if( type == __double_v ){
            return llvm::VectorType::get( t_double_, VECTOR );
        } else if( type == __int ) {
            return t_int_;
        } else if( type == __double ) {
            return t_double_;
        } else if(type == __void){
            return t_void_; 
        } else if(type == __double_ptr ) {
            return t_double_->getPointerTo();
        } else if(type == __int_ptr) {
            return t_int_->getPointerTo();
        } else if(type == __bool_v){
            return t_bool_vec_;
        } else if(type == __double_v_pointer){
            return t_double_vec_p_;
        } else if( type == __int_v_ptr) {
            return t_int_vec_p_; 
        } else if(type == __double_pointer_v) {
            return t_double_ptr_vec_;
        } else if( type == __float_v ){ 
            return t_float_vec_;
        } else if( type == __float ) {
            return t_float_;
        } else if( type == __float_v_ptr ) {
            return t_float_vec_ptr_;
        } else if( type == __float_ptr ) {
            return t_float_ptr_;
        } else if( type == __float_ptr_v ) {
            return t_float_ptr_vec_;
        } else {
            LOG(FATAL) << type <<"type does not support ";
            return t_int_;
        }
    }
    public:
    std::unique_ptr<llvm::Module> get_mod() {
        return std::move(mod_ptr_);
    }
    std::unique_ptr<llvm::LLVMContext> get_ctx() {
        return std::move(ctx_ptr_);
    }

    LLVMCodeGen() {

        ctx_ptr_ = llvm::make_unique<llvm::LLVMContext>();
        mod_ptr_= llvm::make_unique<llvm::Module>("module",*ctx_ptr_);
	TheFPM = llvm::make_unique<llvm::legacy::FunctionPassManager>( mod_ptr_.get());
	TheFPM->add( llvm::createPromoteMemoryToRegisterPass());
	TheFPM->add( llvm::createInstructionCombiningPass() );
	TheFPM->add( llvm::createReassociatePass());
	TheFPM->add( llvm::createGVNPass());
	TheFPM->add(llvm::createCFGSimplificationPass());
	TheFPM->doInitialization();

        build_ptr_ = llvm::make_unique<llvm::IRBuilder<>>(*ctx_ptr_);
        t_void_ = llvm::Type::getVoidTy(*ctx_ptr_);

        t_int_ =  llvm::Type::getInt32Ty(*ctx_ptr_);

        t_int8_ =  llvm::Type::getInt8Ty(*ctx_ptr_);
        t_int64_ =  llvm::Type::getInt64Ty(*ctx_ptr_);
        t_bool_ =  llvm::Type::getInt1Ty(*ctx_ptr_);
        t_double_ =  llvm::Type::getDoubleTy(*ctx_ptr_);
        t_float_ =  llvm::Type::getFloatTy(*ctx_ptr_);

        t_float_ptr_ = t_float_->getPointerTo(); 

        t_float_vec_ = llvm::VectorType::get( t_float_, lanes_ );

        t_float_ptr_vec_ = llvm::VectorType::get( t_float_ptr_, lanes_ );

        t_float_vec_ptr_ = t_float_vec_->getPointerTo();

        t_int_p_ = t_int_->getPointerTo();
        t_int8_p_ = t_int8_->getPointerTo();
        t_int64_p_ = t_int64_->getPointerTo();
        t_bool_p_ =  t_bool_->getPointerTo();
        t_double_p_ =  t_double_->getPointerTo();

        t_int_vec_ = llvm::VectorType::get( t_int_, lanes_);
        t_int64_vec_ = llvm::VectorType::get( t_int64_, lanes_ );
        t_bool_vec_ = llvm::VectorType::get( t_bool_, lanes_ );
        t_double_vec_ = llvm::VectorType::get( t_double_, lanes_ );
        t_double_vec4_ = llvm::VectorType::get(t_double_,VECTOR4);
        t_int_vec4_ = llvm::VectorType::get(t_int_, VECTOR4);

        t_double_vec4_p_ = t_double_vec4_->getPointerTo();
        t_int_vec4_p_ = t_int_vec4_->getPointerTo();

	    t_int_vec_p_ = t_int_vec_->getPointerTo();
	    t_double_vec_p_ = t_double_vec_->getPointerTo();

    	t_int_ptr_vec_ = llvm::VectorType::get(t_int_p_,lanes_);
    	t_double_ptr_vec_ = llvm::VectorType::get(t_double_p_,lanes_);

    	t_double_ptr_vec4_ = llvm::VectorType::get(t_double_p_,VECTOR4);
        
        Zero_ = llvm::ConstantInt::get( t_int_ , 0);

        ZeroVec_ = llvm::ConstantVector::getSplat( lanes_, Zero_);
        FZero_ = llvm::ConstantFP::get( t_float_ , 0);

        FZeroVec_ = llvm::ConstantVector::getSplat( lanes_, FZero_);
        DZero_ = llvm::ConstantFP::get( t_double_ , 0);

        DZeroVec_ = llvm::ConstantVector::getSplat( lanes_, DZero_);

        One_ = llvm::ConstantInt::get( t_int_ , 1);
        //FOne_ = llvm::ConstantFP::get( t_double_ , 1);
        True_ = llvm::ConstantInt::get( t_bool_ , 1);
        Null_ = llvm::Constant::getNullValue( t_int_ );
    }

    std::map< Varience*,Value*> var_val_map_;
    Value* CodeGen_(StateMent * stat ) {
        LOG(FATAL) << "the statement " << stat->get_class_name() \
                    << " does not support";
        return Null_;
    }
    Value * CodeGen_(Block * stat  ) {
        StateMent * stat1 = stat->get_stat1();
        StateMent * stat2 = stat->get_stat2();
        CodeGen(stat1 );
        CodeGen(stat2);
        return Null_;
    }
    Value * CodeGen_(Print * stat) {
        Varience * var = stat->get_var();
        Type & type = var->get_type();
        Value * value = CodeGen_(var);
        if( type == __int ) {
        
            LLVMPrintInt( mod_ptr_.get(), ctx_ptr_.get(),build_ptr_.get(), value );
        } else if( type == __int_ptr ) {
            LLVMPrintPtr( mod_ptr_.get(), ctx_ptr_.get(), build_ptr_.get(), value, 1 );
        } else {
            LOG(FATAL) << "Unsupport Type ";
        }
        return Null_;
    }
    AllocaInst *CreateEntryBlockAlloca( llvm::Type * type,Function *TheFunction,
                                          const std::string &VarName) {
        llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
        return TmpB.CreateAlloca( type, nullptr,
                           VarName.c_str());
    }

    Value * CodeGen_( For * stat ) {
        Varience * var = stat->get_var();
        const string& var_name = var->get_name();
        Function * f = build_ptr_->GetInsertBlock()->getParent();
        
        AllocaInst * Alloca = CreateEntryBlockAlloca(t_int_,f,var_name);
        Value * stat_val = CodeGen( stat->get_begin());
        if( !stat_val ) {
            return nullptr;
        }
        ///store the value into the alloca
        build_ptr_->CreateStore( stat_val, Alloca );
        ////////
        BasicBlock * loop_BB = BasicBlock::Create(*ctx_ptr_, "loop",f);
        build_ptr_->CreateBr(loop_BB);
        build_ptr_->SetInsertPoint(loop_BB);
        /*ctx_ptr_ = std::make_unique<llvm::LLVMContext>();
        mod_ptr_= std::make_unique<llvm::Module>("module",*ctx_ptr_);
        build_ptr_ = std::make_unique<llvm::IRBuilder<>>(*ctx_ptr_);*/


        var_val_map_[var] = Alloca;

        var_mutable_set_.insert(var);

        if( !CodeGen( stat->get_stat() ) ) {
            return nullptr;
        }
        Value * step_val = nullptr;
        StateMent * step = stat->get_space();
        if(step) {
            step_val = CodeGen( step );
            if(!step_val) {
                return nullptr;
            }
        } else {
            step_val = One_;
        }
        Value * end_cond = CodeGen( stat->get_end() );
        if( !end_cond ) {
            return nullptr;
        }
        Value * cur_var = build_ptr_->CreateLoad( Alloca, var_name );

        Value * NextVar = build_ptr_->CreateAdd( cur_var, step_val,"nextvar");

        build_ptr_->CreateStore(NextVar,Alloca);
        end_cond = build_ptr_->CreateICmpSLT( NextVar,end_cond ,"loopcond");     
        BasicBlock * after_BB = BasicBlock::Create(*ctx_ptr_,"afterloop",f);
        build_ptr_->CreateCondBr(end_cond, loop_BB,after_BB);
        build_ptr_->SetInsertPoint(after_BB);
        return Null_;
    }
    template<typename T> 
    void init_vec( llvm::Value * &vec, const void * data_arg, const int lanes,llvm::Type* llvm_type ) {

        const T * vec_ptr = reinterpret_cast<const T*>(data_arg);
        if(lanes == 1) {
            if(typeid(T)==typeid(double)) {
                vec = llvm::ConstantFP::get(llvm_type,vec_ptr[0]);
            } else {
                vec = llvm::ConstantInt::get(llvm_type,vec_ptr[0]);
            }
        } else {
        for( int i = 0 ; i < lanes; i++ ) {
            llvm::Value * index = llvm::ConstantInt::get(t_int_, i  );
            llvm::Value * data;
            if(typeid(T)==typeid(double))
                data = llvm::ConstantFP::get( llvm_type ,vec_ptr[i]);
            else if( typeid(T) == typeid(int) ) {
                data = llvm::ConstantInt::get(llvm_type,vec_ptr[i]);
            } else if( typeid(T)==typeid(bool)) {
                data = llvm::ConstantInt::get(llvm_type,vec_ptr[i]);
            }
            
	        vec = build_ptr_->CreateInsertElement( vec , data , index);
        }
        }
    }

    Value* CodeGen_( Const* stat) {
        Type & stat_type = stat->get_type();
        llvm::Type * type = Type2LLVMType( stat->get_type() );
        void * data = stat->get_data();
        llvm::Value * vec = llvm::UndefValue::get( type );
        const int lanes = stat_type.get_lanes();
        if( stat_type.get_data_type() == DOUBLE ) {
            init_vec<double>( vec , data,lanes,t_double_);
        } else if(stat_type.get_data_type() == INT) {
            init_vec<int>(vec,data, lanes,t_int_); 
        } else if( stat_type.get_data_type() == BOOL ){
            init_vec<bool>(vec,data,lanes,t_bool_);
        } else {
        
            LOG(FATAL) << "does not support";
        }
        return vec;
    }
    Value * CodeGen_( LetStat * stat ) {

        Value * value = CodeGen( stat->get_expr() );
        Varience * var = stat->get_res();
        bool is_const = stat->get_is_const();

        auto var_val_map_value = var_val_map_.find( var );
        if( is_const ) {
            if(var_val_map_value == var_val_map_.end() ) {
                var_val_map_[var] = value;
            } else {
                LOG(FATAL) << var->get_name() << "has already been defined";
            }
        } else {
            if( var_val_map_value == var_val_map_.end() ) {

                Function * f = build_ptr_->GetInsertBlock()->getParent();
                AllocaInst * alloc = CreateEntryBlockAlloca( Type2LLVMType(var->get_type()) , f,
                                                var->get_name()) ;
                var_mutable_set_.insert(var);
                var_val_map_[var] = alloc;
            
                build_ptr_->CreateStore( value , alloc );
            } else {

                build_ptr_->CreateStore( value , var_val_map_value->second );
            }
        }
        return Null_; 
    }

    Value * CodeGen_( IncAddr * stat ) {
        llvm::Value * addr_value = CodeGen( stat->get_addr() );
        llvm::Value * inc_value = CodeGen( stat->get_inc() );


//        return build_ptr_->CreateInBoundsGEP( addr_value, inc_value);

        Value * ret = build_ptr_->CreateInBoundsGEP( addr_value, inc_value );
        return ret;
    }
    Value * CodeGen_(Varience * stat) {
        CHECK(stat!=NULL) << "stat should not be null";
        auto var_val_map_find = var_val_map_.find(stat);
        Value * ret_var;
        if(var_val_map_find == var_val_map_.end()) {
            auto arg_val_map_find = arg_val_map_.find(stat);
            if( arg_val_map_find != arg_val_map_.end() ) {
                ret_var = arg_val_map_find->second;
            } else {
            
                LOG(FATAL) << "can not find var " << *stat;
            }
        } else {
            if( var_mutable_set_.find(stat) != var_mutable_set_.end() )
                ret_var = build_ptr_->CreateLoad( var_val_map_find->second, stat->get_name() );
            else 
                ret_var = var_val_map_find->second;
        }
        return ret_var;
    }
    Value * CodeGen_(Nop * stat) {
        return Null_;
    }
    Value * CodeGen_(Scatter * stat) {
        Value * addr_value = CodeGen( stat->get_addr());
        Value * index_value = CodeGen( stat->get_index());
        Value * data_value = CodeGen(stat->get_data());
        Value * mask_value = CodeGen( stat->get_mask() );
        Value * ptr_value = build_ptr_->CreateInBoundsGEP( addr_value, index_value);
        build_ptr_->CreateMaskedScatter(data_value,ptr_value,alinements_,mask_value);
        return Null_;
    }
    Value * CodeGen_(Init * stat) {
        return Null_;
    }
    Value * CodeGen_(Gather * stat) {

        Value * addr_value = CodeGen( stat->get_addr());

        Value * index_value = CodeGen( stat->get_index());

        Value * ptr_value = build_ptr_->CreateInBoundsGEP( addr_value, index_value);
        
        Value * mask_value = CodeGen( stat->get_mask() );
        if( stat->get_type() == __double_v ) {         
            return  build_ptr_->CreateMaskedGather(  ptr_value , alinements_, mask_value , DZeroVec_);
        } else if( stat->get_type() == __float_v ) {
        
            return  build_ptr_->CreateMaskedGather(  ptr_value , alinements_, mask_value , FZeroVec_);
        } else if( stat->get_type() == __int_v ) { 
            return  build_ptr_->CreateMaskedGather(  ptr_value , alinements_, mask_value , ZeroVec_);
        } else {
            LOG(FATAL ) << "Unsupported type";
            return Null_; 
        }
    }
    Value * CodeGen_(Load * stat) {

        Value * addr_value = CodeGen( stat->get_addr());


        Value * ret = build_ptr_->CreateAlignedLoad( addr_value , alinements_, false);

//        Value * ret = build_ptr_->CreateLoad( addr_value );
        return ret;
    }
    Value * CodeGen_(Store * stat) {

        Value * addr_value = CodeGen( stat->get_addr());
        Value * data_value = CodeGen( stat->get_data());
        return build_ptr_->CreateAlignedStore( data_value, addr_value, alinements_, false);
    }
    Value * CodeGen_(Shuffle * stat) {
        Value * v1_value = CodeGen(stat->get_v1());

        Value * v2_value = CodeGen(stat->get_v2());

        Value * index_value = CodeGen(stat->get_index());
        return build_ptr_->CreateShuffleVector( v1_value, v2_value, index_value);
    }
    Value * CodeGen_(Reduce * stat) {
        StateMent * v1 = stat->get_v1();
        Value * v1_value = CodeGen( v1);

        if(v1->get_type().get_data_type() == DOUBLE) {
            llvm::Value * Acc = llvm::UndefValue::get( t_double_ );
            llvm::FastMathFlags FMFFast;
            FMFFast.setFast();
            llvm::CallInst* ret = build_ptr_->CreateFAddReduce(Acc,v1_value);
            ret->setFastMathFlags(FMFFast);
            return ret;
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateAddReduce(v1_value);
        } else {
            LOG(FATAL) << "the type does not support";
            return Null_;
        }
    }
    Value * CodeGen_(BroadCast* stat) {
        const Type & type = stat->get_type();
        Value * value = CodeGen( stat->get_v1()); 
        const int lanes = type.get_lanes();
        llvm::Constant* undef = llvm::UndefValue::get( llvm::VectorType::get(value->getType(), lanes) );
        value = build_ptr_->CreateInsertElement(undef, value, Zero_);
        return build_ptr_->CreateShuffleVector(value, undef, ZeroVec_);
    }
    Value * CodeGen_(BitCast * stat) {

        StateMent * v1 = stat->get_v1();

        Value * v1_value = CodeGen( v1);
        Type & type = stat->get_type();
        llvm::Type* llvmtype = Type2LLVMType(type);
        const Type & v1_type = v1->get_type();
        const Type & stat_type = stat->get_type();
        if( (v1_type == __int && (stat_type == __double|| stat_type == __float )) ||( v1_type == __int_v ) && ( stat_type == __double_v || stat_type == __float_v )  ) {
            return build_ptr_->CreateSIToFP(v1_value, llvmtype);
        } else {
        
            return build_ptr_->CreateBitCast(v1_value, llvmtype);
        }
    }
    Value * CodeGen_(Binary * stat) {
        LOG(FATAL) << "please specify the binary operation";
        return Null_;
    }
    Value * CodeGen_(Add * stat) {
        StateMent * v1 = stat->get_v1();
        Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFAdd( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateAdd( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }

        return Null_;
    }
     Value * CodeGen_(Minus * stat) {
        StateMent * v1 = stat->get_v1();
        Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFSub( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateSub( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }
        return Null_;
    }
    Value * CodeGen_(Div * stat) {
        StateMent * v1 = stat->get_v1();
        Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFDiv( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateUDiv( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }
        return Null_;
    }

    Value * CodeGen_(Mul * stat) {
        StateMent * v1 = stat->get_v1();
        Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFMul( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateMul( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }
        return Null_;
    }
    Value* CodeGen( StateMent * stat ) {
        static FType * ftype_ptr = nullptr;
        if(ftype_ptr == nullptr) {
            ftype_ptr = new FType();
            SET_DISPATCH( StateMent );
            SET_DISPATCH( Block );
            SET_DISPATCH( For );

            SET_DISPATCH( Const );

            SET_DISPATCH( Varience );
            SET_DISPATCH( LetStat );
            SET_DISPATCH(IncAddr);
            SET_DISPATCH(Nop);
            SET_DISPATCH(Scatter);
            SET_DISPATCH(Init);
            SET_DISPATCH(Gather);
            SET_DISPATCH( Load );
            SET_DISPATCH(Store);
            SET_DISPATCH(Shuffle);
            SET_DISPATCH( Reduce );
            SET_DISPATCH(BitCast);
            SET_DISPATCH(Binary);
            SET_DISPATCH(Add);
            SET_DISPATCH(Mul);

            SET_DISPATCH(Div);
            SET_DISPATCH(Minus);
            SET_DISPATCH(BroadCast);

            SET_DISPATCH(Print);
        }
        return (*ftype_ptr)(stat);
    }
    void AddFunction( FuncStatement * func_state ) {
        ////init args type
        StateMent * state_ptr = func_state->get_state();
        CHECK(state_ptr!=NULL) << "state_ptr is null";

        std::vector<llvm::Type*> llvm_args_type;
        const std::vector<Varience*> * args = 
            func_state->get_args();
        const int args_len = args->size();
        llvm_args_type.resize( args_len, nullptr );
        for( int i = 0 ; i < args_len ; i++ ) {
            llvm_args_type[i] = Type2LLVMType( (*args)[i]->get_type());
        }
        Varience * ret = func_state->get_ret();
        
        StateMent * state = func_state->get_state();
        llvm::Type * llvm_ret_type = Type2LLVMType( ret->get_type());
        llvm::FunctionType* ftype = llvm::FunctionType::get( llvm_ret_type , llvm_args_type, false );

        llvm::Function*  function = llvm::Function::Create( ftype, llvm::GlobalValue::ExternalLinkage, func_state->get_func_name() , mod_ptr_.get());

        for( int arg_i = 0 ; arg_i < args_len ; arg_i++ ) {
            arg_val_map_[ (*args)[arg_i] ] = &function->arg_begin()[arg_i];
        }

        llvm::BasicBlock * entry = llvm::BasicBlock::Create(*ctx_ptr_,"entry",function );
        build_ptr_->SetInsertPoint( entry );
        CodeGen( state_ptr);
        build_ptr_->CreateRet(One_);
    	TheFPM->run( *function );
    }
    void PrintModule() {
        LLVMLOG(INFO) << *mod_ptr_;
    }
};

