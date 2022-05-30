#ifndef CA_GRAPHICS_H
#define CA_GRAPHICS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include "c_matrix.h"
#include <iostream>

template<class T>
class CAGraphics {

    public:
    	CAGraphics( int blockSize, int paddingSize ) : BLOCK_SIZE( blockSize ), PADDING( paddingSize ) {}

        void draw( int y, int x, T n ) {
		int x1 = ( x * BLOCK_SIZE ) + PADDING;
		int y1 = ( y * BLOCK_SIZE ) + PADDING;
		int x2 = x1 + BLOCK_SIZE - PADDING;
		int y2 = y1 + BLOCK_SIZE - PADDING;
		if( n > 0 )
		al_draw_filled_rectangle( x1, y1, x2, y2, al_color_hsl( n, 0.8f, 0.5f ) );       
        }
        
        
        void flip() {
        	al_flip_display();
        }
        
        void clear() {
        	al_clear_to_color( al_map_rgb( 0, 0, 0 ) );
        }

    private:
        const int BLOCK_SIZE;
        const int PADDING;
};

#endif
