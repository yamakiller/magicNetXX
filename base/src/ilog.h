#ifndef CIS_ENGINE_ILOG_H
#define CIS_ENGINE_ILOG_H

#include <string>
#include <format.h>
#include "umemory.h"
#include "ucomponent_msg.h"
#include "umodule_mgr.h"

#define LOG_PRINTF0(LEVEL, SOURCE, FMT, ...)                              \
	do                                                                    \
	{                                                                     \
		std::string body;                                                 \
		try                                                               \
		{                                                                 \
			body = fmt::format(FMT);                                      \
		}                                                                 \
		catch (...)                                                       \
		{                                                                 \
			body = "format error";                                        \
		}                                                                 \
		std::string tail = "";                                            \
		if (LEVEL == cis::ilog::LogLevel::L_ERROR)                        \
		{                                                                 \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__); \
		}                                                                 \
		cis::ilog::doLog(LEVEL, SOURCE, body + tail);                     \
	} while (false)

#define LOG_PRINTF(LEVEL, SOURCE, ...)                                    \
	do                                                                    \
	{                                                                     \
		std::string body;                                                 \
		try                                                               \
		{                                                                 \
			body = fmt::format(__VA_ARGS__);                              \
		}                                                                 \
		catch (...)                                                       \
		{                                                                 \
			LOG_PRINTF0(LEVEL, SOURCE, __VA_ARGS__);                      \
		}                                                                 \
		std::string tail = "";                                            \
		if (LEVEL == cis::ilog::LogLevel::L_ERROR)                        \
		{                                                                 \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__); \
		}                                                                 \
		cis::ilog::doLog(LEVEL, SOURCE, body + tail);                     \
	} while (false)

#define OUT_PRINTF0(LEVEL, FMT, ...)                                      \
	do                                                                    \
	{                                                                     \
		std::string body;                                                 \
		try                                                               \
		{                                                                 \
			body = fmt::format(FMT);                                      \
		}                                                                 \
		catch (...)                                                       \
		{                                                                 \
			body = "format error";                                        \
		}                                                                 \
		std::string tail = "";                                            \
		if (LEVEL == cis::ilog::LogLevel::L_ERROR)                        \
		{                                                                 \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__); \
		}                                                                 \
		cis::ilog::doPrintf(LEVEL, body + tail);                          \
	} while (false)

#define OUT_PRINTF(LEVEL, ...)                                            \
	do                                                                    \
	{                                                                     \
		std::string body;                                                 \
		try                                                               \
		{                                                                 \
			body = fmt::format(__VA_ARGS__);                              \
		}                                                                 \
		catch (...)                                                       \
		{                                                                 \
			OUT_PRINTF0(LEVEL, __VA_ARGS__);                              \
		}                                                                 \
		std::string tail = "";                                            \
		if (LEVEL == cis::ilog::LogLevel::L_ERROR)                        \
		{                                                                 \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__); \
		}                                                                 \
		cis::ilog::doPrintf(LEVEL, body + tail);                          \
	} while (false)

#define LOG_TRACE(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_TRACE, SOURCE, __VA_ARGS__)
#define LOG_DEBUG(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_DEBUG, SOURCE, __VA_ARGS__)
#define LOG_NOTICE(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_NOTICE, SOURCE, __VA_ARGS__)
#define LOG_WARNING(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_WARNING, SOURCE, __VA_ARGS__)
#define LOG_ERROR(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_ERROR, SOURCE, __VA_ARGS__)
#define LOG_FATAL(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_FATAL, SOURCE, __VA_ARGS__)

#define PRINT_TRACE(...) OUT_PRINTF(cis::ilog::LogLevel::L_TRACE, __VA_ARGS__)
#define PRINT_DEBUG(...) OUT_PRINTF(cis::ilog::LogLevel::L_DEBUG, __VA_ARGS__)
#define PRINT_NOTICE(...) OUT_PRINTF(cis::ilog::LogLevel::L_NOTICE, __VA_ARGS__)
#define PRINT_WARNING(...) OUT_PRINTF(cis::ilog::LogLevel::L_WARNING, __VA_ARGS__)
#define PRINT_ERROR(...) OUT_PRINTF(cis::ilog::LogLevel::L_ERROR, __VA_ARGS__)
#define PRINT_FATAL(...) OUT_PRINTF(cis::ilog::LogLevel::L_FATAL, __VA_ARGS__)

