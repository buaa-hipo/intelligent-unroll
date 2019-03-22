/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#define OMEGA 4
#define DELTA 4
#include "llvm/IR/InstrTypes.h"

#include "llvm/IR/Verifier.h"
#include <cstdlib>
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"

#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DIBuilder.h>


/////////////////////////////
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#define DATATYPE double
typedef int(*FP)(DATATYPE *,DATATYPE*);

#define CHECK(x) if(!(x)) \
            llvm::outs() << "CHECK failed:"#x << " "
#define PRINTINT(x) do {    \
                    printf( #x" %d\n" , (x));fflush(stdout); \
                        } while(0)
//using namespace llvm;
    void print_fvec( DATATYPE * ptr , const int num ) {
        for( int i = 0 ; i < num ; i++)
            printf("%f ",ptr[i]);
        printf("\n");
        fflush(stdout);
    }
    void print_vec( int * ptr , const int num ) {
        for( int i = 0 ; i < num ; i++)
            printf("%d ",ptr[i]);
        printf("\n");
        fflush(stdout);
    }
#define PRINTINTVEC( x,len)  do{\
    printf(#x" "#len" \n");\
    print_vec(x,len); } while(0)

#define PRINTDOUBLEVEC( x,len)  do{\
    printf(#x" "#len" \n");\
    print_fvec(x,len); } while(0)

class CSR5JIT {
    llvm::LLVMContext * ctx_ptr_;
    llvm::Module* mod_ptr_;
    llvm::IRBuilder<>* builder_;

    LLVMExecutionEngineRef engine_;
    int lanes_;
    int omega_;
    const int alinements_ = 256;
    llvm::Type*  t_int_;
    llvm::Type*  t_int64_ ;
    llvm::Type*  t_bool_ ;
    llvm::Type*  t_double_;

    llvm::Type*  t_int_p_ ;
    llvm::Type*  t_int64_p_;
    llvm::Type*  t_bool_p_;
    llvm::Type*  t_double_p_;

    llvm::Type* t_bool_vec_;
    llvm::Type* t_int_vec_;
    llvm::Type* t_int64_vec_;
    llvm::Type*  t_double_vec_;

    llvm::Type* t_int_vec_p_;
    llvm::Type*  t_double_vec_p_;

    llvm::Type* t_int_ptr_vec_ ;
    llvm::Type*  t_double_ptr_vec_;

    llvm::Constant *Zero_ ;

    llvm::Constant *FZero_ ;
    llvm::Constant *One_ ;
    llvm::Constant *True_;


    llvm::Constant *FOne_ ;
    llvm::Constant* ZeroVec_;

    llvm::Constant* FZeroVec_;
    llvm::Constant* TrueVec_;
    llvm::Argument * y_ptr_;
    llvm::Argument * x_ptr_;

