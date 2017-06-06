#include <Arduino.h>
#include <Stream.h>
#include <InterQarpe.h>

InterQarpe::InterQarpe(Stream* stream){
    packet.available = false;
    connection.status = DISCONNECTED;
	// We don't care about the value as long as it isn't ONGOING
    query_out.status = SUCCESS;
    query_out.handler = NULL;
    this->stream = stream;
}

InterQarpe::status_t InterQarpe::status(){
    return connection.status;
}

bool InterQarpe::timeout_ms(time_t start, time_t delay){
    return (millis() - start) > delay;
}
