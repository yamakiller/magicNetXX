#ifndef CIS_ENGINE_ILOG_H
#define CIS_ENGINE_ILOG_H

#include <string>
#include <iostream>
//#include <format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
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

#define LOG_TRACE(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_TRACE, SOURCE, __VA_ARGS__)
#define LOG_DEBUG(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_DEBUG, SOURCE, __VA_ARGS__)
#define LOG_NOTICE(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_NOTICE, SOURCE, __VA_ARGS__)
#define LOG_WARNING(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_WARNING, SOURCE, __VA_ARGS__)
#define LOG_ERROR(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_ERROR, SOURCE, __VA_ARGS__)
#define LOG_CRITICAL(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_CRITICAL, SOURCE, __VA_ARGS__)
#define LOG_INFO(SOURCE, ...) LOG_PRINTF(cis::ilog::LogLevel::L_INFO, SOURCE, __VA_ARGS__)

#define CONSOLE_LOG_NAME "cis_console"

namespace cis
{
class ilog
{

public:
	virtual ~ilog()
	{
		spdlog::drop_all();
	}
	enum class LogLevel
	{
		L_TRACE = 1,
		L_DEBUG,
		L_WARNING,
		L_ERROR,
		L_CRITICAL,
		L_INFO,
	};

	static void doLog(LogLevel level, uint32_t source, const std::string &msg)
	{

		static uint32_t logger = INST(umodule_mgr, getModuleId, "module_logger");
		if (logger == 0)
		{
			auto console = spdlog::get(CONSOLE_LOG_NAME);
			if (console == nullptr)
				console = spdlog::stdout_color_mt(CONSOLE_LOG_NAME);

			switch (level)
			{
			case LogLevel::L_INFO:
				console->info("[:{:08d}] {}", source, msg.c_str());
				break;
			case LogLevel::L_DEBUG:
				console->debug("[:{:08d}] {}", source, msg.c_str());
				break;
			case LogLevel::L_TRACE:
				console->trace("[:{:08d}] {}", source, msg.c_str());
				break;
			case LogLevel::L_WARNING:
				console->warn("[:{:08d}] {}", source, msg.c_str());
				break;
			case LogLevel::L_ERROR:
				console->error("[:{:08d}] {}", source, msg.c_str());
				break;
			case LogLevel::L_CRITICAL:
				console->critical("[:{:08d}] {}", source, msg.c_str());
				break;
			default:
				break;
			}
			return;
		}

		static constexpr int log_message_size = 256;
		char tmp[log_message_size];
		char *data = NULL;

		uint8_t tmplevel = (uint8_t)level;

		memcpy(tmp, &tmplevel, sizeof(uint8_t));
		int len = snprintf(tmp + 1, log_message_size - 1, "%s", msg.c_str());
		len += 1;

		if (len >= 0 && len < log_message_size)
			data = (char *)umemory::strdup(tmp);
		else
		{
			int tmp_max = log_message_size;
			for (;;)
			{
				tmp_max *= 2;
				data = (char *)umemory::malloc(tmp_max);
				memcpy(data, &tmplevel, sizeof(uint8_t));
				len = snprintf(data + 1, tmp_max - 1, "%s", msg.c_str());
				len += 1;
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
};
} // namespace cis

#endif