    llvm::Value * x_vec_ptr_;
    llvm::Value * y_vec_ptr_;
    llvm::Value * x_ptr_vec_;
    llvm::Value * y_ptr_vec_;
    int row_num_;
    char *error_;
    public:
    llvm::Module * get_mod_ptr() {
        return mod_ptr_;
    }
    CSR5JIT(const int lanes,const int omega) {
        error_ = NULL;
	    ctx_ptr_ = new llvm::LLVMContext();
	    lanes_ = lanes;
        omega_ = omega;
        t_int_ =  llvm::Type::getInt32Ty(*ctx_ptr_);
        t_int64_ =  llvm::Type::getInt64Ty(*ctx_ptr_);
        t_bool_ =  llvm::Type::getInt1Ty(*ctx_ptr_);
        t_double_ =  llvm::Type::getDoubleTy(*ctx_ptr_);

        t_int_p_ = t_int_->getPointerTo();
        t_int64_p_ = t_int64_->getPointerTo();
        t_bool_p_ =  t_bool_->getPointerTo();
        t_double_p_ =  t_double_->getPointerTo();

        t_int_vec_ = llvm::VectorType::get( t_int_, lanes_ );
        t_int64_vec_ = llvm::VectorType::get( t_int64_, lanes_ );
        t_bool_vec_ = llvm::VectorType::get( t_bool_, lanes_ );
        t_double_vec_ = llvm::VectorType::get( t_double_, lanes_ );

	    t_int_vec_p_ = t_int_vec_->getPointerTo();
	    t_double_vec_p_ = t_double_vec_->getPointerTo();

    	t_int_ptr_vec_ = llvm::VectorType::get(t_int_p_,lanes_);
    	t_double_ptr_vec_ = llvm::VectorType::get(t_double_p_,lanes_);

        Zero_ = llvm::ConstantInt::get( t_int_ , 0);

        FZero_ = llvm::ConstantFP::get( t_double_ , 0);
        One_ = llvm::ConstantInt::get( t_int_ , 1);
        FOne_ = llvm::ConstantFP::get( t_double_ , 1);
        True_ = llvm::ConstantInt::get( t_bool_ , 1);
        ZeroVec_ = llvm::ConstantVector::getSplat( lanes_, Zero_);

        FZeroVec_ = llvm::ConstantVector::getSplat( lanes_, FZero_);
        TrueVec_ = llvm::ConstantVector::getSplat( lanes_, True_ );
        mod_ptr_ = new llvm::Module("my_module", *ctx_ptr_ );

        builder_ = new llvm::IRBuilder<>(*ctx_ptr_);
    }
    llvm::Value* CreateBufferPtr( llvm::Type* t, llvm::Value* buffer, llvm::Value* index) {
   	    llvm::PointerType* btype = llvm::dyn_cast<llvm::PointerType>(buffer->getType());
	    if(btype == nullptr) {
        	fprintf(stderr, "error");
	    }
	    llvm::PointerType* ptype = t->getPointerTo(btype->getAddressSpace());
    	 if (btype != ptype) {
      	    buffer = builder_->CreatePointerCast(buffer, ptype);
    	 }

         return builder_->CreateInBoundsGEP(buffer, index);
    }
    llvm::Constant * ConstInt32(const int value) {
        return llvm::ConstantInt::get( t_int_ , value);
    }
    llvm::CallInst * FReduce( llvm::Value * src ) {

        llvm::Value * Acc = llvm::UndefValue::get( t_double_ );
        llvm::FastMathFlags FMFFast;
        FMFFast.setFast();
        llvm::CallInst * ret = builder_->CreateFAddReduce(Acc,src);

        if(!ret->getType()->isDoubleTy()) {
            llvm::errs() << "is double\n" << *mod_ptr_ << "\n";
        }
        ret->setFastMathFlags(FMFFast);
        return ret;
    }

    llvm::CallInst * Reduce( llvm::Value * src ) {
        return builder_->CreateAddReduce(src);
    }
    llvm::Value* Broadcast(llvm::Value* value) {
        llvm::Constant* undef = llvm::UndefValue::get( llvm::VectorType::get(value->getType(), lanes_) );
        value = builder_->CreateInsertElement(undef, value, Zero_);
        return builder_->CreateShuffleVector(value, undef, ZeroVec_);
    }

    llvm::Value * BitCast( llvm::Value * value , llvm::Type * type ) {
        return builder_->CreateBitCast(value, type);
    }
    llvm::Value * InitFVector( DATATYPE vec[] ) {
        llvm::Value * Vec = llvm::UndefValue::get( t_double_vec_ );

        for( int i = 0 ; i < lanes_ ; i++ ) {
            llvm::Value * index = llvm::ConstantInt::get(t_int_, i  );
            llvm::Value * data = llvm::ConstantFP::get(t_double_,vec[i]);
	        Vec = builder_->CreateInsertElement( Vec , data , index);
        }
        return Vec;
    }
    llvm::Value * InitBoolVector( int mask ) {
        llvm::Value * Vec = llvm::UndefValue::get( t_bool_vec_ );

        for( int i = 0 ; i < lanes_ ; i++ ) {
            llvm::Value * index = llvm::ConstantInt::get(t_int_, i  );
            llvm::Value * data;
            if( (mask >> i ) & 0x1 )
                data = llvm::ConstantInt::get(t_bool_,true);
            else
                data = llvm::ConstantInt::get(t_bool_,false);
	        Vec = builder_->CreateInsertElement( Vec , data , index);
        }
        return Vec;
    }

