#ifndef CIS_ENGINE_ILOG_H
#define CIS_ENGINE_ILOG_H

#include <string>
#include <format.h>
#include "umemory.h"

#define LOG_PRINTF0(LEVEL, SOURCE, FMT, ...)                           \
	do                                                                   \
	{                                                                    \
		std::string body;                                                  \
		try                                                                \
		{                                                                  \
			body = fmt::format(FMT);                                         \
		}                                                                  \
		catch (...)                                                        \
		{                                                                  \
			body = "format error";                                           \
		}                                                                  \
		std::string tail = "";                                             \
		if (LEVEL == L_ERROR || LEVEL == L_FATAL || LEVEL == L_DEBUG)      \
		{                                                                  \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__);    \
		}                                                                  \
		cis::ilog::doLog(cis::ilog::LogLevel::LEVEL, SOURCE, body + tail); \
	} while (false)

#define LOG_PRINTF(LEVEL, SOURCE, ...)                                 \
	do                                                                   \
	{                                                                    \
		std::string body;                                                  \
		try                                                                \
		{                                                                  \
			body = fmt::format(__VA_ARGS__);                                 \
		}                                                                  \
		catch (...)                                                        \
		{                                                                  \
			LOG_PRINTF0(LEVEL, SOURCE, __VA_ARGS__);                         \
		}                                                                  \
		std::string tail = "";                                             \
		if (LEVEL == L_ERROR || LEVEL == L_FATAL || LEVEL == L_DEBUG)      \
		{                                                                  \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__);    \
		}                                                                  \
		cis::ilog::doLog(cis::ilog::LogLevel::LEVEL, SOURCE, body + tail); \
	} while (false)

#define OUT_PRINTF0(LEVEL, FMT, ...)                                \
	do                                                                \
	{                                                                 \
		std::string body;                                               \
		try                                                             \
		{                                                               \
			body = fmt::format(FMT);                                      \
		}                                                               \
		catch (...)                                                     \
		{                                                               \
			body = "format error";                                        \
		}                                                               \
		std::string tail = "";                                          \
		if (LEVEL == L_ERROR || LEVEL == L_FATAL || LEVEL == L_DEBUG)   \
		{                                                               \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__); \
		}                                                               \
		cis::ilog::doPrintf(cis::ilog::LogLevel::LEVEL, body + tail);   \
	} while (false)

#define OUT_PRINTF(LEVEL, ...)                                      \
	do                                                                \
	{                                                                 \
		std::string body;                                               \
		try                                                             \
		{                                                               \
			body = fmt::format(__VA_ARGS__);                              \
		}                                                               \
		catch (...)                                                     \
		{                                                               \
			OUT_PRINTF0(LEVEL, __VA_ARGS__);                              \
		}                                                               \
		std::string tail = "";                                          \
		if (LEVEL == L_ERROR || LEVEL == L_FATAL || LEVEL == L_DEBUG)   \
		{                                                               \
			fmt::format("[{}][{}:{}]", __FUNCTION__, __FILE__, __LINE__); \
		}                                                               \
		cis::ilog::doPrintf(cis::ilog::LogLevel::LEVEL, body + tail);   \
	}
while (false)

#define LOG_TRACE(SOURCE, ...) LOG_PRINTF(L_TRACE, SOURCE, __VA_ARGS__)
#define LOG_DEBUG(SOURCE, ...) LOG_PRINTF(L_DEBUG, SOURCE, __VA_ARGS__)
#define LOG_NOTICE(SOURCE, ...) LOG_PRINTF(L_NOTICE, SOURCE, __VA_ARGS__)
#define LOG_WARNING(SOURCE, ...) LOG_PRINTF(L_WARNING, SOURCE, __VA_ARGS__)
#define LOG_ERROR(SOURCE, ...) LOG_PRINTF(L_ERROR, SOURCE, __VA_ARGS__)
#define LOG_FATAL(SOURCE, ...) LOG_PRINTF(L_FATAL, SOURCE, __VA_ARGS__)

