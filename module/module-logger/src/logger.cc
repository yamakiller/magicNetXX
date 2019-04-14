
#include <base.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace cis;



#define FILE_LOG_NAME "cis_file_log"

class logger : public ucomponent_msg
{
public:
    int initialize(void *lpParam, const char *strParam)
    {
        ucomponent_msg::initialize(lpParam, strParam);

        if (strParam && strcmp(strParam, "") != 0)
        {
            m_ptrConsole = spdlog::basic_logger_mt(FILE_LOG_NAME, strParam);
            m_lpPath = umemory::strdup(strParam);
            strcpy(m_lpPath, strParam);
            fprintf(stderr, "gggggg\n");
        }
        else
        {
            m_ptrConsole = spdlog::get(CONSOLE_LOG_NAME);
            if (m_ptrConsole == nullptr)
            {
                m_ptrConsole = spdlog::stdout_color_mt(CONSOLE_LOG_NAME);
            }
        }

        return 0;
    }

    void finalize()
    {
        m_ptrConsole = nullptr;
        spdlog::drop(CONSOLE_LOG_NAME);
        spdlog::drop(FILE_LOG_NAME);
    }

protected:
    int runOnce(uint32_t source, int session, int type, void *data, size_t sz)
    {
        switch (type)
        {
        case (int)MsgType::T_SIGNAL:
            //set_level
            break;
        case (int)MsgType::T_TEXT:
            {
                uint8_t *level =  (uint8_t*)data;
                std::string msg = ((char*)data) + 1;
                switch(*level)
                {
                    case (uint8_t)ilog::LogLevel::L_INFO:
                        m_ptrConsole->info("[:{:08d}] {}", source, msg.c_str());
                    break;
                    case (uint8_t)ilog::LogLevel::L_DEBUG:
                        m_ptrConsole->debug("[:{:08d}] {}", source, msg.c_str());
                    break;
                    case (uint8_t)ilog::LogLevel::L_TRACE:
                        m_ptrConsole->trace("[:{:08d}] {}", source, msg.c_str());
                    break;
                    case (uint8_t)ilog::LogLevel::L_WARNING:
                        m_ptrConsole->warn("[:{:08d}] {}", source, msg.c_str());
                    break;
                    case (uint8_t)ilog::LogLevel::L_ERROR:
                        m_ptrConsole->error("[:{:08d}] {}", source, msg.c_str());
                    break;
                    case (uint8_t)ilog::LogLevel::L_CRITICAL:
                        m_ptrConsole->critical("[:{:08d}] {}", source, msg.c_str());
                    break;
                    default:
                    break;
                }
                break;
            }
        }

        return 0;
    }

private:
    std::shared_ptr<spdlog::logger> m_ptrConsole;
    char *m_lpPath;
};

extern "C" logger *
module_logger_create()
{
    logger *inst = new logger();
    return inst;
}

extern "C" void 
module_logger_release(logger *inst)
{
    delete inst;
}