    #include "util.h"
#include "llvm_lib/llvm_common.h"
typedef unsigned char BIT_FLAG_TYPE;
typedef int Y_OFFSET_TYPE;
enum TileType { FAST , SWITCH, STORE , NOP};
#define TileTypeNum 3
ostream & operator<<( ostream & out, TileType & tile_type ) {
    const string tile_type_str[] = { "FAST", "SWITCH","STORE" };
    return out << tile_type_str[ tile_type  ];
}
class TileDec {
    public:
    BIT_FLAG_TYPE * bit_flag_;
    Y_OFFSET_TYPE * y_offset_;
    int * empty_offset_;
    bool is_empty_;
    TileType tile_type_;    
    TileType get_type() {
        return tile_type_;
    }
    void init_type( const int delta) {
        BIT_FLAG_TYPE old_mask = 0 ; 
        BIT_FLAG_TYPE write_mask = 0 ;
        old_mask = bit_flag_[0];
        for( int i = 1 ; i < delta ; i++ ) {
            write_mask = bit_flag_[i] & old_mask;
            if( write_mask ) {
                tile_type_ = STORE;
                return;
            }
            old_mask |= bit_flag_[i];
        }

        if( old_mask == 0x1 ) {
            tile_type_ = FAST;
        } else {
            tile_type_ = SWITCH;
        }
    }
};
typedef TileDec *TileDecPtr;
class AnalyzeCSR5{
    TileDecPtr tile_dec_ptr_;
    const int delta_;
    const int omega_;
    int * tile_row_index_ptr_;
    int tile_dec_num_;

    int row_num_;
    private:

    void generate_tile_dec(const int * row_ptr, const int begin_loc, int & tile_row_index, TileDecPtr info ) {
//        const int * row_ptr = &row_ptr_base[tile_row_index];
        for( int i = 0 ; i < delta_ ; i++ ) {
                info->bit_flag_[i] = 0;
        }
        for( int j = 0 ; j < omega_ ; j++ ) {
            info->y_offset_[j] = 0;
        }
        int temp_y_offset[omega_];
        for( int j = 0 ; j < omega_ ; j++ ) {
            temp_y_offset[j] = 0 ;
        }

        info->bit_flag_[0] = 1;
        temp_y_offset[0] ++;
        int row_index = 0;
        bool is_empty = false;
        int distance;
        while( row_index + 1 + tile_row_index < row_num_ && row_ptr[row_index] == row_ptr[row_index + 1]) {row_index++;}
        
        distance = row_ptr[row_index+1] - begin_loc;

        std::vector<int> empty_offset_vec;
        CHECK(distance >= 0) << "Erro\n";
        
        while( distance < omega_ * delta_ ) {
            info->bit_flag_[ distance % delta_ ] |= 1 << ( distance / delta_);
            temp_y_offset[ distance / delta_ ] ++;

            empty_offset_vec.push_back(row_index);
            row_index++;


            if(row_index + tile_row_index >= row_num_ ) {
                break;
            }
            while(row_index + 1 + tile_row_index < row_num_ &&  row_ptr[row_index] == row_ptr[row_index+1] ) {
                row_index++;
                is_empty = true;
            }
            
            distance = row_ptr[row_index+1] - begin_loc;
        }
        empty_offset_vec.push_back(row_index);
        if( distance == omega_ * delta_ ) {
            row_index++;
        }
        info->is_empty_ = is_empty;
        if(is_empty) {
            int empty_size = empty_offset_vec.size();
            info->empty_offset_ = (int*)malloc(sizeof(int) * empty_size);
            for (int i = 0 ; i < empty_size; i++ ) {
                info->empty_offset_[i] = empty_offset_vec[i];
            }
        }
        for( int j = 1 ; j < omega_ ; j++ ) {
            info->y_offset_[j] = info->y_offset_[j-1] + temp_y_offset[j-1];
        }
        if(is_empty) {
//            PRINTINTVEC( info.empty_offset_ , empty_offset_vec.size() );
        }
//        PRINTINTVEC( info.y_offset_, DELTA );
        tile_row_index += row_index;
    }


