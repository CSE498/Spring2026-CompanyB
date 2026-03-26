#include "Step.hpp"

using cse498::steps::StepContainer;

namespace cse498 {

template <typename DataClass> 
class AgentBase {
protected:
  DataClass mData;

public:

  // The main logic that separates the agents. When prompted for their turn, this where
  // the descisions are made
  virtual [[nodiscard]] StepContainer GetTurn() = 0;

  /*
  * Returning a copy is (most likely) correct here since DataClass is most
  * likely cheap to copy and we dont to modify the DataClass without passing
  * through SetState() since thats what logs the state changes for replay (?)
  */
  [[nodiscard]] DataClass GetState() const noexcept { return mData; }

  void SetState(DataClass data) {
    mData = data;
    // Here handle logic to log for replay?
  }
};

}; // namespace cse498
