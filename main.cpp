#include "ca_graphics.h"
#include <mpi.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>

using namespace std;

#define DIM 220  //size of the matrix
#define COUNTER 2000 //cycles

//cells type
#define CTYPE unsigned int    //you can easly change types using these two macros, just make sure
#define MPITYPE MPI_UNSIGNED  //to pick the right corresponding MPI_Type ( e.g. int -> MPI_INT )

//mpi stuff
#define MASTER 0

#define FC_TAG 0 //first column tag ( send )
#define LC_TAG 1 //last column tag ( send )
#define PRINT_TAG 2
#define IM_TAG 3

//graphics stuff
#define HAS_GRAPHICS true //toggles graphic output  <----- VERY IMPORTANT ( should be false if you're measuring performances )
#define SLEEP_TIME 50 //50 is the intended speed. Although lowering this value speeds everything up ( only works with graphics on )
#define BLOCK_SIZE 4
#define PADDING 0 //default 0
#define D_WIDTH ( DIM * BLOCK_SIZE )
#define D_HEIGHT ( DIM * BLOCK_SIZE )

atomic<bool> stop( false );
void *eventH( ALLEGRO_THREAD*, void* );
void compute( CMatrix<CTYPE>*, CMatrix<CTYPE>* );
void computePrinting( CMatrix<CTYPE>* );
void draw( int, CTYPE*, int, int, int );

//globals
bool quit = false;
int numProcs, //number of processes
myId, //current process id
counter = COUNTER, //main loop counter
numCols, //number of columns foreach process
master_plus = 0;
long startt, endt;
MPI_Datatype column;
MPI_Datatype bigColumn;
CMatrix<CTYPE>* initialBoard;

#if HAS_GRAPHICS
CAGraphics<CTYPE> graphics( BLOCK_SIZE, PADDING );
#endif


int main( int argc, char *argv[] ) {
		
	//mpi stuff
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &myId );
	
	//allegro stuff
	#if HAS_GRAPHICS
	ALLEGRO_DISPLAY *display;
	ALLEGRO_THREAD *t1;
	if( !al_init() || !al_init_primitives_addon() ) return -1;
	
	if( myId == MASTER ) {
	display = al_create_display( D_WIDTH, D_HEIGHT );
	if (display == nullptr) return -1;
	al_set_window_title( display, "AC Project - V.R" );
	al_clear_to_color( al_map_rgb( 0, 0, 0 ) );
	t1 = al_create_thread( eventH, display );
	al_start_thread( t1 );
	}
	#endif
	
	//START TIME
	if( myId == MASTER ) startt = MPI_Wtime();
			
	numCols = ( DIM / numProcs ) + 2; //2 more columns to store first-- and last++

	master_plus += DIM % numProcs;
	
	//needed to cover each column
	if( myId == MASTER ) numCols += master_plus;
	
	
	//custom data-type to send columns
	MPI_Type_vector( DIM, 1, numCols, MPITYPE, &column );
	MPI_Type_commit( &column );
	
	
	//linear matrix to use dynamic allocation
	CMatrix<CTYPE> board1( DIM, numCols );
	CMatrix<CTYPE> board2( DIM, numCols );

	//INIT INITIAL BOARD
	if( myId == MASTER ) {
		initialBoard = new CMatrix<CTYPE>( DIM, DIM );
		*initialBoard->at( DIM / 2, DIM / 2 ) = 1;
		*initialBoard->at( DIM / 2 + 1, DIM / 2 ) = 1;
		*initialBoard->at( DIM / 2, DIM / 2 + 1 ) = 1;
		*initialBoard->at( DIM / 2 + 1, DIM / 2 + 1 ) = 1;
	}
	
	//custom data-type to send initial matrix columns
	MPI_Type_vector( DIM, 1, DIM, MPITYPE, &bigColumn );
	MPI_Type_commit( &bigColumn );
		
	
	//init processes boards
	int c1 = numCols - 2, c2 = 1;
	switch( myId ) {
		case MASTER :
		for( int y = 0; y < DIM; y++ )
		for( int x1 = 1, x = 0; x < numCols - 2; x++, x1++ )
		*board2.at( y, x1 ) = *initialBoard->at( y, x );
		
		for( int proc = 1; proc < numProcs; proc++ )
		for( int x = 0; x < DIM / numProcs; x++, c1++ )
		MPI_Send( initialBoard->col( c1 ), 1, bigColumn, proc, IM_TAG, MPI_COMM_WORLD );
		break;
		
		default:
		for( ; c2 < numCols - 1; c2++ )
		MPI_Recv( board2.col( c2 ), 1, column, MASTER, IM_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		break;
	}

	if( myId == MASTER )
	delete initialBoard;		
				
	//main loop
	while( counter-- ) {
	
		//END CHECK ( ugly way to quit when you click the "x" on the window )
		#if HAS_GRAPHICS
		if( stop )
			quit = true;
		MPI_Bcast( &quit, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD );
		if( quit )
			break;
		#endif	
			
		//COMPUTING
		compute( &board1, &board2 );
		
		//PRINT
		computePrinting( &board2 );
		#if HAS_GRAPHICS
		this_thread::sleep_for( chrono::milliseconds( SLEEP_TIME ) );	
		#endif
	}
	
	#if HAS_GRAPHICS
	if( myId == MASTER ) {
		stop = true;
		al_destroy_thread( t1 );
		al_destroy_display( display );
	}
	#endif
	
	//START TIME
	if( myId == MASTER ) {
	endt = MPI_Wtime();
	cout << "Start: " << startt << endl;
	cout << "End: " << endt << endl;
	cout << "Elapsed: " << endt - startt << endl;
	}
	
	MPI_Finalize();  
	
	return 0;
}

