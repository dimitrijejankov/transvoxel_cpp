#include "structs.hpp"
#include "transition_sides.hpp"
#include "implementation/extractor.hpp"

template <typename F, typename D, typename S>
Mesh<F> extract(S source, const Block<F>& block, const D& threshold, TransitionSides transition_sides) {
    Extractor<F, D, S> extractor(source, block, threshold, transition_sides);
    return extractor.extract();
}