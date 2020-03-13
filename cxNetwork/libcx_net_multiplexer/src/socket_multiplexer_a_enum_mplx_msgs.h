#ifndef SOCKET_MULTIPLEXER_A_ENUM_MPLX_MSGS
#define SOCKET_MULTIPLEXER_A_ENUM_MPLX_MSGS


enum eMultiplexedSocketMessage {
    MPLX_MSG_CLOSE             =0x00,
    MPLX_CLOSE_ACK1            =0x01,
    MPLX_CLOSE_ACK2            =0x02,
    MPLX_PLUGIN_DATA           =0x10,
    MPLX_PLUGIN_JSON           =0x11,
    MPLX_LINE_CONNECT          =0xF0,
    MPLX_LINE_CONNECT_ANS      =0xF1,
    MPLX_LINE_DATA             =0xF2,
    MPLX_LINE_BYTESREADEN =0xF3,
    BCMSG_CNT_END              =0xFF
};


#endif // SOCKET_MULTIPLEXER_A_ENUM_MPLX_MSGS
