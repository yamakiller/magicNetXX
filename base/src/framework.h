#ifndef CIS_ENGINE_FRAMEWORK_H
#define CIS_ENGINE_FRAMEWORK_H

namespace engine {
class framework {
public:
  framework();
  virtual ~framework();
  static framework *instance();

  bool doInit();
  void doUnInit();
  void startLoop();

protected:
  virtual bool initialize() = 0;
  virtual bool loop() = 0;
  virtual void finalize() = 0;
};
} // namespace engine

#endif