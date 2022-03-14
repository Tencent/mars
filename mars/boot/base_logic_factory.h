//
// Created by Cpan on 2022/3/14.
//

#ifndef MMNET_BASE_LOGIC_FACTORY_H
#define MMNET_BASE_LOGIC_FACTORY_H

namespace mars {
namespace boot {
template <class AbstractLogicManager_t> class AbstractLogicManagerFactory {
public:
  virtual AbstractLogicManager_t *CreateLogicManager() = 0;
  virtual ~AbstractLogicManagerFactory() {}
};

template <class AbstractLogicManager_t, class MarsLogicManager_t>
class MarsLogicManagerFactory : public AbstractLogicManagerFactory<AbstractLogicManager_t> {
public:
  AbstractLogicManager_t *CreateLogicManager() { return new MarsLogicManager_t(); }
};

}
}

#endif // MMNET_BASE_LOGIC_FACTORY_H
