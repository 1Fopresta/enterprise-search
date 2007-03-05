// Uferdig!!!

/**
 *	(C) Copyleft 2007, Magnus Gal�en
 *
 *	dqueue.h: Queue-container.
 */

#ifndef _DQUEUE_H_
#define _DQUEUE_H_

#include <stdarg.h>

#include "dcontainer.h"


container* queue_container( container *C );

int queue_size( container *C );
    // Advarsel, verdiene slettes (free) n�r denne funksjonen kj�res:
void queue_pop( container *C );
value queue_peak( container *C );
void queue_push( container *C, ... );


#endif	// _DQUEUE_H_
