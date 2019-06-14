#include "llvm_codegen.hpp"
    llvm::Type * LLVMCodeGen::Type2LLVMType(const Type & type) {
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
        } else if( type == __int8 ) { 
            return t_int8_;
        } else if( type == __int8_ptr ) {
            return t_int8_ptr_;
        } else if(type == __int8_v) {
            return t_int8_vec_;
        } else if( type == __int8_v_ptr ) { 
            return t_int8_vec_ptr_;
        } else {
            LOG(FATAL) << type <<"type does not support ";
            return t_int_;
        }
    }

LLVMCodeGen::LLVMCodeGen() {

        ctx_ptr_ = llvm::make_unique<llvm::LLVMContext>();
        mod_ptr_= llvm::make_unique<llvm::Module>("module",*ctx_ptr_);

        conflict_512_ = llvm::Intrinsic::getDeclaration( mod_ptr_.get(), llvm::Intrinsic::x86_avx512_mask_conflict_d_512);

        permvar_int_512_ = llvm::Intrinsic::getDeclaration( mod_ptr_.get() , llvm::Intrinsic::x86_avx512_permvar_si_512);

        permvar_float_512_ = llvm::Intrinsic::getDeclaration(mod_ptr_.get(), llvm::Intrinsic::x86_avx512_permvar_sf_512);
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

        t_int16_ = llvm::Type::getInt16Ty( *ctx_ptr_ ); 
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
        t_int8_ptr_ = t_int8_->getPointerTo();
        t_int64_p_ = t_int64_->getPointerTo();
        t_bool_p_ =  t_bool_->getPointerTo();
        t_double_p_ =  t_double_->getPointerTo();

        t_int_vec_ = llvm::VectorType::get( t_int_, lanes_);
        t_int64_vec_ = llvm::VectorType::get( t_int64_, lanes_ );
        t_bool_vec_ = llvm::VectorType::get( t_bool_, lanes_ );

	    t_int8_vec_ = llvm::VectorType::get( t_int8_, lanes_ );
        t_double_vec_ = llvm::VectorType::get( t_double_, lanes_ );
        t_double_vec4_ = llvm::VectorType::get(t_double_,VECTOR4);
        t_int_vec4_ = llvm::VectorType::get(t_int_, VECTOR4);

        t_double_vec4_p_ = t_double_vec4_->getPointerTo();
        t_int_vec4_p_ = t_int_vec4_->getPointerTo();

	    t_int8_vec_ptr_ = t_int8_vec_->getPointerTo();
        t_int_vec_p_ = t_int_vec_->getPointerTo();
	    t_double_vec_p_ = t_double_vec_->getPointerTo();

    	t_int_ptr_vec_ = llvm::VectorType::get(t_int_p_,lanes_);
    	t_double_ptr_vec_ = llvm::VectorType::get(t_double_p_,lanes_);

    	t_double_ptr_vec4_ = llvm::VectorType::get(t_double_p_,VECTOR4);
        
        Zero_ = llvm::ConstantInt::get( t_int_ , 0);

        SixTeen_ = llvm::ConstantInt::get( t_int_ , 16);

        ZeroVec_ = llvm::ConstantVector::getSplat( lanes_, Zero_);

        SixTeenVec_ = llvm::ConstantVector::getSplat( lanes_, SixTeen_);
        FZero_ = llvm::ConstantFP::get( t_float_ , 0);

        FZeroVec_ = llvm::ConstantVector::getSplat( lanes_, FZero_);
        DZero_ = llvm::ConstantFP::get( t_double_ , 0);

        DZeroVec_ = llvm::ConstantVector::getSplat( lanes_, DZero_);

        FFFF_ = llvm::ConstantInt::get( t_int16_ , 0xffff);
        One_ = llvm::ConstantInt::get( t_int_ , 1);
        //FOne_ = llvm::ConstantFP::get( t_double_ , 1);
        True_ = llvm::ConstantInt::get( t_bool_ , 1);
        for( int i = 0 ; i < VECTOR ; i++ ) {
            CONST_INDEX_NUM_[i ] = llvm::ConstantInt::get( t_int_ , i );
        }
        Null_ = llvm::Constant::getNullValue( t_int_ );
}
    llvm::Value* LLVMCodeGen::CodeGen_(StateMent * stat ) {
        LOG(FATAL) << "the statement " << stat->get_class_name() \
                    << " does not support";
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Block * stat  ) {
    std::vector<StateMent* > * state_vec_ptr = stat->get_stat_vec();
    for(int i = 0 ; i < state_vec_ptr->size() ; i++) {
        CodeGen((*state_vec_ptr)[i]);
    }
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Print * stat) {
        Varience * var = stat->get_var();
        Type & type = var->get_type();
        llvm::Value * value = CodeGen_(var);
        if( type == __int ) {
        
            LLVMPrintInt( mod_ptr_.get(), ctx_ptr_.get(),build_ptr_.get(), value );
        } else if( type == __int_ptr ) {
            LLVMPrintPtr( mod_ptr_.get(), ctx_ptr_.get(), build_ptr_.get(), value, 1 );
        } else {
            LOG(FATAL) << "Unsupport Type ";
        }
        return Null_;
    }
