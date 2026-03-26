#pragma once

#include <any>
#include <concepts>
#include <expected>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "core.hpp"
#include "tools/FuncInfo.hpp"
#include "core/WorldPosition.hpp"

/* Gonna redefine things that I know are already
built, this will be updated once this is more than a POC */

namespace cse498
{
  namespace mock {
    struct Location {
      int x;
      int y;
    };
  }; // namespace mock
  
  namespace steps {
    
    using InfoType = std::variant<int, double, bool>;
    using _InfoTuple = std::variant<int, double, bool>;
    
    template <typename T>
    concept IsInfoType = Concepts::IsOneOf<T, int, double, bool>;
    
    struct StepErr {
      enum class Kind {
    EXAMPLE,
    WRONG_TYPE,
    HANDLER_NOT_SET,
  };
  
  Kind kind;
  std::string msg;
};

template <IsInfoType I>
using InfoFunc = std::function<std::expected<bool, StepErr>(I)>;

template <IsInfoType... Is> using InfoFuncTuple = std::tuple<InfoFunc<Is>...>;

// Parameterize the info handler so that we can extremely easily add more types
// later
template <typename... Ts> struct _InfoHandler {
  // Generate the default handler function
  template <IsInfoType T> static InfoFunc<T> defaulted_handler() {
    return [](T) {
      return std::unexpected(
        StepErr{StepErr::Kind::WRONG_TYPE,
          "InfoStep passed invalidly typed InfoType into InfoHandler"});
        };
      }
      
      // Pick and return the passed in func if types match, otherwise generate the
      // default
      template <IsInfoType Target, IsInfoType Current>
      static InfoFunc<Current> pick_handler(InfoFunc<Target> f) {
        if constexpr (std::is_same_v<Target, Current>) {
          return f;
        } else {
          return defaulted_handler<Current>();
        }
      }
      
      // Holds the defaulted handlers which will get one function overwritten
      InfoFuncTuple<Ts...> funcs;
      
      // Allows passing in a lambda
      template <typename F>
      _InfoHandler(F f)
      : funcs({pick_handler<typename std::tuple_element<
        0, typename FuncInfo::FuncInfo<F>::args>::type,
        Ts>(f)...}){};
        
        template <typename S> bool operator()(S s) {
          return std::invoke(std::get<InfoFunc<S>>(funcs), s);
        }
      };
      
      // Now set our desired InfoHandler type and let the compiler handle the rest
      using InfoHandler = _InfoHandler<int, double, bool>;
      
      template <IsInfoType I>
      std::function<std::expected<bool, StepErr>(I)> default_call() {
        return [](I) {
          std::unexpected(StepErr{StepErr::Kind::WRONG_TYPE,
            "InfoStep invalid type InfoType into InfoHandler"});
          };
        }
        
        struct MovementStep {
          WorldPosition loc;
        };
        
        struct InfoStep {
          enum class Aspect {
            OCCUPANCY_RAW,  // How many in area?
            OCCUPANCY_FRAC, // How much of area is occupied?
            LOC_AVAIL,      // Is specific spot available?
          };
          
          Aspect aspect;
          InfoType type;
        };
        
        struct ConditionalStep {
          std::optional<size_t> t_body{std::nullopt};
          std::optional<size_t> f_body{std::nullopt};

          /**
           * NOTE THIS WAS GIVING ME WARNINGS BECAUSE IT HAS NO DEFAULT CONSTRUCTOR. NEED TO FIX LATER AND BRING BACK;
           */
          // InfoHandler condition;
        };
        
        struct ReconStep {
          // TODO (probably gonna scrap)
        };
        
        template <typename T>
        concept StepKind =
        Concepts::IsOneOf<T, MovementStep, InfoStep, ConditionalStep, ReconStep>;
        
        using Step = std::variant<MovementStep, InfoStep, ConditionalStep, ReconStep>;
        
        struct StepContainer {
          std::vector<Step> steps;
          
          // Probably shouldn't be void, later fix
          // This way, "creating" a step sequence should only really involve
          // interacting with the stepcontainer
          template <StepKind S, typename... Args> void add_step(Args &&...a) {
            this->steps.push_back(
              Step{std::in_place_type<S>, std::forward<Args>(a)...});
            }
          };
          
        }; // namespace steps
        
} // namespace cse498