    llvm::Value * InitVector( int vec[] ) {
        llvm::Value * Vec = llvm::UndefValue::get( t_int_vec_ );

        for( int i = 0 ; i < lanes_ ; i++ ) {
            llvm::Value * index = llvm::ConstantInt::get(t_int_, i  );
            llvm::Value * data = llvm::ConstantInt::get(t_int_,vec[i]);
	        Vec = builder_->CreateInsertElement( Vec , data , index);
        }
        return Vec;
    }
    llvm::Value * IncAddr( llvm::Value * PtrVec , llvm::Value * Vec ) {
        return builder_->CreateInBoundsGEP( PtrVec, Vec);
    }
    llvm::CallInst * FGather( llvm::Value * Ptr, llvm::Value * Mask) {
        return  builder_->CreateMaskedGather(  Ptr , alinements_,Mask , FZeroVec_);
    }

    llvm::CallInst * Gather( llvm::Value * Ptr, llvm::Value * Mask) {
        return  builder_->CreateMaskedGather(  Ptr , alinements_,Mask , ZeroVec_);
    }
    llvm::StoreInst * Store(llvm::Value * addr, llvm::Value * value) {

        return builder_->CreateAlignedStore( value, addr, alinements_, false);
    }
    llvm::Value* ExtractElement( llvm::Value* vec,const int index ) {
        llvm::Value * index_value = llvm::ConstantInt::get(t_int_, index  );
        return builder_->CreateExtractElement( vec , index_value );
    }
    llvm::Value* LoadOffset( llvm::Value * ptr ,const int index ) {
        llvm::Value * index_value = llvm::ConstantInt::get(t_int_, index  );
        return Load(IncAddr(ptr,index_value));
    }

    void StoreOffset( llvm::Value * ptr ,const int index ,llvm::Value* value) {
        llvm::Value * index_value = llvm::ConstantInt::get(t_int_, index  );
        Store(IncAddr(ptr,index_value), value);
    }
    llvm::Value *  LoadWithMask(int * y_offset, int * empty_offset,bool is_empty ,int write_flag, int tile_row_begin ) {
            llvm::Value * ret;
            int y_offset_add_tile_begin[ DELTA ];
            if(is_empty) {
                for (int i = 0; i < DELTA ; i++ ) {
                    y_offset_add_tile_begin[i] = empty_offset[ y_offset[i] ] + tile_row_begin;
                }
            } else {
               for (int i = 0; i < DELTA ; i++ ) {
                    y_offset_add_tile_begin[i] = y_offset[i] + tile_row_begin;
                }
            }
            llvm::Value* ptr_vec = IncAddr(y_ptr_vec_ ,InitVector(y_offset_add_tile_begin));

            ret = FGather( ptr_vec ,  InitBoolVector(write_flag) );
            return ret;
    }


    void StoreWithMask(llvm::Value* simd ,int * y_offset, int * empty_offset,bool is_empty ,int write_flag, int tile_row_begin ) {
        switch( write_flag ) {
            case 0x1:
                if(is_empty)
                    StoreOffset(y_ptr_, tile_row_begin + empty_offset[ y_offset[0]],ExtractElement(simd,0));
                else
                    StoreOffset(y_ptr_, tile_row_begin + y_offset[0],ExtractElement(simd,0));
                y_offset[0]++;
                break;
            case 0x2:
                if(is_empty)
                    StoreOffset(y_ptr_, tile_row_begin + empty_offset[ y_offset[1] ],ExtractElement(simd,1));
                else
                    StoreOffset(y_ptr_, tile_row_begin + y_offset[1] ,ExtractElement(simd,1));
                y_offset[1]++;
                break;
            case 0x4:
                if(is_empty)
                    StoreOffset(y_ptr_, tile_row_begin + empty_offset[ y_offset[2] ],ExtractElement(simd,2));
                else
                    StoreOffset(y_ptr_, tile_row_begin + y_offset[2],ExtractElement(simd,2));
                y_offset[2]++;
                break;
            case 0x8:
                if(is_empty)
                    StoreOffset(y_ptr_, tile_row_begin + empty_offset[ y_offset[3] ],ExtractElement(simd,3));
                else
                    StoreOffset(y_ptr_, tile_row_begin + y_offset[3],ExtractElement(simd,3));
                y_offset[3]++;
                break;
            default:
                int y_offset_add_tile_begin[ DELTA ];
                if(is_empty) {
                    for (int i = 0; i < DELTA ; i++ ) {
                        y_offset_add_tile_begin[i] = empty_offset[ y_offset[i] ] + tile_row_begin;
                    }
                } else {
                    for (int i = 0; i < DELTA ; i++ ) {
                        y_offset_add_tile_begin[i] = y_offset[i] + tile_row_begin;
                    }
                }
                llvm::Value* ptr_vec = IncAddr(y_ptr_vec_ ,InitVector(y_offset_add_tile_begin));

                SCATTER( ptr_vec , simd , InitBoolVector(write_flag) );
                for( int i = 0 ; i < DELTA ; i++  )  {
                    y_offset[i] += (0x1 & write_flag>>i);
                }
                break;
        }
    }

