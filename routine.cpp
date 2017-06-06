#include <Arduino.h>
#include <Stream.h>
#include <InterQarpe.h>

void InterQarpe::routine(){
    check_stream();
    do {
        manage_connection();
        manage_query();

        // we drop any unhandled packet
        packet.available = false;
    } while(check_stream());
}

void InterQarpe::manage_connection(){
    time_t timeout_heartbeat = TM_HEARTBEAT;

    if(packet.available && packet.type == PAK_HEARTBEAT){
        connection.heartbeat_received = millis();
        connection.status = CONNECTED;
    }

    if(connection.status != CONNECTED){
        timeout_heartbeat = TM_FAST_HEARTBEAT;
    }

    if(timeout_ms(connection.heartbeat_sent, timeout_heartbeat)){

        write_packet(PAK_HEARTBEAT, NULL, 0);
        connection.heartbeat_sent = millis();
    }

    if(timeout_ms(connection.heartbeat_received, TM_CON_LOST)){
        connection.status = CON_LOST;
    }
}
