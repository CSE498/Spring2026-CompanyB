#include <print>

#include "../../source/tools/WeightedSet.hpp"
int main() {
  cse498::WeightedSet<int> ws;
  /* We insert 1, 2, and 3 into the set, with weights 1, 2, and 3.
   * The relative weights determine the probability that each element will
   * be selected by GetRandomElement, so (whatever the total weight of the
   * elements in the set) an element with weight 2 will be twice as likely to be
   * selected as an element with weight 1.
   *
   * More concretely the probability of a given element being selected is just
   * (weight of element) / (total weight), so since the set has a total weight
   * of 6 after these insertions, 1 will have a 1/6 chance of being selected, 2
   * has a 2/6 = 1/3 chance, and 3 has a 3/6 = 1/2 chance.
   */
  ws.Insert(1, 1.0);
  ws.Insert(2, 2.0);
  ws.Insert(3, 3.0);

  /* Internally, the elements are represented in a tree--see the comment on
   * GetElementAt in WeightedSet for more on this. (Note, though, that they are stored as 
   * keys in a map, which is used to speed up some operations; the tree just has pointers
   * to those elements in the map.) After inserting 1, the tree just has a single node (the root) 
   * pointing to 1. 2 gets inserted on the left branch--it could go in 
   */

  std::println("Total weight: {}", ws.total_weight());
  const int num_samples = 60000;

  std::unordered_map<int, int> counts;
  for (int i = 0; i < num_samples; ++i) {
    auto result = ws.GetRandomElement();
    ++counts[result.value()];
  }

  for (int elem = 1; elem <= 3; ++elem) {
    const double p = static_cast<double>(elem) / ws.total_weight();
    const double p_hat = static_cast<double>(counts[elem]) / num_samples;

    std::cout << "element " << elem << ": expected=" << p << " actual=" << p_hat
              << std::endl;
  }
  ws.Remove(3);
  std::println("After removing 3");
  std::println("Total weight: {}", ws.total_weight());
  counts.clear();
  for (int i = 0; i < num_samples; ++i) {
    auto result = ws.GetRandomElement();
    ++counts[result.value()];
  }
  for (int elem = 1; elem <= 2; ++elem) {
    const double p = static_cast<double>(elem) / ws.total_weight();
    const double p_hat = static_cast<double>(counts[elem]) / num_samples;

    std::cout << "element " << elem << ": expected=" << p << " actual=" << p_hat
              << std::endl;
  }
}