//catches close event
void *eventH( ALLEGRO_THREAD *thread, void *arg ) {
	#if HAS_GRAPHICS
	ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
	ALLEGRO_EVENT event;
	ALLEGRO_DISPLAY *display = ( ALLEGRO_DISPLAY* ) arg;
	al_register_event_source( event_queue, al_get_display_event_source( display ) );	
	while( true ) {
		al_wait_for_event( event_queue, &event );
		if( event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || stop ) {
			stop = true;	
			return NULL;
		}
	}
	#endif
	return NULL;
}

//MPI + AC stuff
void compute( CMatrix<CTYPE> *board1, CMatrix<CTYPE> *board2 ) {

	board1->copyMatrix( board2 );
	board2->clearMatrix();
	
	MPI_Request lastCol, firstCol;
	
	//receive MY first-- col from previous process
	int sen_id2 = ( myId - 1 >= 0 ) ? myId - 1 : numProcs - 1;
	MPI_Irecv( board1->col( 0 ), 1, column, sen_id2, LC_TAG, MPI_COMM_WORLD, &firstCol );
	
	//receive MY last++ col from next process
	int sen_id = ( myId + 1 < numProcs ) ? myId + 1 : 0;
	MPI_Irecv( board1->col( numCols - 1 ), 1, column, sen_id, FC_TAG, MPI_COMM_WORLD, &lastCol );
	
	//send MY last col to next process
	int rec_id2 = ( myId + 1 < numProcs ) ? myId + 1 : 0;
	MPI_Send( board1->col( numCols - 2 ), 1, column, rec_id2, LC_TAG, MPI_COMM_WORLD );
	
	//send MY first col to previous process
	int rec_id = ( myId - 1 >= 0 ) ? myId - 1 : numProcs - 1;
	MPI_Send( board1->col( 1 ), 1, column, rec_id, FC_TAG, MPI_COMM_WORLD );
	
	//computing from 2nd col to last - 2 col
	for( int y = 0; y < DIM; y++ )
		for( int x = 2; x < numCols - 2; x++ )
			*board2->at( y, x ) = board1->updatedValue( y, x, DIM, numCols );
	
	//computing first col
	MPI_Wait( &firstCol, MPI_STATUS_IGNORE );
	for( int y = 0; y < DIM; y++ )
		*board2->at( y, 1 ) = board1->updatedValue( y, 1, DIM, numCols );
		
	//computing last col
	MPI_Wait( &lastCol, MPI_STATUS_IGNORE );
	for( int y = 0; y < DIM; y++ )
		*board2->at( y, numCols - 2 ) = board1->updatedValue( y, numCols - 2, DIM, numCols );

}



void draw( int id, CTYPE *board, int dimY, int dimX, int master_plus ) {
	#if HAS_GRAPHICS
	CMatrix<CTYPE> temp( board, dimY, dimX );
	int offset = ( id * ( dimX - 2 ) + ( master_plus / 2 ) );
	for( int y = 0; y < dimY; y++ ) {
		for( int x = 1, x1 = 0; x < dimX - 1; x++, x1++ ) {
			graphics.draw( y, x1 + offset, *temp.at( y, x ) );
		}
	}
	#endif
}


void computePrinting( CMatrix<CTYPE> *board2 ) {
	switch( myId ) {
	case MASTER :
		#if HAS_GRAPHICS
		graphics.clear();
		draw( MASTER, board2->getLinearMatrix(), DIM, numCols, master_plus );
		#endif
		for( int proc = 1; proc < numProcs; proc++ )
		{
			int nc = 0;
			MPI_Recv( &nc, 1, MPI_INT, proc, PRINT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			int size = DIM * nc;
			CTYPE *lm = new CTYPE[ size ];
			MPI_Recv( lm, size, MPITYPE, proc, PRINT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			#if HAS_GRAPHICS
			draw( proc, lm, DIM, nc, master_plus );
			#endif
			delete[] lm;			
		}
		#if HAS_GRAPHICS
		graphics.flip();
		#endif
	break;
	default:
		int nc = board2->getDimX();
		MPI_Send( &nc, 1, MPI_INT, MASTER, PRINT_TAG, MPI_COMM_WORLD );
		MPI_Send( board2->getLinearMatrix(), board2->getSize(), MPITYPE, MASTER, PRINT_TAG, MPI_COMM_WORLD );
	break;
	}

}