    public:
    AnalyzeCSR5( const int omega, const int delta ): omega_(omega),delta_(delta) {
    }
    int * get_tile_row_index_ptr(){
        return tile_row_index_ptr_;
    };
    int get_tile_dec_num() {
        return tile_dec_num_;
    };

    TileDecPtr get_tile_dec_ptr() {
        return tile_dec_ptr_;
    };

    void analyze( const csrSparseMatrixPtr csr_sparse_matrix_ptr  ) {

        const int data_num = csr_sparse_matrix_ptr->data_num;
        const int row_num = csr_sparse_matrix_ptr->row_num;
        const int * row_ptr = csr_sparse_matrix_ptr->row_ptr;
        if( data_num <=0 ) {
            LOG(FATAL) << "data_num can not less than zero\n";
            exit(1);
        }
        row_num_ = row_num;
        tile_dec_num_ = (data_num - 1) / ( delta_ * omega_ ) + 1;
        tile_dec_ptr_ = SIMPLE_MALLOC( TileDec , tile_dec_num_ );
        BIT_FLAG_TYPE * bit_flag_all = SIMPLE_MALLOC( BIT_FLAG_TYPE , delta_ * tile_dec_num_ );
        Y_OFFSET_TYPE * y_offset_all = SIMPLE_MALLOC( Y_OFFSET_TYPE, omega_ * tile_dec_num_ );
        tile_row_index_ptr_ = SIMPLE_MALLOC( int , tile_dec_num_ );  
        for( int i = 0 ; i < tile_dec_num_ ; i++ ) {
            tile_dec_ptr_[i].bit_flag_ = bit_flag_all + i * delta_;
            tile_dec_ptr_[i].y_offset_ = y_offset_all + i * omega_;
        }
        int row_index = 0;
        for( int i = 0 ; i < tile_dec_num_ ; i++  ) {
            tile_row_index_ptr_[i] = row_index;
            generate_tile_dec( &row_ptr[row_index] , i * delta_ * omega_ , row_index , tile_dec_ptr_ + i );
            tile_dec_ptr_[i].init_type( delta_ );
            
        }

    }
    void count() {
        int type_num = 1 ;
        int count_type[3] = {0};

        TileType old_type = tile_dec_ptr_[0].get_type();
        for( int i = 1 ; i < tile_dec_num_ ; i++ ) {
            TileType cur_type = tile_dec_ptr_[i].get_type();
            if( old_type == cur_type ) {
                type_num++;
            } else {
                std::cout << old_type << " " << type_num << std::endl;
                old_type = cur_type;
                count_type[ old_type ]++;
                type_num = 1;
            }
        }
        std::cout << old_type << " " << type_num << "\n";
        count_type[ old_type ]++;
        std::cout<< FAST << " " <<count_type[FAST]<<std::endl;

        std::cout<< SWITCH << " " <<count_type[SWITCH]<<std::endl;
        std::cout<< STORE << " "  << count_type[STORE] << std::endl;
    }
};

class CSR5SuperBlock {
    public:
    TileType tiletype_order_[TileTypeNum];
    int num_[TileTypeNum];
    public:
    TileType * getTileTypeOrder() {
        return  tiletype_order_;
    }
    TileType  getTileTypeOrder(int index)const {
        return  tiletype_order_[index];
    }
    int  get_num(int index)const {
        return num_[index];
    }

    int * get_num() {
        return num_;
    }

    CSR5SuperBlock( TileType * tiletype_order, int *num ) {
        for( int i = 0 ; i < TileTypeNum; i++ ) {
            num_[i] = num[i];
            tiletype_order_[i] = tiletype_order[i];
        }
    }
    inline bool operator <( const CSR5SuperBlock & b ) const {
        bool ret = false;
        for( int i = 0 ; i < TileTypeNum; i++ ) {
            if(this->tiletype_order_[i] < b.getTileTypeOrder(i)) {
                ret = true;
                break;
            }
            if(this->num_[i] < b.get_num(i) ) {
                ret = true;
                break;
            }
        }
        return ret;
    }

