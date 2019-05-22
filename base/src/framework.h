#ifndef CIS_ENGINE_FRAMEWORK_H
#define CIS_ENGINE_FRAMEWORK_H

namespace engine {

class icommandLine;
class framework {
public:
  framework();
  virtual ~framework();
  static framework *instance();

  bool doInit(const icommandLine *opt);
  void doUnInit();
  void startLoop();

protected:
  virtual bool initialize(const icommandLine *opt) = 0;
  virtual bool loop() = 0;
  virtual void finalize() = 0;
};
} // namespace engine

#endif