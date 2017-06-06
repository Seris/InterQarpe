#include <Arduino.h>
#include <Stream.h>
#include <InterQarpe.h>

bool InterQarpe::query(const char *qry, query_handler_t handler){
    if(query_out.status != ONGOING){
        write_packet(PAK_QUERY, (uint8_t*) qry, strlen(qry) + 1);
        query_out.status = ONGOING;
        query_out.time_start = millis();
        query_out.handler = handler;
        return true;
    }

    return false;
}

InterQarpe::res_t InterQarpe::query_sync(const char *qry,
                                         uint8_t** data,
                                         size_t* dat_size){

    // Wait for any active query to be finished before sending ours
    while(query_out.status == ONGOING) routine();

    write_packet(PAK_QUERY, (uint8_t*) qry, strlen(qry) + 1);
    query_out.status = ONGOING;
    query_out.time_start = millis();
    query_out.handler = NULL;

    // Wait for the query to complete
    while(query_out.status == ONGOING) routine();

    *dat_size = query_out.dat_size;
    *data = query_out.data;
    return query_out.status;
}

void InterQarpe::manage_query(){
    manage_inc_query();
    if(query_out.status == ONGOING){
        manage_out_query();
    }
}

void InterQarpe::manage_out_query(){
    query_out.status = get_query_result();

    if(query_out.status != ONGOING){
        switch(query_out.status){
            case SUCCESS:
            case FAILURE:
            query_out.data = packet.data;
            query_out.dat_size = packet.dat_size;
            break;

            default:
            query_out.data = NULL;
            query_out.dat_size = 0;
        }

        if(query_out.handler != NULL){
            query_out.handler(	query_out.status,
								query_out.data,
								query_out.dat_size);
        }
    }
}

InterQarpe::res_t InterQarpe::get_query_result(){
    if(packet.available && (packet.type & 0xF0) == PAK_QUERYRES_MASK){
        switch (packet.type) {
            case PAK_SUCCESS:
            return SUCCESS;
            break;

            case PAK_FAILED:
            return FAILURE;
            break;

            case PAK_BAD_QUERY:
            return BAD_QUERY;
            break;
        }

        packet.available = false;
    }

    if(timeout_ms(query_out.time_start, TM_QUERY)){
        return TIMEOUT;
    }

    return ONGOING;
}

void InterQarpe::manage_inc_query(){
    if(packet.available && packet.type == PAK_QUERY){

        // We check if the string is zero terminated. if not, we drop the packet
        if(packet.data[packet.dat_size - 1] == 0){
            on_query((char*) packet.data);
        }
        packet.available = false;
    }
}

void InterQarpe::send_response(bool success, uint8_t *data, size_t dat_size){
    if(success){
        write_packet(PAK_SUCCESS, data, dat_size);
    } else {
        write_packet(PAK_FAILED, data, dat_size);
    }
}

void InterQarpe::send_badquery(){
    write_packet(PAK_BAD_QUERY, NULL, 0);
}