llvm::AllocaInst *CreateEntryBlockAlloca( llvm::Type * type,llvm::Function *TheFunction,
                                          const std::string &VarName) {
        llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
        return TmpB.CreateAlloca( type, nullptr,
                           VarName.c_str());
    }

llvm::Value * LLVMCodeGen::CodeGen_( For * stat ) {
        Varience * var = stat->get_var();
        const std::string& var_name = var->get_name();
        llvm::Function * f = build_ptr_->GetInsertBlock()->getParent();
        
        llvm::BasicBlock * after_BB = llvm::BasicBlock::Create(*ctx_ptr_,"afterloop",f);

        llvm::BasicBlock * loop_BB = llvm::BasicBlock::Create(*ctx_ptr_, "loop",f);
        llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(t_int_,f,var_name);

        llvm::Value * stat_val = CodeGen( stat->get_begin());
        if( !stat_val ) {
            return nullptr;
        }

        ///store the value into the alloca
        /*ctx_ptr_ = std::make_unique<llvm::LLVMContext>();
        mod_ptr_= std::make_unique<llvm::Module>("module",*ctx_ptr_);
        build_ptr_ = std::make_unique<llvm::IRBuilder<>>(*ctx_ptr_);*/
        llvm::Value * end_value = CodeGen( stat->get_end() );
        if( !end_value ) {
            return nullptr;
        }

        build_ptr_->CreateStore( stat_val, Alloca );
        llvm::Value * end_cond = build_ptr_->CreateICmpSLT( stat_val,end_value,"loopcond");     
        build_ptr_->CreateCondBr(end_cond, loop_BB,after_BB);

        ////////
        build_ptr_->SetInsertPoint(loop_BB);


        var_val_map_[var] = Alloca;

        var_mutable_set_.insert(var);

        if( !CodeGen( stat->get_stat() ) ) {
            return nullptr;
        }
        llvm::Value * step_val = nullptr;
        StateMent * step = stat->get_space();
        if(step) {
            step_val = CodeGen( step );
            if(!step_val) {
                return nullptr;
            }
        } else {
            step_val = One_;
        }
        llvm::Value * cur_var = build_ptr_->CreateLoad( Alloca, var_name );

        llvm::Value * NextVar = build_ptr_->CreateAdd( cur_var, step_val,"nextvar");

        build_ptr_->CreateStore(NextVar,Alloca);
        end_cond = build_ptr_->CreateICmpSLT( NextVar,end_value ,"loopcond");     
        build_ptr_->CreateCondBr(end_cond, loop_BB,after_BB);
        build_ptr_->SetInsertPoint(after_BB);
        return Null_;
    }
    template<typename T> 
    void LLVMCodeGen::init_vec( llvm::Value * &vec, const void * data_arg, const int lanes,llvm::Type* llvm_type ) {

        const T * vec_ptr = reinterpret_cast<const T*>(data_arg);
        if(lanes == 1) {
            if(typeid(T)==typeid(double)||typeid(T)==typeid(float) ) {
                vec = llvm::ConstantFP::get(llvm_type,vec_ptr[0]);
            } else if( typeid(T)==typeid(int) || typeid(T)==typeid(bool) ) {
                vec = llvm::ConstantInt::get(llvm_type,vec_ptr[0]);
            } else {
                LOG(FATAL) << "Unsupported Type";
            }
        } else {
        for( int i = 0 ; i < lanes; i++ ) {
            llvm::Value * index = llvm::ConstantInt::get(t_int_, i  );
            llvm::Value * data;
            if(typeid(T)==typeid(double) || typeid(T)==typeid(float) )
                data = llvm::ConstantFP::get( llvm_type ,vec_ptr[i]);
            else if( typeid(T) == typeid(int) ) {
                data = llvm::ConstantInt::get(llvm_type,vec_ptr[i]);
            } else if( typeid(T)==typeid(bool)) {
                data = llvm::ConstantInt::get(llvm_type,vec_ptr[i]);
            } else {
                LOG(FATAL) << "Unsupported Type";
            }
            
	        vec = build_ptr_->CreateInsertElement( vec , data , index);
        }
        }
    }

