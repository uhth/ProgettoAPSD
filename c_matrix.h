#ifndef C_MATRIX_H
#define C_MATRIX_H

#include<cstdlib>

template<class T>
class CMatrix {

public:

	CMatrix( int dimY, int dimX, int a = 0 ) {
		this->dimY = dimY;
		this->dimX = dimX;
		this->size = dimX * dimY;
		this->linearMatrix = new T[ size ];
		for( int i = 0; i < size; i++ ) 
			if ( linearMatrix[ i ] != a ) linearMatrix[ i ] = a;
	}
	
	CMatrix( CMatrix &m ) {
		this->dimY = m->dimY;
		this->dimX = m->dimX;
		this->size = this->dimX * this->dimY;
		this->linearMatrix = new T[ size ];
		for( int i = 0; i < size; i++ ) 
			if ( linearMatrix[ i ] != m->linearMatrix[ i ] ) linearMatrix[ i ] = m->linearMatrix[ i ];
	}
	
	CMatrix( T* lm, int dimY, int dimX ) {
		this->dimY = dimY;
		this->dimX = dimX;
		this->size = this->dimX * this->dimY;
		this->linearMatrix = new T[ size ];
		for( int i = 0; i < size; i++ ) 
			if ( linearMatrix[ i ] != lm[ i ] ) linearMatrix[ i ] = lm[ i ];
	}
		
	~CMatrix() { 
		if( linearMatrix != nullptr ) delete[] linearMatrix; 
	}
	
	T* at( int index ) { return &linearMatrix[ index ]; }
	T* at( int row, int col ) { return at( ( row * dimX ) + col ); }
	T* row( int index ) { return &linearMatrix[ index * dimX ]; }
	T* col( int index ) { return &linearMatrix[ index ]; }
	
	void createMatrix() {
		if( linearMatrix != nullptr )
			delete[] linearMatrix;
		linearMatrix = new T[ size ];
		for( int i = 0; i < size; i++ ) if ( linearMatrix[ i ] != 0 ) linearMatrix[ i ] = 0;
	}
	
	void deleteMatrix() {
		if( linearMatrix != nullptr )
			delete[] linearMatrix;
	}
	
	void copyMatrix( CMatrix *m ) {
		if( linearMatrix == nullptr || this->size != m->size ) return;
		for( int i = 0; i < size; i++ ) linearMatrix[ i ] = m->linearMatrix[ i ];		
	}
	
	void clearMatrix() {
		for( int i = 0; i < size; i++ ) 
			if ( linearMatrix[ i ] != 0 ) linearMatrix[ i ] = 0;
	}
	
	//ALGORITMO AUTOMA ( called for each cell ( x,y ) )
	
	T updatedValue( int y, int x, int Ymax, int Xmax ) {
		//vertical scan from -1 to 1
		int y_k;
		int x_l;
		int diag = 0;
		for( int k = -1; k <= 1; k++ ) { //rows 0 -1 +1 Y
			y_k = y + k;
			if( y_k < 0 ) y_k = Ymax - 1;
			else if( y_k >= Ymax ) y_k = 0;
	 		for( int l = -1; l <= 1; l++ ) { //columns 0 -1 +1  X
				x_l = x + l;
				if( ( !k && !l ) || x_l >= Xmax || x_l < 0 ) continue;
				if( ( k == -1 || k == 1 ) && ( l == -1 || l == 1 ) ) //diag tiles
					diag += *at( y_k, x_l ) & 1;
			}
		}
		
		//edit this to change AC behaviour
        T self = *at( y, x );
		T newSelf = ( ( diag & 1 ) == 1 ) ? 1 : 0;
				
		return ( ( self << 1 ) | newSelf );
			
	}
	
	void initBoard() {
		long a;
		for( int y = 0; y < dimY; y++ )
			for( int x = 0; x < dimX; x++ ) {
				a = std::rand() % 100;
				if( a >= 68 ) *this->at( y, x ) = 1;
			}
	}
	

	int getDimX() const { return dimX; }
	int getDimY() const { return dimY; }
	int getSize() const { return size; }
	T* getLinearMatrix() { return linearMatrix; }
		
private:
	int dimY;
	int dimX;
	long size;
	T *linearMatrix;

};

#endif