    llvm::Value * Mul( llvm::Value * x , llvm::Value * y ) {
        return builder_->CreateMul( x , y);
    }
    llvm::Value * FMul( llvm::Value * x , llvm::Value * y ) {
        return builder_->CreateFMul( x , y);
    }
    llvm::Value * FAdd( llvm::Value * x , llvm::Value * y ) {
        return builder_->CreateFAdd( x , y);
    }

    llvm::Value * Add( llvm::Value * x , llvm::Value * y ) {
        return builder_->CreateAdd( x , y);
    }
    llvm::LoadInst* Load( llvm::Value * addr ) {
        return builder_->CreateAlignedLoad( addr , alinements_, false);
    }
    void Return( llvm::Value * ret) {
        builder_->CreateRet(ret);
    }
    llvm::Value* spmv_mul(  llvm::Value * x_addr_vec, llvm::Value * data) {
        llvm::Value * gather_value = FGather( x_addr_vec, TrueVec_ );
        llvm::Value * mult_value = FMul( gather_value, data );
        return mult_value;
    }

    llvm::Value* spmv_reduce( llvm::Value* y_ptr, llvm::Value * x_addr_vec, llvm::Value * data_vec_ptr) {

        llvm::Value * mult_value = spmv_mul( x_addr_vec , data_vec_ptr);
        llvm::Value * ret_value = FReduce( mult_value );
        CHECK( ret_value->getType()->isDoubleTy()) << "ret value type is not double type\n";
        return ret_value;
    }
    void SCATTER( llvm::Value* ptr_vec, llvm::Value * data_vec, llvm::Value * mask_vec) {
        builder_->CreateMaskedScatter(data_vec,ptr_vec,alinements_,mask_vec);
    }
    llvm::Value* ShuffleWithMask( llvm::Value* first,llvm::Value* second,int mask) {
        int index[DELTA];
        for( int i = 0 ; i < DELTA ; i++) {
           index[i] =  i + DELTA * ( (mask >> i) & (0x1 ) );
        }
        return builder_->CreateShuffleVector( first, second,InitVector(index));
    }

    llvm::Value* Shuffle( llvm::Value* first,llvm::Value* second,int *index) {

        return builder_->CreateShuffleVector( first, second,InitVector(index));
    }
/*    llvm::Function * CreateFunc(  ) {
    }
    std::vector<llvm::Type*> get_args_type(  ) {

    }*/
    typedef struct tile_dec {
        int bit_flag_[ OMEGA ];
        int y_offset_[ DELTA ];
        int * empty_offset_;
        bool is_empty_;
    } tile_dec;
    tile_dec generate_tile_dec(const int * row_ptr, const int begin_loc, int & tile_row_index) {
        tile_dec info;
//        const int * row_ptr = &row_ptr_base[tile_row_index];
        for( int i = 0 ; i < OMEGA ; i++ ) {
//            for( int j = 0 ; j < DELTA ; j++) {
                info.bit_flag_[i] = 0;
//            }
        }
        for( int j = 0 ; j < DELTA ; j++ ) {
            info.y_offset_[j] = 0;
        }
        int temp_y_offset[DELTA];
        for( int j = 0 ; j < DELTA ; j++ ) {
            temp_y_offset[j] = 0 ;
        }

        info.bit_flag_[0] = 1;
        temp_y_offset[0] ++;
        int row_index = 0;
        bool is_empty = false;
        int distance;
        while( row_index + 1 + tile_row_index < row_num_ && row_ptr[row_index] == row_ptr[row_index + 1]) {row_index++;}
        
        distance = row_ptr[row_index+1] - begin_loc;

        std::vector<int> empty_offset_vec;
        CHECK(distance > 0) << "Erro\n";
        
        while( distance < OMEGA * DELTA ) {
//            info.bit_flag_[ distance / OMEGA + (distance % OMEGA) * DELTA ] = 1;
            PRINTINT(distance);
            info.bit_flag_[ distance % OMEGA ] |= 1 << ( distance / OMEGA);
            temp_y_offset[ distance / OMEGA ] ++;

            empty_offset_vec.push_back(row_index);
            row_index++;

            PRINTINT( row_index + tile_row_index );

            PRINTINT( row_num_ );
            if(row_index + tile_row_index >= row_num_ ) {

                PRINTINT( row_index + tile_row_index );
                break;
            }
            while(row_index + 1 + tile_row_index < row_num_ &&  row_ptr[row_index] == row_ptr[row_index+1] ) {
                row_index++;
                is_empty = true;
            }
            
            distance = row_ptr[row_index+1] - begin_loc;
        }
        PRINTINT( row_index );
//        if(distance != OMEGA * DELTA)
//            row_index--;
        info.is_empty_ = is_empty;
        if(is_empty) {
            int empty_size = empty_offset_vec.size();
            info.empty_offset_ = (int*)malloc(sizeof(int) * empty_size);
            for (int i = 0 ; i < empty_size; i++ ) {
                info.empty_offset_[i] = empty_offset_vec[i];
            }
        }
        for( int j = 1 ; j < DELTA ; j++ ) {
            info.y_offset_[j] = info.y_offset_[j-1] + temp_y_offset[j-1];
        }
        PRINTINTVEC( info.y_offset_, DELTA );

        PRINTINTVEC( info.bit_flag_, OMEGA );
        tile_row_index += row_index;
        return info;
    }