llvm::Value* LLVMCodeGen::CodeGen_( Const* stat) {
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
llvm::Value * LLVMCodeGen::CodeGen_( LetStat * stat ) {

    llvm::Value * value = CodeGen( stat->get_expr() );
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

                llvm::Function * f = build_ptr_->GetInsertBlock()->getParent();
                llvm::AllocaInst * alloc = CreateEntryBlockAlloca( Type2LLVMType(var->get_type()) , f,
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

llvm::Value * LLVMCodeGen::CodeGen_( IncAddr * stat ) {
        llvm::Value * addr_value = CodeGen( stat->get_addr() );
        llvm::Value * inc_value = CodeGen( stat->get_inc() );


//        return build_ptr_->CreateInBoundsGEP( addr_value, inc_value);

        llvm::Value * ret = build_ptr_->CreateInBoundsGEP( addr_value, inc_value );
        return ret;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Varience * stat) {
        CHECK(stat!=NULL) << "stat should not be null";
        auto var_val_map_find = var_val_map_.find(stat);
        llvm::Value * ret_var;
        if(var_val_map_find == var_val_map_.end()) {
            auto arg_val_map_find = arg_val_map_.find(stat);
            if( arg_val_map_find != arg_val_map_.end() ) {
                ret_var = arg_val_map_find->second;
            } else {
            
                LOG(FATAL) << "can not find var " << *stat ;
            }
        } else {
            if( var_mutable_set_.find(stat) != var_mutable_set_.end() )
                ret_var = build_ptr_->CreateLoad( var_val_map_find->second, stat->get_name() );
            else 
                ret_var = var_val_map_find->second;
        }
        return ret_var;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Nop * stat) {
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Scatter * stat) {
        llvm::Value * addr_value = CodeGen( stat->get_addr());
        llvm::Value * index_value = CodeGen( stat->get_index());
        llvm::Value * data_value = CodeGen(stat->get_data());
        llvm::Value * mask_value = CodeGen( stat->get_mask() );
        llvm::Value * ptr_value = build_ptr_->CreateInBoundsGEP( addr_value, index_value);
        build_ptr_->CreateMaskedScatter(data_value,ptr_value,alinements_,mask_value);
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Init * stat) {
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Gather * stat) {

    llvm::Value * addr_value = CodeGen( stat->get_addr());

    llvm::Value * index_value = CodeGen( stat->get_index());

    llvm::Value * ptr_value = build_ptr_->CreateInBoundsGEP( addr_value, index_value);
        
    llvm::Value * mask_value = CodeGen( stat->get_mask() );
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
llvm::Value * LLVMCodeGen::CodeGen_(Load * stat) {

    llvm::Value * addr_value = CodeGen( stat->get_addr());


    llvm::Value * ret = build_ptr_->CreateAlignedLoad( addr_value , alinements_, false);

//        Value * ret = build_ptr_->CreateLoad( addr_value );
        return ret;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Store * stat) {

    llvm::Value * addr_value = CodeGen( stat->get_addr());
    llvm::Value * data_value = CodeGen( stat->get_data());
        return build_ptr_->CreateAlignedStore( data_value, addr_value, alinements_, false);
    }
llvm::Value * LLVMCodeGen::CodeGen_(Shuffle * stat) {
    llvm::Value * v1_value = CodeGen(stat->get_v1());

    llvm::Value * v2_value = CodeGen(stat->get_v2());

    llvm::Value * index_value = CodeGen(stat->get_index());
        return build_ptr_->CreateShuffleVector( v1_value, v2_value, index_value);
    }
llvm::Value * LLVMCodeGen::CodeGen_(Reduce * stat) {
        StateMent * v1 = stat->get_v1();
        llvm::Value * v1_value = CodeGen( v1);

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
    llvm::Value * LLVMCodeGen::LLVMBroadCast( llvm::Value * value, const int lanes) {
         llvm::Constant* undef = llvm::UndefValue::get( llvm::VectorType::get(value->getType(), lanes) );
        value = build_ptr_->CreateInsertElement(undef, value, Zero_);
        return build_ptr_->CreateShuffleVector(value, undef, ZeroVec_);
   
    }
llvm::Value * LLVMCodeGen::CodeGen_(BroadCast* stat) {
        const Type & type = stat->get_type();
        llvm::Value * value = CodeGen( stat->get_v1()); 
        const int lanes = type.get_lanes();
        return LLVMBroadCast( value, lanes );
    }
llvm::Value * LLVMCodeGen::CodeGen_(BitCast * stat) {

        StateMent * v1 = stat->get_v1();

        llvm::Value * v1_value = CodeGen( v1);
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
llvm::Value * LLVMCodeGen::CodeGen_(Binary * stat) {
        LOG(FATAL) << "please specify the binary operation";
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Add * stat) {
        StateMent * v1 = stat->get_v1();
        llvm::Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        llvm::Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFAdd( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateAdd( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }

        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Minus * stat) {
        StateMent * v1 = stat->get_v1();
        llvm::Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        llvm::Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFSub( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateSub( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_(Div * stat) {
        StateMent * v1 = stat->get_v1();
        llvm::Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        llvm::Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFDiv( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateUDiv( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }
        return Null_;
    }

llvm::Value * LLVMCodeGen::CodeGen_(Mul * stat) {
        StateMent * v1 = stat->get_v1();
        llvm::Value * v1_value = CodeGen( v1);
        StateMent * v2 = stat->get_v2();
        llvm::Value * v2_value = CodeGen( v2);
        if(v1->get_type().get_data_type() == DOUBLE || v1->get_type().get_data_type() == FLOAT ) {

            return build_ptr_->CreateFMul( v1_value , v2_value);
            
        } else if(v1->get_type().get_data_type() == INT) {
            return build_ptr_->CreateMul( v1_value , v2_value);
        } else {
            LOG(FATAL) << "the type does not support";
        }
        return Null_;
    }
llvm::Value * LLVMCodeGen::CodeGen_( ComplexReduce * stat) {
    llvm::Value * v1 = CodeGen( stat->get_v1() );
    const Type & type = stat->get_v1()->get_type();
    const int lanes = type.get_lanes();
    llvm::Value * index = CodeGen( stat->get_index() );
    int mask = stat->get_mask();
    if(VECTOR == VECTOR16) {
        llvm::Value * shuffle_index_ptr = CodeGen(stat->get_shuffle_index_ptr());
        for( int i = 0 ; i < mask ; i++ ) {
            llvm::Value * addr = build_ptr_->CreateInBoundsGEP( shuffle_index_ptr , CONST_INDEX_NUM_[i] );
            llvm::Value * index = build_ptr_->CreateLoad(  addr);
            llvm::Value * index_ext = build_ptr_->CreateZExtOrBitCast( index, t_int_vec_);
            std::vector<llvm::Value*> args;
            args.push_back(v1);
            args.push_back(index_ext);
            llvm::Value * shuffle_data = build_ptr_->CreateCall( permvar_float_512_ ,args );

            llvm::Value * equal_sixteen = build_ptr_->CreateICmpEQ( index_ext, SixTeenVec_);
            shuffle_data = build_ptr_->CreateSelect( equal_sixteen, FZeroVec_, shuffle_data );
            v1 = build_ptr_->CreateFAdd( shuffle_data, v1 );

        }
/*    #define VREDUCE_ELEM(NUM) \
        llvm::Value * tmp = build_ptr_->CreateExtractElement( index ,CONST_INDEX_NUM_[(NUM)]); \
        llvm::Value * index_vec = LLVMBroadCast( tmp, lanes ); \
        llvm::Value * index_mask = build_ptr_->CreateICmpEQ( index_vec, index );        \
        llvm::Value * value_select = build_ptr_->CreateSelect( index_mask, v1, FZeroVec_ );\
        llvm::Value * Acc = llvm::UndefValue::get( t_double_ );\
        llvm::FastMathFlags FMFFast;\
        FMFFast.setFast(); \
        llvm::CallInst * reduce_value = build_ptr_->CreateFAddReduce( Acc,value_select); \
        reduce_value->setFastMathFlags(FMFFast);\
        llvm::Value * broad_cast_reduce_value = LLVMBroadCast( reduce_value, lanes);\
        v1 = build_ptr_->CreateSelect( index_mask, broad_cast_reduce_value,v1 );
//        v1 = build_ptr_->CreateInsertElement( v1 , reduce_value , CONST_INDEX_NUM_[(NUM)]);

    if( mask != 0x0 ) {
        if( (mask&0x2) ) {
            VREDUCE_ELEM( 1 );
        }
        if( ( mask&0x4 ) ) {
            VREDUCE_ELEM(2);
        }
        if( ( mask&0x8 ) ) {
            VREDUCE_ELEM(3);
        }
        if( ( mask&0x10 ) ) {
            VREDUCE_ELEM(4);
        }
        if( ( mask&0x20 ) ) {
            VREDUCE_ELEM(5);
        }
        if( ( mask&0x40 ) ) {
            VREDUCE_ELEM(6);
        }
        if( ( mask&0x80 ) ) {
            VREDUCE_ELEM(7);
        }
        if( ( mask&0x100 ) ) {
            VREDUCE_ELEM(8);
        }
        if( ( mask&0x200 ) ) {
            VREDUCE_ELEM(9);
        }
        if( ( mask&0x400 ) ) {
            VREDUCE_ELEM(10);
        }
        if( ( mask&0x800 ) ) {
            VREDUCE_ELEM(11);
        }
        if( ( mask&0x1000 ) ) {
            VREDUCE_ELEM(12);
        }
        if( ( mask&0x2000 ) ) {
            VREDUCE_ELEM(13);
        }
        if( ( mask&0x4000 ) ) {
            VREDUCE_ELEM(14);
        }
        if( ( mask&0x8000 ) ) {
            VREDUCE_ELEM(15);
        }
    }*/
        return v1;
    } else {
        LOG(FATAL) <<"Unsupported";
    }
    return Null_;
}

llvm::Value * LLVMCodeGen::CodeGen_( DetectConflict * stat) {
     StateMent * index = stat->get_index();
     llvm::Value * index_value = CodeGen( index );
     std::vector<llvm::Value *> values;

     values.push_back( index_value );
     values.push_back( ZeroVec_ );

     values.push_back( FFFF_ );
     llvm::Value * conf512 = build_ptr_->CreateCall( conflict_512_, values);
     
     llvm::Value * ret = build_ptr_->CreateICmpEQ( conf512, ZeroVec_);
     return ret;

}

llvm::Value* LLVMCodeGen::CodeGen( StateMent * stat ) {

        using FType = ir_func<llvm::Value*(StateMent*)>; 
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

            SET_DISPATCH(ComplexReduce);
            SET_DISPATCH(DetectConflict);
            SET_DISPATCH(Print);
        }
        return (*ftype_ptr)(stat);
    }
    void LLVMCodeGen::AddFunction( FuncStatement * func_state ) {
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
