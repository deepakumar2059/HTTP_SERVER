class Socket{
    public:
    int fd ;
    int port;
    Socket(int);
    ~Socket();
    void create();
    void bind_socket();
    void listen_socket();
    int accept_con();
    
};