namespace cis
{
class ilog
{
	static constexpr const char *none = "\033[0m";
	static constexpr const char *black = "\033[0;30m";
	static constexpr const char *dark_gray = "\033[1;30m";
	static constexpr const char *blue = "\033[0;34m";
	static constexpr const char *light_blue = "\033[1;34m";
	static constexpr const char *green = "\033[0;32m";
	static constexpr const char *light_green = "\033[1;32m";
	static constexpr const char *cyan = "\033[0;36m";
	static constexpr const char *light_cyan = "\033[1;36m";
	static constexpr const char *red = "\033[0;31m";
	static constexpr const char *light_red = "\033[1;31m";
	static constexpr const char *purple = "\033[0;35m";
	static constexpr const char *light_purple = "\033[1;35m";
	static constexpr const char *brown = "\033[0;33m";
	static constexpr const char *yellow = "\033[1;33m";
	static constexpr const char *light_gray = "\033[0;37m";
	static constexpr const char *white = "\033[1;37m";

public:
	virtual ~ilog() {}
	enum class LogLevel
	{
		L_TRACE,
		L_DEBUG,
		L_NOTICE,
		L_WARNING,
		L_ERROR,
		L_FATAL,
		L_INFO,
	};

	static void doLog(LogLevel level, uint32_t source, const std::string &msg)
	{

		static uint32_t logger = INST(umodule_mgr, getModuleId, "logger");
		if (logger == 0)
			return;

		char color[16];
		char status[16];

		switch (level)
		{
		case LogLevel::L_FATAL:
			sprintf(color, purple);
			sprintf(status, " [fatal] ");
			break;
		case LogLevel::L_ERROR:
			sprintf(color, red);
			sprintf(status, " [error] ");
			break;
		case LogLevel::L_WARNING:
			sprintf(color, yellow);
			sprintf(status, " [warning] ");
			break;
		case LogLevel::L_NOTICE:
			sprintf(color, green);
			sprintf(status, " [notice] ");
			break;
		case LogLevel::L_DEBUG:
			sprintf(color, white);
			sprintf(status, " [debug] ");
			break;
		case LogLevel::L_TRACE:
			sprintf(color, light_gray);
			sprintf(status, " [trace] ");
			break;
		default:
			sprintf(color, none);
			sprintf(status, " [info] ");
			break;
		}

		static constexpr int log_message_size = 256;
		char tmp[log_message_size];
		char *data = NULL;

		int len = snprintf(tmp, log_message_size, "%s[:%08x]%s%s", color, source, status, msg.c_str());

		if (len >= 0 && len < log_message_size)
			data = (char *)umemory::strdup(tmp);
		else
		{
			int tmp_max = log_message_size;
			for (;;)
			{
				tmp_max *= 2;
				data = (char *)umemory::malloc(tmp_max);
				len = snprintf(data, tmp_max, "%s[:%08x]%s%s", color, source, status, msg.c_str());
				if (len < tmp_max)
					break;
				umemory::free(data);
			}
		}

		if (len < 0)
		{
			umemory::free(data);
			return;
		}

		struct umsg hmsg;

		hmsg.source = source;
		hmsg.session = 0;
		hmsg.target = logger;
		hmsg.data = data;
		hmsg.sz = MSG_PACK(len, MsgType::T_TEXT);
		shared_ptr<umodule> ptr = INST(umodule_mgr, getModule, logger);
		if (ptr->push(&hmsg) != 0)
		{
			umemory::free(data);
		}
	}

	static void doPrintf(LogLevel level, const std::string &msg)
	{
		switch (level)
		{
		case LogLevel::L_FATAL:
			fprintf(stderr, "%s [fatal] ", purple);
			break;
		case LogLevel::L_ERROR:
			fprintf(stderr, "%s [error] ", red);
			break;
		case LogLevel::L_WARNING:
			fprintf(stderr, "%s [warning] ", yellow);
			break;
		case LogLevel::L_NOTICE:
			fprintf(stderr, "%s [notice] ", green);
			break;
		case LogLevel::L_DEBUG:
			fprintf(stderr, "%s [debug] ", white);
			break;
		case LogLevel::L_TRACE:
			fprintf(stderr, "%s [trace] ", light_gray);
			break;
		default:
			fprintf(stderr, "%s [info]", "none");
			break;
		}
		fprintf(stderr, "%s\n", msg.c_str());
	}
};
} // namespace cis

#endif