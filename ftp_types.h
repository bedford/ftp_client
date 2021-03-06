#ifndef _FTP_TYPES_H_
#define _FTP_TYPES_H_

enum RES_ {

        RES_CONNECT_SUCCESS     = 220,
        RES_REQUIRE_PASSWORD    = 331,
        RES_LOGIN_PASS          = 230,
        RES_QUIT_SUCCESS        = 221,
        RES_PWD                 = 257,
        RES_MKDIR               = 257,
        RES_CHANGE_DIR          = 250,
        RES_SERVER_DOWN         = 421,
        RES_UPLOAD              = 150,
        RES_SYST                = 215,
        RES_EXCUTE_SUCCESS      = 200,
        RES_PASSIVE             = 227,
        RES_NOT_EXCUTE          = 202,
        RES_UPLOADED_SIZE       = 213,
        RES_REST_POSITION       = 350,
        RES_TRANSFER_OK         = 226,
};

enum Openrate_ {

        Username        = 0,
        Password        = 1,
        Quit            = 2,
        Pwd             = 3,
        Mkdir           = 4,
        ChangeDir       = 5,
        Upload          = 6,
        FileType        = 7,
        PassiveMode     = 8,
        PortMode        = 9,
        UploadedSize    = 10,
        RestPosition    = 11,

        MaxIndex,
};

struct _ftp_cmd {
        char            *cmd;
        unsigned int    response_code;
};

struct _data_connect {
        char    addr[64];
        unsigned int port;
};

#endif