    int spmv_func_unit( const DATATYPE  * data_ptr, const int * row_ptr, const int * column_ptr , const int begin_loc, const int  row_index ) {

        int ret_row_index = row_index;
        const int tile_row_index = row_index;
        const int * tile_column_ptr = column_ptr;
        const DATATYPE * tile_data_ptr = data_ptr;
        //DATATYPE * tile_y_ptr = y_ptr + begin_loc;

        tile_dec ret_tile_dec = generate_tile_dec( row_ptr, begin_loc,ret_row_index);
        int * y_offset = ret_tile_dec.y_offset_;
        int * empty_offset = ret_tile_dec.empty_offset_;
        int column_tran[OMEGA* DELTA];
        DATATYPE data_tran[ OMEGA* DELTA];

        for( int j = 0 ; j < omega_ ; j++) {
            for( int i = 0 ; i < lanes_ ; i++ ) {
               column_tran[j* lanes_ + i ] = tile_column_ptr[j + i * omega_]  ;
               data_tran[j*lanes_ + i] =  tile_data_ptr[j + i * omega_];
            }
        }

        llvm::Value * index = InitVector( column_tran);
        llvm::Value * data = InitFVector( data_tran);
        llvm::Value * addr = IncAddr(x_ptr_vec_,index);

        llvm::Value* simd =  spmv_mul( addr, data );
        llvm::Value* old_simd = FZeroVec_;
        int old_simd_flag = 0;
        int write_flag = 0;
        old_simd_flag |= ret_tile_dec.bit_flag_[0];
        for( int i = 1 ; i < omega_ ; i++ ) {
            index = InitVector( &column_tran[i*lanes_]);
            addr = IncAddr(x_ptr_vec_ , index );
            data = InitFVector( &data_tran[i*lanes_] );

            //process old simd
            int cur_old_simd_flag = ret_tile_dec.bit_flag_[i] & (~old_simd_flag);
            if( cur_old_simd_flag != 0x0 ) {
                old_simd = ShuffleWithMask( old_simd, simd ,cur_old_simd_flag);
            }

           //process memory write
            write_flag = ret_tile_dec.bit_flag_[i] & old_simd_flag;
            if( write_flag != 0x0) {
                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , write_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , write_flag,  tile_row_index );
            }
            if( ret_tile_dec.bit_flag_[i] != 0x0 ) {
            simd = ShuffleWithMask( simd , FZeroVec_ , ret_tile_dec.bit_flag_[i]);
            }
            old_simd_flag |= cur_old_simd_flag; //update old_simd_flag
            llvm::Value* tmp = spmv_mul( addr , data );
            simd =  FAdd( simd , tmp );
        }

        int shift_1[] = {1,2,3,4};
        llvm::Value * old_simd_shift_1 = Shuffle( old_simd, FZeroVec_,shift_1 );
        simd = FAdd(old_simd_shift_1,simd);
        llvm::Value * simd_tmp1;
        llvm::Value * simd_tmp2;
        
