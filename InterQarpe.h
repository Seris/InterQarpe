#ifndef __INTERQarpe__
#define __INTERQarpe__

#include <Arduino.h>
#include <Stream.h>

class InterQarpe {
public:
    InterQarpe(Stream* data_stream);

    typedef enum {
        SUCCESS,
        FAILURE,
        DATA,
        BAD_QUERY,
        TIMEOUT,
        ERROR,

		/// For internal use only
		ONGOING,
    } res_t;

    typedef void (*query_handler_t)(res_t, uint8_t*, size_t);

    typedef enum {
        CONNECTED,
        DISCONNECTED,
        CON_LOST
    } status_t;

    /**
     * Check for incoming packet on the stream
     */
    bool check_stream();

    /**
     * Dispatch packet and handle the connection
     */
    void routine();

    /**
     * Connection status (CONNECTED, DISCONNECTED..)
     */
    status_t status();

    /**
     * Send a query
     * @param  qry     query (*MUST BE A ZERO TERMINATED STRING*)
     * @param  handler function pointer that will be executed when a response
     *                 is received or when the query timeout.
     *                 void handler(res_t result, uint8_t* data, size_t datsiz);
     * @return         true is the query was send, false otherwise (only one
     *                 query can be send at a time)
     */
    bool query(const char* qry, query_handler_t handler);

    /**
     * Send a query but return when a response is received instead of using
     * an
     * @param  qry      query (*MUST BE A ZERO TERMINATED STRING*)
     * @param  data
     * @param  dat_size
     */
    res_t query_sync(const char* qry, uint8_t** data, size_t* dat_size);

protected:
    typedef uint32_t time_t;

    static const int32_t DATA_MAX_SIZE = 255;

    /**
     * This function will be executed when a query is received
     * @param query query (guaranteed to be a zero terminated string)
     */
    virtual void on_query(char* query) = 0;

    void send_response(bool success, uint8_t* data, size_t dat_size);
    void send_badquery();

    bool timeout_ms(time_t start, time_t delay);

private:
    // Packet prefix
    static const uint8_t PREFIX1 = 0xB1;
    static const uint8_t PREFIX2 = 0x42;

    // Packet type
    static const uint8_t PAK_OK = 0;
    static const uint8_t PAK_QUERY = 0x10;
    static const uint8_t PAK_SUCCESS = 0x21;
    static const uint8_t PAK_FAILED = 0x22;
    static const uint8_t PAK_BAD_QUERY = 0x29;
    static const uint8_t PAK_HEARTBEAT = 0xF0;

    static const uint8_t PAK_QUERYRES_MASK = 0x20;

    static const int32_t HEADER_SIZE = 4;

    // Timeouts & delays
    time_t TM_DATA = 20;
    time_t TM_QUERY = 2000;
    time_t TM_FAST_HEARTBEAT = 1000;
    time_t TM_HEARTBEAT = 10000;
    time_t TM_CON_LOST = 21000;

    typedef struct {
        bool available;
        uint8_t type;
        uint8_t dat_size;
        uint8_t data[DATA_MAX_SIZE];
    } pak_t;

    typedef struct {
        time_t heartbeat_sent;
        time_t heartbeat_received;
        status_t status;
    } connection_t;

    typedef struct {
		res_t status;
        time_t time_start;
		query_handler_t handler;
        uint8_t* data;
        size_t dat_size;
    } query_t;

    Stream* stream;
    pak_t packet;
    connection_t connection;

    query_t query_out;

    void manage_connection();

    void manage_query();
    void manage_inc_query();
    void manage_out_query();
    res_t get_query_result();

    void write_packet(uint8_t type, uint8_t* data, size_t dat_size);
    void write_buffer(uint8_t* buffer, size_t dat_size);

    bool read_header();
    bool read_data();

    int32_t checkcode(uint8_t* buffer, size_t buf_size);
};

#endif // __INTERQarpe__
