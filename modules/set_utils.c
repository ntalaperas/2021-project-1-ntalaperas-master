#include <stdio.h>
#include <stdlib.h>


#include "ADTSet.h"
#include "set_utils.h"


Pointer set_find_eq_or_greater( Set set, Pointer value ){

    SetNode num = set_find_node( set, value );
    Pointer nodevalue  = set_node_value( set, num );

    if( num != SET_EOF ){

            return nodevalue;

    }else{

        if( set_next( set, num ) ){
            return set_next( set, num );
        }else{
            return NULL;
        }

    }
}


Pointer set_find_eq_or_smaller( Set set, Pointer value ){

    SetNode num = set_find_node( set, value );
    Pointer nodevalue  = set_node_value( set, num );

    if( num != SET_EOF ){

          return nodevalue;

    }else{

        if( set_previous( set, num ) ){
            return set_previous( set, num );
        }else{
            return NULL;
        }
        
    }
}