        int index_tmp[] = {4,4,4,4};
        const int base = 0x1;
        switch(old_simd_flag) {
            case base:  //1000
                simd_tmp1 = FReduce(simd);
                simd_tmp2 = LoadOffset( y_ptr_, ret_tile_dec.y_offset_[0]  + tile_row_index);
                StoreOffset( y_ptr_ ,ret_tile_dec.y_offset_[0] + tile_row_index , FAdd( simd_tmp2, simd_tmp1));

                break;
            case base + 0x8:  //1001
                index_tmp[0] = 1; //1 4 4 4
                simd_tmp1 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp1,simd);
                index_tmp[0] = 2; //2 4 4 4
                simd_tmp2 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp2,simd);
                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));

                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            case base + 0x4:  //1010
                index_tmp[0] = 1;
                index_tmp[2] = 3; //1 4 3 4
                simd_tmp1 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp1,simd);

                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            case base + 0x2:  //1100
                index_tmp[1] = 2 ; // 4,2,4,4
                simd_tmp1 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp1,simd);
                index_tmp[1] = 3 ; //4,3,4,4;
                simd_tmp2 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp2,simd);

                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            ////////////
            case base + 0xc:  //1011
                index_tmp[0] = 1;// {1,4,4,4};
                simd_tmp1 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp1,simd);

                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            case base + 0xb:  //1101
                index_tmp[1] = 2;// = {4,2,4,4};
                simd_tmp1 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp1,simd);

                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            case base + 0x6:  //1110
                index_tmp[2] = 3; // {4,4,4,3};
                simd_tmp1 = Shuffle(simd,FZeroVec_,index_tmp);
                simd = FAdd(simd_tmp1,simd);

                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            ///////////
            case base + 0xe:  //1111
                
                simd = FAdd( simd , LoadWithMask( ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_ , ret_tile_dec.is_empty_ , old_simd_flag, tile_row_index ));
                StoreWithMask( simd , ret_tile_dec.y_offset_, ret_tile_dec.empty_offset_, ret_tile_dec.is_empty_ , old_simd_flag,  tile_row_index );
                break;
            default:
                llvm::errs() << "unKnown case " << old_simd_flag << "\n";
        }

        return ret_row_index;
    }

    void spmv_func(const DATATYPE  * data_ptr,const int * column_ptr,const int * row_ptr, const int row_num) {


        std::vector<llvm::Type*> arg_type;
        arg_type.push_back(t_double_p_);
        arg_type.push_back(t_double_p_);
        row_num_ = row_num;
        llvm::FunctionType* ftype = llvm::FunctionType::get( t_int_ , arg_type, false );
        llvm::Function*  sum = llvm::Function::Create( ftype, llvm::GlobalValue::ExternalLinkage, "sum", mod_ptr_);


        llvm::BasicBlock * entry  = llvm::BasicBlock::Create(*ctx_ptr_, "entry", sum);
        builder_->SetInsertPoint(entry);
        y_ptr_ = & sum->arg_begin()[0];
        x_ptr_ = & sum->arg_begin()[1];

        x_vec_ptr_ = BitCast( x_ptr_ , t_double_vec_p_ );
        y_vec_ptr_ = BitCast( y_ptr_ , t_double_vec_p_ );
        x_ptr_vec_ = Broadcast( x_ptr_ );
        y_ptr_vec_ = Broadcast( y_ptr_ );

        int row_index = 0;
        int data_num = row_ptr[row_num];
        int begin_loc;
        for (begin_loc = 0 ; begin_loc < data_num - DELTA * OMEGA + 1 ; begin_loc += DELTA * OMEGA ) {
            row_index = spmv_func_unit( data_ptr + begin_loc, row_ptr + row_index, column_ptr + begin_loc , begin_loc, row_index );
            PRINTINT(row_index);
        }
        //handle the special case
        //
        
        int rem_data = data_num % (DELTA * OMEGA);
        if(rem_data > 0) {
            DATATYPE data_fill[ DELTA * OMEGA ];
            int column_fill[DELTA * OMEGA];
            int *row_fill = (int*) malloc(sizeof(int)*(row_num - row_index+1));
            for( int i = 0 ; i < rem_data ; i++  ) {
                data_fill[i] = data_ptr[ begin_loc + i ];
                column_fill[i] = column_ptr[ begin_loc + i ];
            }
            for( int i = rem_data ; i < DELTA * OMEGA ; i++  ) {
                data_fill[i] = 0;
                column_fill[i] = column_ptr[ begin_loc + rem_data - 1 ];
            }
            for( int i = row_index ; i < row_num_ ; i++ ) {
                row_fill[i-row_index] = row_ptr[i];
            }
            row_fill[ row_num_ - row_index ] = data_num - rem_data + DELTA * OMEGA;
            row_index = spmv_func_unit( data_fill, row_fill, column_fill , begin_loc, row_index );
        }
//        SCATTER( addr,res ,TrueVec_ );
//        res = FReduce(res);
//        Store( y_ptr,res);
        Return( Zero_ );
        LLVMVerifyModule(wrap(mod_ptr_), LLVMAbortProcessAction, &error_);
        LLVMDisposeMessage(error_);

        return;
    }
    void compiler_jit() {
        error_ = NULL;
        LLVMLinkInMCJIT();
        LLVMInitializeNativeTarget();
        LLVMInitializeNativeAsmPrinter();
        if (LLVMCreateExecutionEngineForModule(&engine_, wrap(mod_ptr_), &error_) != 0) {
            fprintf(stderr, "failed to create execution engine\n");
            abort();
        }
        if (error_) {
            fprintf(stderr, "error: %s\n", error_);
            LLVMDisposeMessage(error_);
            exit(EXIT_FAILURE);
        }
    }

    // Write out bitcode to file
    void write_bit() {
        if (LLVMWriteBitcodeToFile(wrap(mod_ptr_), "sum.bc") != 0) {
            fprintf(stderr, "error writing bitcode to file, skipping\n");
        }
        return;
    }
    ~CSR5JIT() {
        LLVMDisposeBuilder(wrap(builder_));
        LLVMDisposeExecutionEngine(engine_);
    }

    FP get_function() {
        return (FP)LLVMGetFunctionAddress(engine_, "sum");
    }

};
void spvm_local(DATATYPE * y_ptr,const DATATYPE * x_ptr,const DATATYPE * data_ptr, const int * column_ptr, const int * row_ptr, const int row_num ) {
    for (int i = 0 ; i < row_num ; i++ ) {
        DATATYPE sum = 0;
        for (int j = row_ptr[i]; j < row_ptr[i+1] ; j++ ) {
            sum += x_ptr[column_ptr[j]] * data_ptr[j];
        }
        y_ptr[i] = sum;
    }
}
//#define DEBUG
int main(int argc, char const *argv[]) {


    const int row_num = 8;
    CSR5JIT * csr5_jit_ptr = new CSR5JIT(4,4);
    int column_array[] = { 2,3,0,1, 2,3,4,5, 6,1,3,4, 5,1,3,4 ,
                           6,7,0,3, 6,0,3,4, 6,7,2,0,1,2, 3,4,6,7, 
                           };
    int row_array[] = {0 , 2 , 9, 13, 18, 21, 26, 27, 34 };

    DATATYPE A_array[34];
    for( int i = 0 ; i < 34; i++ ) {
#ifdef DEBUG
        A_array[i] = 1;
#else
        A_array[i] = i;
#endif
    }

    DATATYPE x_array[8];
    DATATYPE y_array[8];
    DATATYPE y_array_bak[8];
    for( int i = 0 ; i < 8 ; i++ ) {
#ifdef DEBUG
        x_array[i] = 1;
#else
        x_array[i] = i;
#endif
        y_array[i] = 0;
        y_array_bak[i] = 0;
    }

    csr5_jit_ptr->spmv_func( A_array, column_array, row_array,row_num);
    csr5_jit_ptr->compiler_jit();
    csr5_jit_ptr->write_bit();

    llvm::errs() << "We just constructed this LLVM module:\n\n---------\n" << *csr5_jit_ptr->get_mod_ptr();
    fflush(stdout);

    FP spvm_func = csr5_jit_ptr->get_function();
    printf("%d\n", spvm_func(y_array,x_array));
    spvm_local( y_array_bak, x_array, A_array , column_array, row_array, row_num ) ;

    print_fvec( y_array , row_num  );
    print_fvec( y_array_bak, row_num);
    printf("\n");

}