    inline bool operator ==( const CSR5SuperBlock & b ) const {
        bool ret = true;
        for( int i = 0 ; i < TileTypeNum; i++ ) {
            if(this->num_[i] != b.num_[i] || this->tiletype_order_[i] != b.tiletype_order_[i]) {
                ret = false ;
                break;
            }
        }
        return ret;
    }
    friend std::ostream& operator<<( std::ostream &os, const CSR5SuperBlock & csr5_super_block ) {
        os << "CSR5SuperBlock : \n";
        for( int i = 0 ; i < TileTypeNum; i++ ) {
            os << csr5_super_block.tiletype_order_[i];
            os << " : ";
            os << csr5_super_block.num_[i];
            os << "\n";
        }

        return os; 
    }
};

class CSR5SuperBlockSet{
    std::vector<CSR5SuperBlock*> CSR5SuperBlockVec_;
    TileDecPtr tile_dec_ptr_;
    int tile_dec_num_;
    public:
    CSR5SuperBlockSet( TileDecPtr tile_dec_ptr, int tile_dec_num, int * tile_row_index_ptr ) : tile_dec_ptr_( tile_dec_ptr ), tile_dec_num_( tile_dec_num ) {
        int type_index = 0;
        TileType tiletype_order[TileTypeNum];
        int num[TileTypeNum];

        for( int i = 0 ; i < TileTypeNum ; i++ ) {
            tiletype_order[i] = NOP;
            num[ i ] = 0; 
        }
        
        for( int tile_i = 0,next_tile_i = 0 ; tile_i < tile_dec_num_ ; tile_i = next_tile_i ) {
            TileType old_type = tile_dec_ptr_[tile_i].get_type();
            TileType cur_type = tile_dec_ptr_[tile_i].get_type();
            int old_row = tile_row_index_ptr[ tile_i ] ;
           
            next_tile_i = tile_i;

            while( next_tile_i < tile_dec_num && cur_type == old_type && ( old_type != FAST || (old_type == FAST && old_row == tile_row_index_ptr[ next_tile_i ] ) ) ) {
                next_tile_i++;
                cur_type = tile_dec_ptr[next_tile_i].get_type();
            }
            bool if_duplicate = false;
            for( int type_i = 0 ; type_i < type_index ; type_i++ ) {
                if( tiletype_order[ type_i] == old_type ) {
                    if_duplicate = true;
                    break;
                }
            }
            if( if_duplicate ) {
                CSR5SuperBlock * csr5_super_block_ptr = new CSR5SuperBlock( tiletype_order, num );
                CSR5SuperBlockVec_.push_back( csr5_super_block_ptr );
                for( int i = 0 ; i < TileTypeNum ; i++ ) {
                    tiletype_order[i] = NOP;
                    num[ i ] = 0; 
                }
                type_index = 0;
            } 
            tiletype_order[type_index] = old_type;
            num[type_index] = next_tile_i - tile_i;
            type_index++;
            if( type_index == TileTypeNum - 1 ) {
                CSR5SuperBlock * csr5_super_block_ptr = new CSR5SuperBlock( tiletype_order, num );
                CSR5SuperBlockVec_.push_back( csr5_super_block_ptr );
                for( int i = 0 ; i < TileTypeNum ; i++ ) {
                    tiletype_order[i] = NOP;
                    num[ i ] = 0; 
                }
                type_index = 0;
            } 
        }
        if(type_index != 0) {
            CSR5SuperBlock * csr5_super_block_ptr = new CSR5SuperBlock( tiletype_order, num );
            CSR5SuperBlockVec_.push_back( csr5_super_block_ptr );
        }
    }

    std::vector<CSR5SuperBlock*>& GetCSR5SuperBlockVec() {
        return CSR5SuperBlockVec_;
    }
    void Analyse(  ) {
        std::map<CSR5SuperBlock, int> CSR5SuperBlockCount;
        for( auto &it : CSR5SuperBlockVec_ ) {
             if( CSR5SuperBlockCount.find( *it ) == CSR5SuperBlockCount.end() ) {
                CSR5SuperBlockCount[ *it ] = 0;
             } else {
                CSR5SuperBlockCount[*it]++;
             }
        }
        for( auto it : CSR5SuperBlockCount ) {
            std::cout<< (it.first) << " : " << it.second;
        } 
    } 
};



