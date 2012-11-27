#ifndef _FTP_TYPES_H_
#define _FTP_TYPES_H_

enum RES_ {

        RES_CONNECT_SUCCESS     = 220,
        RES_REQUIRE_PASSWORD    = 331,
        RES_LOGIN_PASS          = 230,
        RES_QUIT_SUCCESS        = 221,
        RES_PWD                 = 257,
        RES_MKDIR               = 257,
        RES_CDIR                = 250,
        RES_SERVER_DOWN         = 421,
        RES_UPLOAD              = 150,
};

enum Openrate_ {

        Username        = 0,
        Password        = 1,
        Quit            = 2,
        Pwd             = 3,
        Mkdir           = 4,
        Cdir            = 5,
        Upload          = 6,
        MaxIndex,
};

struct _ftp_cmd {
        char            *cmd;
        unsigned int    response_code;
};

#endif
