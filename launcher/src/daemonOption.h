#ifndef LAUNCHER_ENGINE_DAEMONOPTION_H
#define LAUNCHER_ENGINE_DAEMONOPTION_H

namespace wolf
{
class daemonOption
{
public:
  daemonOption();
  ~daemonOption();

  int init();
  int exit();

private:
  int local_check_pid();
  int local_write_pid();
  int local_redirect_fds();

private:
  char *m_pidFile;
};
} // namespace wolf

#endif