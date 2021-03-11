#include <stdio.h>
#include "altitude_parsing.h"
#include "canlib/util/safe_ring_buffer.h"





int main (void){
    FILE *altitude_data = fopen("altitude_data.txt", "r");
    if(altitude_data == NULL){
        printf("ERROR opening input file");
        system("pause");
        return(EXIT_FAILURE);
    }

    int next_char = getc(altitude_data); //Thanks to EOF we need an int

    init_srb();
    srb_ctx_t *rx_buf_ptr = NULL;
    get_rx_buf_ptr(&rx_buf_ptr);
    if(rx_buf_ptr == NULL){
        printf("ERROR getting buffer pointer");
        system("pause");
        return(EXIT_FAILURE);
    }

    while (next_char >= 0){

        srb_push(rx_buf_ptr, &next_char);
        next_char = getc(altitude_data);

        parse_altitude();
        if (new_altitude_available()){
            printf("Alt: %d\n", get_altitude());
        }
    }
}