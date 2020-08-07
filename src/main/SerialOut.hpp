#ifdef DEBUG_ESMART
template <typename T>
struct SerialHelper {
    static T raw(T val) { return val; }
};

template <>
struct SerialHelper<std::string> {
    static const char* raw(const std::string& val) { return val.c_str(); }
};

class SerialOut {
   public:
    template <class... Ts>
    static void info(const char* msg, Ts... args) { printf(msg, SerialHelper<Ts>::raw(args)...); }
};
#else
class SerialOut {
   public:
    template <class... Ts>
    static void info(const char* msg, Ts... args) {}
};
#endif