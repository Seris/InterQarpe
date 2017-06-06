#include <Arduino.h>
#include <InterQarpe.h>

bool InterQarpe::check_stream(){
    if(!packet.available){
        if(read_header() && read_data()){
            packet.available = true;
        }
    }

    return packet.available;
}

bool InterQarpe::read_header(){
    int32_t check_code = PREFIX1 ^ PREFIX2;
    while(stream->available() > HEADER_SIZE + 1){
        if(stream->read() == PREFIX1 && stream->read() == PREFIX2){
            packet.type = stream->read();
            packet.dat_size = stream->read();

            // Compute check code
            check_code ^= packet.type ^ packet.dat_size;

            if(packet.dat_size <= DATA_MAX_SIZE){
                return check_code == stream->read();
            }
        }
    }

    return false;
}

bool InterQarpe::read_data(){
    time_t start = millis();

    while(!timeout_ms(start, TM_DATA)){
        if(stream->available() > packet.dat_size){
            stream->readBytes(packet.data, packet.dat_size);
            return checkcode(packet.data, packet.dat_size) == stream->read();
        }
    }

    return false;
}

int32_t InterQarpe::checkcode(uint8_t* buffer, size_t buf_size){
    int32_t check_code = 0;
    for (size_t i = 0; i < buf_size; i++) {
        check_code ^= buffer[i];
    }
    return check_code;
}


void InterQarpe::write_packet(uint8_t type, uint8_t *data, size_t dat_size){
    uint8_t header[HEADER_SIZE] = {PREFIX1, PREFIX2, type, (uint8_t) dat_size};
    write_buffer(header, HEADER_SIZE);
    write_buffer(data, dat_size);
}

void InterQarpe::write_buffer(uint8_t *buffer, size_t dat_size){
    for(size_t i = 0; i < dat_size; i++){
        stream->write(buffer[i]);
    }
    stream->write(checkcode(buffer, dat_size));
}
