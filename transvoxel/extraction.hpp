#include "structs.hpp"
#include "transition_sides.hpp"
#include "voxel_source.hpp"
#include "implementation/extractor.hpp"

template <typename F, typename D, typename S>
Mesh<F> extract(S source, const Block<F>& block, const D& threshold, TransitionSides transition_sides) {
    Extractor<F, D, S> extractor(source, block, threshold, transition_sides);
    return extractor.extract();
}


template <typename F, typename D, typename SF>
Mesh<F> extract_from_field(
    SF source, const Block<F>& block, const D& threshold, TransitionSides transition_sides)
{
    using WS = WorldMappingVoxelSource<F, D, SF>;

    WS new_src{ source, block };
    return Extractor<F, D, WS>(new_src, block, threshold, transition_sides).extract();
}

template <typename F, typename S, typename C, typename D, typename FUN>
auto extract_from_fn(FUN f, const Block<F>& block, D threshold, TransitionSides transition_sides) -> Mesh<F>
{
    auto field = ScalarFieldForFn<F, FUN>(f);
    auto source = WorldMappingVoxelSource<C, D, decltype(field)>{field, &block};

    return Extractor<F, D, decltype(source)>(source, block, threshold, transition_sides).extract();
}