#define PRINT_TRACE(...) OUT_PRINTF(L_TRACE, __VA_ARGS__)
#define PRINT_DEBUG(...) OUT_PRINTF(L_DEBUG, __VA_ARGS__)
#define PRINT_NOTICE(...) OUT_PRINTF(L_NOTICE, __VA_ARGS__)
#define PRINT_WARNING(...) OUT_PRINTF(L_WARNING, __VA_ARGS__)
#define PRINT_ERROR(...) OUT_PRINTF(L_ERROR, __VA_ARGS__)
#define PRINT_FATAL(...) OUT_PRINTF(L_FATAL, __VA_ARGS__)

	namespace cis
	{
	class ilog
	{
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
		};

		static void doLog(LogLevel level, uint32_t source, const std::string &msg)
		{
			static const char *none = "\033[0m";
			(void)none;
			static const char *black = "\033[0;30m";
			(void)black;
			static const char *dark_gray = "\033[1;30m";
			(void)dark_gray;
			static const char *blue = "\033[0;34m";
			(void)blue;
			static const char *light_blue = "\033[1;34m";
			(void)light_blue;
			static const char *green = "\033[0;32m";
			(void)green;
			static const char *light_green = "\033[1;32m";
			(void)light_green;
			static const char *cyan = "\033[0;36m";
			(void)cyan;
			static const char *light_cyan = "\033[1;36m";
			(void)light_cyan;
			static const char *red = "\033[0;31m";
			(void)red;
			static const char *light_red = "\033[1;31m";
			(void)light_red;
			static const char *purple = "\033[0;35m";
			(void)purple;
			static const char *light_purple = "\033[1;35m";
			(void)light_purple;
			static const char *brown = "\033[0;33m";
			(void)brown;
			static const char *yellow = "\033[1;33m";
			(void)yellow;
			static const char *light_gray = "\033[0;37m";
			(void)light_gray;
			static const char *white = "\033[1;37m";
			(void)white;

			static uint32_t logger = 0;
			if (logger == 0)
				return;

			char color[16];

			switch (level)
			{
			case LogLevel::L_FATAL:
				sprintf(color, purple);
				break;
			case LogLevel::L_ERROR:
				sprintf(color, red);
				break;
			case LogLevel::L_WARNING:
				sprintf(color, yellow);
				break;
			case LogLevel::L_NOTICE:
				sprintf(color, green);
				break;
			case LogLevel::L_DEBUG:
				sprintf(color, white);
				break;
			case LogLevel::L_TRACE:
				sprintf(color, light_gray);
				break;
			default:
				sprintf(color, "");
				break;
			}

			static constexpr int log_message_size = 256;
			char tmp[log_message_size];
			char *data = NULL;

			int len = snprintf(tmp, log_message_size, "%s%s", color, msg.c_str());

			if (len >= 0 && len < log_message_size)
				data = (char *)umemory::strdup(tmp);
			else
			{
				int tmp_max = log_message_size;
				for (;;)
				{
					tmp_max *= 2;
					data = (char *)umemory::malloc(tmp_max);
					len = snprintf(data, tmp_max, "%s%s", color, msg.c_str());
					if (len < tmp_max)
						break;
					umemory::free(data);
				}
			}

			if (len < 0)
			{
				umemory::free(data);
				perror("snprintf error :");
				return;
			}

			/*uevent event;
		event.source = source;
		event.session = 0;
		event.data = data;
		event.type = EP_TEXT;
		event.sz = len;

		ctx_ptr logger_ptr = INST(uctxMgr, grab, logger);
		logger_ptr->eq->push(&event, logger_ptr->eq);*/
		}

		static void doPrintf(LogLevel level, const std::string &msg)
		{
			static const char *none = "\033[0m";
			(void)none;
			static const char *black = "\033[0;30m";
			(void)black;
			static const char *dark_gray = "\033[1;30m";
			(void)dark_gray;
			static const char *blue = "\033[0;34m";
			(void)blue;
			static const char *light_blue = "\033[1;34m";
			(void)light_blue;
			static const char *green = "\033[0;32m";
			(void)green;
			static const char *light_green = "\033[1;32m";
			(void)light_green;
			static const char *cyan = "\033[0;36m";
			(void)cyan;
			static const char *light_cyan = "\033[1;36m";
			(void)light_cyan;
			static const char *red = "\033[0;31m";
			(void)red;
			static const char *light_red = "\033[1;31m";
			(void)light_red;
			static const char *purple = "\033[0;35m";
			(void)purple;
			static const char *light_purple = "\033[1;35m";
			(void)light_purple;
			static const char *brown = "\033[0;33m";
			(void)brown;
			static const char *yellow = "\033[1;33m";
			(void)yellow;
			static const char *light_gray = "\033[0;37m";
			(void)light_gray;
			static const char *white = "\033[1;37m";
			(void)white;

			switch (level)
			{
			case LogLevel::L_FATAL:
				fprintf(stderr, "%s", purple);
				break;
			case LogLevel::L_ERROR:
				fprintf(stderr, "%s", red);
				break;
			case LogLevel::L_WARNING:
				fprintf(stderr, "%s", yellow);
				break;
			case LogLevel::L_NOTICE:
				fprintf(stderr, "%s", green);
				break;
			case LogLevel::L_DEBUG:
				fprintf(stderr, "%s", white);
				break;
			case LogLevel::L_TRACE:
				fprintf(stderr, "%s", light_gray);
				break;
			default:
				break;
			}
			fprintf(stderr, "%s\n", msg.c_str());
		}
	};
	} // namespace cis

#endif